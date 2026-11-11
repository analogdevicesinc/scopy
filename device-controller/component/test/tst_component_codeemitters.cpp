/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "component/backends/iio/iioprovider.h"
#include "component/codeemitterregistry.h"

#include "core/codeemitter.h"
#include "iioutil/ibackend.h"
#include "iioutil/libiioemitter.h"

#include <QDir>
#include <QFile>
#include <QTest>

using namespace scopy;
using namespace scopy::component;
using scopy::iio::LibiioEmitter;
using scopy::iio::LibiioVersion;

namespace {

namespace os = originscope;

constexpr qint64 T0 = 1'767'225'600'000'000'000LL; // 2026-01-01 00:00:00 UTC
constexpr auto URI = "ip:192.168.2.1";

QString lang(const char *l) { return QString::fromLatin1(l); }

CallRecord rec(const char *fn, quintptr handle, const QList<CallArg> &args, int rc = 0, const QVariant &out = {},
	       CallArg::Kind outKind = CallArg::Int, qint64 outSize = -1)
{
	CallRecord r;
	r.library = QStringLiteral("libiio");
	r.function = QString::fromLatin1(fn);
	r.handle = handle;
	r.args = args;
	r.rc = rc;
	r.out = out;
	r.outKind = outKind;
	r.outSize = outSize;
	r.durationNs = 1000;
	return r;
}

CallRecord handleOut(const char *fn, quintptr handle, const QList<CallArg> &args, quintptr out)
{
	return rec(fn, handle, args, 0, QVariant::fromValue(out), CallArg::Handle);
}

CallRecord payloadOut(const char *fn, quintptr handle, const QList<CallArg> &args, int rc, char fill)
{
	return rec(fn, handle, args, rc, QByteArray(8, fill), CallArg::Bytes, 4096);
}

// Builds entries one after the other (1 ms apart), like the command monitor delivers them.
struct Log
{
	QList<CommandLogEntry> entries;

	void add(const QString &origin, const char *cls, const QVariantMap &scope, const QList<CallRecord> &calls)
	{
		CommandLogEntry e;
		e.origin = origin;
		e.uri = QString::fromLatin1(URI);
		e.backendId = QStringLiteral("iio");
		e.commandClass = QStringLiteral("scopy::iio::") + QString::fromLatin1(cls);
		e.startNs = T0 + entries.size() * 1'000'000LL;
		e.durationNs = 5000;
		e.calls = calls;
		e.originScope = scope;
		for(const CallRecord &c : calls) {
			if(c.rc < 0) {
				e.error = Error{c.rc, QStringLiteral("failed")};
			}
		}
		entries << e;
	}
};

QVariantMap channelAttrScope()
{
	return {{os::Kind, "Attribute"},	 {os::Context, "ctx"},	      {os::Device, "ad9361-phy"},
		{os::DeviceId, "iio:device1"},	 {os::Channel, "voltage0"},   {os::Output, true},
		{os::Attribute, "hardwaregain"}, {os::AttributeOf, "channel"}};
}

QVariantMap regScope(const char *kind)
{
	return {{os::Kind, kind}, {os::Context, "ctx"}, {os::Device, "ad9361-phy"}, {os::DeviceId, "iio:device1"}};
}

QVariantMap streamScope()
{
	return {{os::Kind, "InputStream"},
		{os::Context, "ctx"},
		{os::Device, "cf-ad9361-lpc"},
		{os::DeviceId, "iio:device3"},
		{os::Stream, "input"},
		{os::BufferIndex, 0},
		{os::Output, false}};
}

QVariantMap scanScope()
{
	QVariantMap s = streamScope();
	s.insert(os::Kind, "ScanElement");
	s.insert(os::Channel, "voltage0");
	return s;
}

// Handles as the backends record them.
constexpr quintptr H_CHN_OUT = 0x1000; // v0: iio_channel* of voltage0 (output) on ad9361-phy
constexpr quintptr H_ATTR = 0xa000;    // v1: iio_attr* of hardwaregain
constexpr quintptr H_PHY = 0x3000;     // iio_device* ad9361-phy
constexpr quintptr H_CHN_IN = 0x4000;  // iio_channel* of voltage0 (input) on cf-ad9361-lpc
constexpr quintptr H_LPC = 0x5000;     // iio_device* cf-ad9361-lpc
constexpr quintptr H_BUF = 0x6000;     // v0: iio_buffer*
constexpr quintptr H_MASK = 0x7000;    // v1: iio_channels_mask* (created with the stream, not logged)
constexpr quintptr H_SBUF = 0x8000;    // v1: iio_buffer* (static buffer 0)
constexpr quintptr H_STREAM = 0x9000;  // v1: iio_buffer_stream*
constexpr quintptr H_BLOCK = 0xb000;   // v1: iio_block*
constexpr quintptr H_CTX = 0xc000;

const QString PATH_GAIN = QStringLiteral("ctx/ad9361-phy/voltage0/hardwaregain");
const QString PATH_REGW = QStringLiteral("ctx/ad9361-phy/RegisterWriter");
const QString PATH_REGR = QStringLiteral("ctx/ad9361-phy/RegisterReader");
const QString PATH_STREAM = QStringLiteral("ctx/cf-ad9361-lpc/InputStream");
const QString PATH_SCAN = QStringLiteral("ctx/cf-ad9361-lpc/InputStream/Scan element 0");

// The fixed golden log: attr write + read, register write + read, channel enable, stream open,
// refill x4, an unmapped call.
QList<CommandLogEntry> goldenLog(LibiioVersion v)
{
	Log log;
	const bool v1 = v == LibiioVersion::V1;
	const QString gain = QStringLiteral("-10.0");
	const QString gainRead = QStringLiteral("-10.000000 dB");
	if(v1) {
		log.add(PATH_GAIN, "AttrWriteCommand", channelAttrScope(),
			{rec("iio_attr_write_raw", H_ATTR,
			     {CallArg::string("attr", "hardwaregain"), CallArg::string("src", gain),
			      CallArg::integer("len", 5)},
			     5)});
		log.add(PATH_GAIN, "AttrReadCommand", channelAttrScope(),
			{rec("iio_attr_read_raw", H_ATTR,
			     {CallArg::string("attr", "hardwaregain"), CallArg::outLen("len", 4096)}, 14, gainRead,
			     CallArg::String, 13)});
	} else {
		log.add(PATH_GAIN, "AttrWriteCommand", channelAttrScope(),
			{rec("iio_channel_attr_write", H_CHN_OUT,
			     {CallArg::string("channel", "voltage0"), CallArg::string("attr", "hardwaregain"),
			      CallArg::string("src", gain)},
			     6)});
		log.add(PATH_GAIN, "AttrReadCommand", channelAttrScope(),
			{rec("iio_channel_attr_read", H_CHN_OUT,
			     {CallArg::string("channel", "voltage0"), CallArg::string("attr", "hardwaregain"),
			      CallArg::outLen("len", 4096)},
			     14, gainRead, CallArg::String, 13)});
	}
	log.add(PATH_REGW, "RegWriteCommand", regScope("RegisterWriter"),
		{rec("iio_device_reg_write", H_PHY,
		     {CallArg::string("device", "ad9361-phy"), CallArg::hex("address", 0x37),
		      CallArg::hex("value", 0x5a)})});
	log.add(PATH_REGR, "RegReadCommand", regScope("RegisterReader"),
		{rec("iio_device_reg_read", H_PHY,
		     {CallArg::string("device", "ad9361-phy"), CallArg::hex("address", 0x37)}, 0,
		     QVariant::fromValue(quint64(0x5a)))});
	if(v1) {
		log.add(PATH_SCAN, "ChnEnableCommand", scanScope(),
			{rec("iio_channel_enable", H_CHN_IN,
			     {CallArg::string("channel", "voltage0"), CallArg::handle("mask", (void *)H_MASK)})});
		log.add(PATH_STREAM, "BufferOpenCommand", streamScope(),
			{handleOut("iio_device_get_buffer", H_LPC, {CallArg::integer("index", 0)}, H_SBUF),
			 handleOut("iio_buffer_open", H_SBUF, {CallArg::handle("mask", (void *)H_MASK)}, H_STREAM),
			 rec("iio_device_get_sample_size", H_LPC, {CallArg::handle("mask", (void *)H_MASK)}, 4),
			 handleOut("iio_buffer_stream_create_block", H_STREAM, {CallArg::integer("size", 4096)},
				   H_BLOCK)});
		const CallRecord enq = rec("iio_block_enqueue", H_BLOCK,
					   {CallArg::integer("bytes_used", 0), CallArg::boolean("cyclic", false)});
		auto deq = [](char fill) {
			return payloadOut("iio_block_dequeue", H_BLOCK, {CallArg::boolean("nonblock", false)}, 0, fill);
		};
		log.add(PATH_STREAM, "BufferRefillCommand", streamScope(),
			{enq, rec("iio_buffer_stream_start", H_STREAM, {}), deq('\x01')});
		for(char f : {'\x02', '\x03', '\x04'}) {
			log.add(PATH_STREAM, "BufferRefillCommand", streamScope(), {enq, deq(f)});
		}
	} else {
		log.add(PATH_SCAN, "ChnEnableCommand", scanScope(),
			{rec("iio_channel_enable", H_CHN_IN, {CallArg::string("channel", "voltage0")})});
		log.add(PATH_STREAM, "BufferOpenCommand", streamScope(),
			{rec("iio_device_set_kernel_buffers_count", H_LPC, {CallArg::integer("nb_buffers", 4)}),
			 handleOut("iio_device_create_buffer", H_LPC,
				   {CallArg::integer("samples_count", 1024), CallArg::boolean("cyclic", false)},
				   H_BUF)});
		for(char f : {'\x01', '\x02', '\x03', '\x04'}) {
			log.add(PATH_STREAM, "BufferRefillCommand", streamScope(),
				{payloadOut("iio_buffer_refill", H_BUF, {}, 4096, f)});
		}
	}
	log.add(QStringLiteral("ctx"), "FnCommand", {{os::Kind, "Context"}, {os::Context, "ctx"}},
		{rec("iio_context_get_xml", H_CTX, {CallArg::boolean("compressed", false)})});
	return log.entries;
}

std::shared_ptr<CodeEmitterRegistry> registryFor(LibiioVersion v)
{
	auto r = std::make_shared<CodeEmitterRegistry>();
	for(const auto &e : LibiioEmitter::create(v)) {
		r->add(e);
	}
	return r;
}

// A second library with a C emitter only.
class OtherEmitter : public CodeEmitter
{
public:
	QString library() const override { return QStringLiteral("other"); }
	QString language() const override { return lang(codegen::C); }
	QStringList headers(EmitContext &) const override { return {QStringLiteral("#include <other.h>")}; }
	QString prologue(EmitContext &) const override { return QStringLiteral("other_init();"); }
	QString emitCall(const CallRecord &call, EmitContext &) const override { return call.function + "();"; }
	QString epilogue(EmitContext &) const override { return QStringLiteral("other_exit();"); }
};

QString goldenPath(const QString &name) { return QStringLiteral(DC_GOLDEN_DIR "/") + name; }

QString readFile(const QString &path)
{
	QFile f(path);
	if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return {};
	}
	return QString::fromUtf8(f.readAll());
}

QList<CommandLogEntry> refills(int n, bool sameValue = false)
{
	Log log;
	log.add(PATH_STREAM, "BufferOpenCommand", streamScope(),
		{handleOut("iio_device_create_buffer", H_LPC,
			   {CallArg::integer("samples_count", 1024), CallArg::boolean("cyclic", false)}, H_BUF)});
	for(int i = 0; i < n; ++i) {
		log.add(PATH_STREAM, "BufferRefillCommand", streamScope(),
			{payloadOut("iio_buffer_refill", H_BUF, {}, 4096, sameValue ? 1 : char(i))});
	}
	return log.entries;
}

QList<CommandLogEntry> gainReads(const QStringList &values)
{
	Log log;
	for(const QString &v : values) {
		log.add(PATH_GAIN, "AttrReadCommand", channelAttrScope(),
			{rec("iio_attr_read_raw", H_ATTR,
			     {CallArg::string("attr", "hardwaregain"), CallArg::outLen("len", 4096)}, int(v.size()) + 1,
			     v, CallArg::String, v.size())});
	}
	return log.entries;
}

} // namespace

class TstComponentCodeEmitters : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void golden_data();
	void golden();

	void languagesForMixedLibraries();
	void languagesForEmptySelection();
	void addReplacesSameLanguage();
	void instanceMatchesLinkedBackend();
	void emptySelectionGivesNoScript();
	void orderedByTime();
	void oneProloguePerLibrary();
	void libraryWithoutEmitterIsComment();
	void headerWarnsAboutState();

	void loopsOnOff();
	void sameHandleSameVariable();
	void unresolvedAttributeIsComment();
	void failedCallIsCaught();
	void partialPayloadNotWritten();

	void literalsPerKind();
	void stringEscaping();
	void comments();
};

void TstComponentCodeEmitters::golden_data()
{
	QTest::addColumn<int>("version");
	QTest::addColumn<QString>("language");
	QTest::addColumn<QString>("file");
	for(int v : {0, 1}) {
		for(const char *l : {codegen::C, codegen::Python}) {
			const QString file =
				QStringLiteral("libiio_v%1.%2").arg(v).arg(codegen::fileExtension(lang(l)));
			QTest::newRow(qPrintable(file)) << v << lang(l) << file;
		}
	}
}

void TstComponentCodeEmitters::golden()
{
	QFETCH(int, version);
	QFETCH(QString, language);
	QFETCH(QString, file);
	const auto v = version ? LibiioVersion::V1 : LibiioVersion::V0;
	EmitOptions opts;
	opts.loops = true;
	const QString script = registryFor(v)->exportScript(goldenLog(v), language, opts);
	QVERIFY(!script.isEmpty());
	if(qEnvironmentVariableIsSet("DC_UPDATE_GOLDENS")) {
		QFile f(goldenPath(file));
		QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
		f.write(script.toUtf8());
		QSKIP("golden updated");
	}
	const QString expected = readFile(goldenPath(file));
	QVERIFY2(!expected.isEmpty(), qPrintable(QStringLiteral("missing golden %1").arg(goldenPath(file))));
	if(script != expected) {
		const QStringList a = script.split('\n'), b = expected.split('\n');
		for(int i = 0; i < qMax(a.size(), b.size()); ++i) {
			if(a.value(i) != b.value(i)) {
				qWarning().noquote()
					<< "line" << i + 1 << "got:" << a.value(i) << "expected:" << b.value(i);
				break;
			}
		}
	}
	QCOMPARE(script, expected);
}

void TstComponentCodeEmitters::languagesForMixedLibraries()
{
	auto r = registryFor(LibiioVersion::V1);
	r->add(std::make_shared<OtherEmitter>());
	const QStringList all = {lang(codegen::C), lang(codegen::Python)};
	QCOMPARE(r->languagesFor({QStringLiteral("libiio")}), all);
	QCOMPARE(r->languagesFor({QStringLiteral("libiio"), QStringLiteral("other")}), QStringList{lang(codegen::C)});
	QCOMPARE(r->languagesFor({QStringLiteral("other")}), QStringList{lang(codegen::C)});
	QVERIFY(r->languagesFor({QStringLiteral("libiio"), QStringLiteral("unknown")}).isEmpty());
	QCOMPARE(r->libraries(), (QStringList{QStringLiteral("libiio"), QStringLiteral("other")}));
}

void TstComponentCodeEmitters::languagesForEmptySelection()
{
	auto r = registryFor(LibiioVersion::V0);
	QVERIFY(r->languagesFor({}).isEmpty());
	QVERIFY(CodeEmitterRegistry::librariesOf({}).isEmpty());
	CommandLogEntry noCalls;
	QVERIFY(CodeEmitterRegistry::librariesOf({noCalls}).isEmpty());
	QCOMPARE(CodeEmitterRegistry::librariesOf(goldenLog(LibiioVersion::V0)), QStringList{QStringLiteral("libiio")});
}

void TstComponentCodeEmitters::addReplacesSameLanguage()
{
	CodeEmitterRegistry r;
	r.add(std::make_shared<LibiioEmitter>(LibiioVersion::V0, lang(codegen::C)));
	r.add(std::make_shared<LibiioEmitter>(LibiioVersion::V1, lang(codegen::C)));
	auto e = std::dynamic_pointer_cast<LibiioEmitter>(r.emitter(QStringLiteral("libiio"), lang(codegen::C)));
	QVERIFY(e);
	QCOMPARE(e->version(), LibiioVersion::V1);
	QCOMPARE(r.languagesFor({QStringLiteral("libiio")}), QStringList{lang(codegen::C)});
	QVERIFY(r.remove(QStringLiteral("libiio"), lang(codegen::C)));
	QVERIFY(!r.remove(QStringLiteral("libiio"), lang(codegen::C)));
	QVERIFY(r.languagesFor({QStringLiteral("libiio")}).isEmpty());
}

void TstComponentCodeEmitters::instanceMatchesLinkedBackend()
{
	auto *r = CodeEmitterRegistry::GetInstance();
	QCOMPARE(r, CodeEmitterRegistry::GetInstance());
	const LibiioVersion linked = component::iio::IIOProvider::defaultBackend()->version();
	for(const char *l : {codegen::C, codegen::Python}) {
		auto e = std::dynamic_pointer_cast<LibiioEmitter>(r->emitter(QStringLiteral("libiio"), lang(l)));
		QVERIFY(e);
		QCOMPARE(e->version(), linked);
	}
}

void TstComponentCodeEmitters::emptySelectionGivesNoScript()
{
	auto r = registryFor(LibiioVersion::V1);
	QVERIFY(r->exportScript({}, lang(codegen::Python)).isEmpty());
	CommandLogEntry noCalls;
	noCalls.origin = PATH_GAIN;
	QVERIFY(r->exportScript({noCalls}, lang(codegen::Python)).isEmpty());
}

void TstComponentCodeEmitters::orderedByTime()
{
	auto r = registryFor(LibiioVersion::V0);
	QList<CommandLogEntry> log = goldenLog(LibiioVersion::V0);
	const CommandLogEntry write = log.takeFirst(); // attr write, the oldest
	log.append(write);			       // passed last, still emitted first
	const QString script = r->exportScript(log, lang(codegen::Python));
	const qsizetype w = script.indexOf(QStringLiteral(".value = \"-10.0\""));
	const qsizetype rd = script.indexOf(QStringLiteral("val = attr0.value"));
	const qsizetype reg = script.indexOf(QStringLiteral("reg_write(0x37, 0x5a)"));
	QVERIFY(w > 0 && rd > 0 && reg > 0);
	QVERIFY(w < rd);
	QVERIFY(rd < reg);
}

void TstComponentCodeEmitters::oneProloguePerLibrary()
{
	auto r = registryFor(LibiioVersion::V1);
	r->add(std::make_shared<OtherEmitter>());
	QList<CommandLogEntry> log = goldenLog(LibiioVersion::V1);
	CommandLogEntry other;
	other.origin = QStringLiteral("dongle/i2c0");
	other.startNs = log.first().startNs + 500'000; // between the first two libiio commands
	CallRecord oc;
	oc.library = QStringLiteral("other");
	oc.function = QStringLiteral("other_i2c_write");
	other.calls = {oc, oc};
	log.insert(1, other);
	const QString script = r->exportScript(log, lang(codegen::C));
	QCOMPARE(script.count(QStringLiteral("iio_create_context(NULL")), 1);
	QCOMPARE(script.count(QStringLiteral("other_init();")), 1);
	QCOMPARE(script.count(QStringLiteral("other_exit();")), 1);
	QCOMPARE(script.count(QStringLiteral("#include <other.h>")), 1);
	QCOMPARE(script.count(QStringLiteral("#include <iio/iio.h>")), 1);
	// Time order across libraries: the other library's calls sit between the attribute write and read.
	const qsizetype write = script.indexOf(QStringLiteral("iio_attr_write_raw("));
	const qsizetype mid = script.indexOf(QStringLiteral("other_i2c_write();"));
	const qsizetype read = script.indexOf(QStringLiteral("iio_attr_read_raw("));
	QVERIFY(write > 0 && write < mid && mid < read);
	// Its two identical calls in one command: a loop, as for any library.
	QVERIFY(!script.contains(QStringLiteral("other_i2c_write();\nother_i2c_write();")));
	// Epilogues in reverse library order: other closes before the libiio context.
	QVERIFY(script.indexOf(QStringLiteral("other_exit();")) <
		script.indexOf(QStringLiteral("iio_context_destroy(ctx);")));
}

void TstComponentCodeEmitters::libraryWithoutEmitterIsComment()
{
	auto r = registryFor(LibiioVersion::V1);
	r->add(std::make_shared<OtherEmitter>());
	CommandLogEntry other;
	other.origin = QStringLiteral("dongle/i2c0");
	CallRecord oc;
	oc.library = QStringLiteral("other");
	oc.function = QStringLiteral("other_i2c_write");
	other.calls = {oc};
	QList<CommandLogEntry> log = gainReads({QStringLiteral("1")});
	log << other;
	const QString script = r->exportScript(log, lang(codegen::Python));
	QVERIFY(script.contains(QStringLiteral("# other_i2c_write() = 0: no Python emitter for other")));
	QVERIFY(script.contains(QStringLiteral("# other: no Python emitter, its calls are comments")));
	QVERIFY(script.contains(QStringLiteral("ctx = iio.Context(\"ip:192.168.2.1\")")));
}

void TstComponentCodeEmitters::headerWarnsAboutState()
{
	auto r = registryFor(LibiioVersion::V0);
	for(const char *l : {codegen::C, codegen::Python}) {
		const QString script = r->exportScript(gainReads({QStringLiteral("1")}), lang(l));
		QVERIFY(script.contains(QStringLiteral("state configured before logging started")));
		QVERIFY(script.contains(QStringLiteral("Connection: ip:192.168.2.1")));
	}
}

void TstComponentCodeEmitters::loopsOnOff()
{
	auto r = registryFor(LibiioVersion::V0);
	const QList<CommandLogEntry> log = refills(3);
	EmitOptions opts;
	opts.loops = true;
	const QString on = r->exportScript(log, lang(codegen::Python), opts);
	QVERIFY(on.contains(QStringLiteral("for _ in range(3):\n    buf0.refill()\n    data = buf0.read()")));
	QCOMPARE(on.count(QStringLiteral("buf0.refill()")), 1);
	const QString c = r->exportScript(log, lang(codegen::C), opts);
	QVERIFY(c.contains(QStringLiteral("\tfor (int i = 0; i < 3; i++) {\n\t\tret = iio_buffer_refill(buf0);\n\t}")));
	opts.loops = false;
	const QString off = r->exportScript(log, lang(codegen::Python), opts);
	QVERIFY(!off.contains(QStringLiteral("for _ in range")));
	QCOMPARE(off.count(QStringLiteral("buf0.refill()")), 3);
	// A single refill is never a loop.
	opts.loops = true;
	QVERIFY(!r->exportScript(refills(1), lang(codegen::Python), opts).contains(QStringLiteral("for _ in")));
}

void TstComponentCodeEmitters::sameHandleSameVariable()
{
	auto r = registryFor(LibiioVersion::V1);
	Log log;
	log.entries = gainReads({QStringLiteral("1")});
	log.add(PATH_REGR, "RegReadCommand", regScope("RegisterReader"),
		{rec("iio_device_reg_read", H_PHY,
		     {CallArg::string("device", "ad9361-phy"), CallArg::hex("address", 1)})});
	// A second read of the same attribute handle, after something else.
	log.entries << gainReads({QStringLiteral("2")});
	log.entries.last().startNs = log.entries[1].startNs + 1'000'000;
	const QString py = r->exportScript(log.entries, lang(codegen::Python));
	QCOMPARE(py.count(QStringLiteral("dev0 = ctx.find_device(\"ad9361-phy\")")), 1);
	QCOMPARE(py.count(QStringLiteral("attr0 = ch0.attrs[\"hardwaregain\"]")), 1);
	QCOMPARE(py.count(QStringLiteral("val = attr0.value")), 2);
	QVERIFY(!py.contains(QStringLiteral("attr1")));
	QVERIFY(!py.contains(QStringLiteral("dev1")));
}

void TstComponentCodeEmitters::unresolvedAttributeIsComment()
{
	auto r = registryFor(LibiioVersion::V1);
	QList<CommandLogEntry> log = gainReads({QStringLiteral("1")});
	log[0].originScope.clear(); // no origin: v1 records only carry the attribute name
	for(const char *l : {codegen::C, codegen::Python}) {
		const QString script = r->exportScript(log, lang(l));
		QVERIFY2(script.contains(QStringLiteral(
				 "iio_attr_read_raw(hardwaregain, len=4096) = 2: cannot resolve the attribute")),
			 qPrintable(script));
		QVERIFY(!script.contains(QStringLiteral("attr0")));
	}
}

void TstComponentCodeEmitters::failedCallIsCaught()
{
	auto r = registryFor(LibiioVersion::V0);
	Log log;
	log.add(PATH_GAIN, "AttrWriteCommand", channelAttrScope(),
		{rec("iio_channel_attr_write", H_CHN_OUT,
		     {CallArg::string("channel", "voltage0"), CallArg::string("attr", "hardwaregain"),
		      CallArg::string("src", "x")},
		     -22)});
	const QString py = r->exportScript(log.entries, lang(codegen::Python));
	QVERIFY2(py.contains(QStringLiteral("try:\n    attr0.value = \"x\"\nexcept OSError as e:\n"
					    "    print(\"expected failure (recorded: failed with rc = -22):\", e)")),
		 qPrintable(py));
	const QString c = r->exportScript(log.entries, lang(codegen::C));
	QVERIFY(c.contains(QStringLiteral(
		"// recorded: failed with rc = -22\n\tret = iio_channel_attr_write(ch0, \"hardwaregain\", \"x\");")));
}

void TstComponentCodeEmitters::partialPayloadNotWritten()
{
	auto r = registryFor(LibiioVersion::V0);
	Log log;
	log.entries = refills(0);
	log.add(PATH_STREAM, "BufferPushCommand", streamScope(),
		{rec("iio_buffer_push", H_BUF,
		     {CallArg::bytes("data", QByteArray(4, 'a')), CallArg::integer("size", 4096)}, 4096)});
	log.add(PATH_STREAM, "BufferPushCommand", streamScope(),
		{rec("iio_buffer_push", H_BUF,
		     {CallArg::bytes("data", QByteArray("\x01\x02", 2)), CallArg::integer("size", 2)}, 2)});
	const QString py = r->exportScript(log.entries, lang(codegen::Python));
	QVERIFY(py.contains(QStringLiteral("# payload: 4 of 4096 bytes logged, not written")));
	QVERIFY(py.contains(QStringLiteral("buf0.write(bytearray(bytes([0x01, 0x02])))\nbuf0.push()")));
	const QString c = r->exportScript(log.entries, lang(codegen::C));
	QVERIFY(c.contains(QStringLiteral("static const uint8_t data0[] = { 0x01, 0x02 };")));
	QVERIFY(c.contains(QStringLiteral("memcpy(iio_buffer_start(buf0), data0, sizeof(data0));")));
}

void TstComponentCodeEmitters::literalsPerKind()
{
	using namespace codegen;
	const QString c = lang(C), py = lang(Python);
	for(const QString &l : {c, py}) {
		QCOMPARE(literal(l, quint64(0x50), CallArg::Hex), QStringLiteral("0x50"));
		QCOMPARE(literal(l, qint64(-12), CallArg::Int), QStringLiteral("-12"));
		QCOMPARE(literal(l, qint64(4096), CallArg::OutLen), QStringLiteral("4096"));
		QCOMPARE(literal(l, QVariant::fromValue(quintptr(0xdead)), CallArg::Handle), QStringLiteral("0xdead"));
	}
	QCOMPARE(literal(py, true, CallArg::Bool), QStringLiteral("True"));
	QCOMPARE(literal(c, false, CallArg::Bool), QStringLiteral("false"));
	const QByteArray b("\x00\x7f\xff", 3);
	QCOMPARE(literal(py, b, CallArg::Bytes), QStringLiteral("bytes([0x00, 0x7f, 0xff])"));
	QCOMPARE(literal(c, b, CallArg::Bytes), QStringLiteral("{ 0x00, 0x7f, 0xff }"));
	QCOMPARE(bytesLiteral(py, {}), QStringLiteral("bytes([])"));
	// Long payloads wrap at 16 bytes per line.
	const QString wrapped = bytesLiteral(py, QByteArray(17, '\x01'));
	QCOMPARE(wrapped.count('\n'), 3);
	QVERIFY(wrapped.startsWith(QStringLiteral("bytes([\n    0x01, ")));
	QVERIFY(wrapped.endsWith(QStringLiteral("\n    0x01\n])")));
	QCOMPARE(literal(py, QStringLiteral("ab"), CallArg::String), QStringLiteral("\"ab\""));
	QCOMPARE(fileExtension(py), QStringLiteral("py"));
	QCOMPARE(fileExtension(c), QStringLiteral("c"));
}

void TstComponentCodeEmitters::stringEscaping()
{
	using namespace codegen;
	const QString c = lang(C), py = lang(Python);
	const QString tricky = QStringLiteral("a\"b\\c\nd\te\r") + QChar(0x01);
	QCOMPARE(stringLiteral(c, tricky), QStringLiteral("\"a\\\"b\\\\c\\nd\\te\\r\\001\""));
	QCOMPARE(stringLiteral(py, tricky), QStringLiteral("\"a\\\"b\\\\c\\nd\\te\\r\\x01\""));
	// Non-ASCII: UTF-8 octal in C (a following digit is not swallowed), \x / \U in Python.
	const QString micro = QString(QChar(0x00b5)) + QStringLiteral("1");
	QCOMPARE(stringLiteral(c, micro), QStringLiteral("\"\\302\\2651\""));
	QCOMPARE(stringLiteral(py, micro), QStringLiteral("\"\\xb51\""));
	QCOMPARE(stringLiteral(py, QString::fromUcs4(U"\U0001F600")), QStringLiteral("\"\\U0001f600\""));
	// No trigraphs in C.
	QCOMPARE(stringLiteral(c, QStringLiteral("??=")), QStringLiteral("\"?\\?=\""));
	QCOMPARE(stringLiteral(py, QStringLiteral("??=")), QStringLiteral("\"??=\""));
	// An escaped value in generated code.
	Log log;
	log.add(PATH_GAIN, "AttrWriteCommand", channelAttrScope(),
		{rec("iio_attr_write_raw", H_ATTR,
		     {CallArg::string("attr", "hardwaregain"), CallArg::string("src", "say \"hi\"\n"),
		      CallArg::integer("len", 9)})});
	auto r = registryFor(LibiioVersion::V1);
	QVERIFY(r->exportScript(log.entries, py).contains(QStringLiteral("attr0.value = \"say \\\"hi\\\"\\n\"")));
	QVERIFY(r->exportScript(log.entries, c)
			.contains(QStringLiteral("iio_attr_write_raw(attr0, \"say \\\"hi\\\"\\n\", 9);")));
}

void TstComponentCodeEmitters::comments()
{
	using namespace codegen;
	QCOMPARE(comment(lang(Python), QStringLiteral("a\nb")), QStringLiteral("# a\n# b"));
	QCOMPARE(comment(lang(C), QStringLiteral("x\n\ny")), QStringLiteral("// x\n//\n// y"));
	QCOMPARE(comment(lang(C), QStringLiteral("path\\")), QStringLiteral("// path\\."));
	QCOMPARE(loopBegin(lang(Python), 4), QStringLiteral("for _ in range(4):"));
	QCOMPARE(loopEnd(lang(Python)), QString());
}

QTEST_MAIN(TstComponentCodeEmitters)
#include "tst_component_codeemitters.moc"
