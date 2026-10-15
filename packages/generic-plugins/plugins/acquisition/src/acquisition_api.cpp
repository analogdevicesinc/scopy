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

#include "acquisition_api.h"

#include "acqaxis.h"
#include "acqchannel.h"
#include "acqinstrument.h"
#include "acqinstrumentcontroller.h"
#include "acqplot.h"
#include "acqplotmanager.h"

#include "acquisition.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <cmath>
#include <core/decoder/decodermanager.h>
#include <core/decoder/decoderpanel.h>
#include <pluginbase/scopyjs.h>

using namespace scopy;
using namespace scopy::acq;
using namespace scopy::adc;
using namespace scopy::acquisition;

static Acquisition_API *s_api = nullptr;

// Numeric samples, at the stream's own precision. Annotation chunks give none.
static QVariantList toList(const SampleVariant &v)
{
	QVariantList out;
	std::visit(
		[&out](const auto &vec) {
			using T = typename std::decay_t<decltype(vec)>::value_type;
			if constexpr(!std::is_same_v<T, Annotation>) {
				out.reserve(vec.size());
				for(const T &s : vec) {
					out.append(static_cast<double>(s));
				}
			}
		},
		v);
	return out;
}

static QStringList toStrings(const QList<DataKey> &keys)
{
	QStringList out;
	for(const DataKey &k : keys) {
		out.append(k.toString());
	}
	return out;
}

static bool reprKindFromString(const QString &s, ReprKind &out)
{
	for(ReprKind k :
	    {ReprKind::Hidden, ReprKind::Curve, ReprKind::Digital, ReprKind::Annotations, ReprKind::Waterfall}) {
		if(s.compare(reprKindName(k), Qt::CaseInsensitive) == 0) {
			out = k;
			return true;
		}
	}
	return false;
}

void Acquisition_API::attach(AcquisitionPlugin *plugin)
{
	if(!s_api) {
		s_api = new Acquisition_API();
		s_api->setObjectName("acq");
		ScopyJS::GetInstance()->registerApi(s_api);
	}
	s_api->m_plugins.removeAll(plugin);
	s_api->m_plugins.append(plugin);
	s_api->m_current = plugin;
}

void Acquisition_API::detach(AcquisitionPlugin *plugin)
{
	if(!s_api) {
		return;
	}
	s_api->m_plugins.removeAll(plugin);
	if(s_api->m_current == plugin) {
		s_api->m_current = s_api->m_plugins.isEmpty() ? nullptr : s_api->m_plugins.last();
	}
	if(s_api->m_plugins.isEmpty()) {
		ScopyJS::GetInstance()->unregisterApi(s_api);
		delete s_api;
		s_api = nullptr;
	}
}

AcqInstrumentController *Acquisition_API::ctrl() const
{
	if(!m_current || m_current->controllers().isEmpty()) {
		return nullptr;
	}
	return m_current->controllers().first();
}

Block *Acquisition_API::block(const QString &name) const
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return nullptr;
	}
	for(SourceBlock *b : c->ui()->engine()->sources()) {
		if(b->name() == name) {
			return b;
		}
	}
	for(ProcessorBlock *b : c->ui()->engine()->processors()) {
		if(b->name() == name) {
			return b;
		}
	}
	return nullptr;
}

AcqPlot *Acquisition_API::plot(int uuid) const
{
	AcqInstrumentController *c = ctrl();
	return c ? c->plots()->plot(static_cast<quint32>(uuid)) : nullptr;
}

AcqChannel *Acquisition_API::channel(int uid) const
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return nullptr;
	}
	for(AcqChannel *ch : c->plots()->channels()) {
		if(ch->uid() == uid) {
			return ch;
		}
	}
	return nullptr;
}

decoder::DecoderEditor *Acquisition_API::decoderEditor(const QString &uid) const
{
	AcqInstrumentController *c = ctrl();
	return c && c->decoders() ? c->decoders()->editor(uid) : nullptr;
}

// ---- devices ------------------------------------------------------------------

QStringList Acquisition_API::getDevices()
{
	QStringList out;
	for(AcquisitionPlugin *p : std::as_const(m_plugins)) {
		out.append(p->param());
	}
	return out;
}

QString Acquisition_API::getDevice() { return m_current ? m_current->param() : QString(); }

bool Acquisition_API::setDevice(const QString &uri)
{
	for(AcquisitionPlugin *p : std::as_const(m_plugins)) {
		if(p->param() == uri) {
			m_current = p;
			return true;
		}
	}
	return false;
}

// ---- run control --------------------------------------------------------------

void Acquisition_API::run()
{
	if(AcqInstrumentController *c = ctrl()) {
		c->ui()->run();
	}
}

void Acquisition_API::stop()
{
	if(AcqInstrumentController *c = ctrl()) {
		c->stop();
	}
}

void Acquisition_API::single()
{
	if(AcqInstrumentController *c = ctrl()) {
		c->single();
	}
}

bool Acquisition_API::singleAndWait(int timeoutMs)
{
	AcqInstrumentController *c = ctrl();
	if(!c || c->ui()->engine()->isRunning()) {
		return false;
	}
	// A previous run's stop is posted from the worker, queued; delivered now, it cannot end
	// the wait for this one.
	QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);

	// The engine reports from its worker, queued, so nothing is missed by connecting first.
	// Its own signals, not the instrument's: those fold a fault stop into a plain one.
	AcquisitionEngine *engine = c->ui()->engine();
	QEventLoop loop;
	connect(engine, &AcquisitionEngine::stopped, &loop, [&loop]() { loop.exit(0); });
	connect(engine, &AcquisitionEngine::forceStopped, &loop, [&loop]() { loop.exit(2); });
	QTimer::singleShot(timeoutMs, &loop, [&loop]() { loop.exit(1); });
	c->single();
	const int rc = loop.exec();
	if(rc == 1) {
		c->stop();
	}
	return rc == 0;
}

bool Acquisition_API::isRunning()
{
	AcqInstrumentController *c = ctrl();
	return c && c->ui()->engine()->isRunning();
}

bool Acquisition_API::setMode(const QString &mode)
{
	// Refused like the UI greys the combo: the engine does not expect a mode change mid-run.
	AcqInstrumentController *c = ctrl();
	if(!c || c->ui()->engine()->isRunning()) {
		return false;
	}
	if(mode.compare("continuous", Qt::CaseInsensitive) == 0) {
		c->ui()->setMode(static_cast<int>(AcquisitionEngine::Mode::Continuous));
	} else if(mode.compare("triggered", Qt::CaseInsensitive) == 0) {
		c->ui()->setMode(static_cast<int>(AcquisitionEngine::Mode::Triggered));
	} else {
		return false;
	}
	return true;
}

QString Acquisition_API::getMode()
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return QString();
	}
	return c->ui()->engine()->mode() == AcquisitionEngine::Mode::Continuous ? "continuous" : "triggered";
}

bool Acquisition_API::setBufferSize(int n)
{
	AcqInstrumentController *c = ctrl();
	// The spinbox clamps but takes any value typed in, so the documented range is enforced here.
	if(!c || c->ui()->engine()->isRunning() || n < 16 || n > (1 << 20) || (n & (n - 1))) {
		return false;
	}
	c->ui()->setBufferSize(n);
	return static_cast<int>(c->ui()->engine()->bufferSize()) == n;
}

int Acquisition_API::getBufferSize()
{
	AcqInstrumentController *c = ctrl();
	return c ? static_cast<int>(c->ui()->engine()->bufferSize()) : 0;
}

// ---- pipeline -----------------------------------------------------------------

QStringList Acquisition_API::getSources()
{
	QStringList out;
	if(AcqInstrumentController *c = ctrl()) {
		for(SourceBlock *b : c->ui()->engine()->sources()) {
			out.append(b->name());
		}
	}
	return out;
}

QStringList Acquisition_API::getProcessors()
{
	QStringList out;
	if(AcqInstrumentController *c = ctrl()) {
		for(ProcessorBlock *b : c->ui()->engine()->processors()) {
			out.append(b->name());
		}
	}
	return out;
}

QVariantMap Acquisition_API::getBlockInfo(const QString &name)
{
	Block *b = block(name);
	if(!b) {
		return {};
	}
	ProcessorBlock *proc = qobject_cast<ProcessorBlock *>(b);
	QVariantMap out{{"enabled", b->isEnabled()},
			{"source", proc == nullptr},
			{"in", proc ? toStrings(proc->watchedKeys()) : QStringList()},
			{"out", toStrings(b->outputKeys())}};
	if(SourceBlock *src = qobject_cast<SourceBlock *>(b)) {
		out["channels"] = QStringList(src->enabledChannels());
	}
	return out;
}

bool Acquisition_API::setBlockEnabled(const QString &name, bool en)
{
	Block *b = block(name);
	if(!b) {
		return false;
	}
	b->setEnabled(en);
	return true;
}

bool Acquisition_API::setBlockInputs(const QString &name, const QString &keys)
{
	ProcessorBlock *proc = qobject_cast<ProcessorBlock *>(block(name));
	// The worker reads watchedKeys() of every enabled block, so the retarget contract is
	// "stopped or disabled", as the block's own settings panel enforces.
	if(!proc || (proc->isEnabled() && ctrl()->ui()->engine()->isRunning())) {
		return false;
	}
	QList<DataKey> list;
	for(const QString &k : keys.split(',', Qt::SkipEmptyParts)) {
		list.append(DataKey(k.trimmed()));
	}
	proc->setWatchedKeys(list);
	// Blocks may reject a list (the FFT takes at most two keys) without saying so.
	return proc->watchedKeys() == list;
}

QStringList Acquisition_API::getSourceChannels(const QString &source)
{
	SourceBlock *src = qobject_cast<SourceBlock *>(block(source));
	return src ? QStringList(src->channelIds()) : QStringList();
}

bool Acquisition_API::setSourceChannelEnabled(const QString &source, const QString &channel, bool en)
{
	SourceBlock *src = qobject_cast<SourceBlock *>(block(source));
	if(!src || !src->channelIds().contains(channel)) {
		return false;
	}
	src->enableChannel(channel, en);
	return true;
}

// ---- decoders -----------------------------------------------------------------

QStringList Acquisition_API::getDecoderIds()
{
	AcqInstrumentController *c = ctrl();
	if(!c || !c->decoders() || !c->decoders()->catalog()) {
		return {};
	}
	QStringList ids = c->decoders()->catalog()->decoders();
	ids.sort();
	return ids;
}

QVariantMap Acquisition_API::getDecoderInfo(const QString &decoderId)
{
	AcqInstrumentController *c = ctrl();
	if(!c || !c->decoders() || !c->decoders()->catalog() ||
	   !c->decoders()->catalog()->decoders().contains(decoderId)) {
		return {};
	}
	const decoder::DecoderInfo info = c->decoders()->catalog()->info(decoderId);
	QVariantList channels, options;
	for(const decoder::ChannelInfo &ch : info.channels) {
		channels.append(QVariantMap{{"id", ch.id}, {"name", ch.name}, {"required", ch.required}});
	}
	for(const decoder::OptionInfo &o : info.options) {
		options.append(QVariantMap{
			{"id", o.id}, {"name", o.name}, {"default", o.defaultValue}, {"choices", o.choices}});
	}
	return {{"id", info.id},	   {"name", info.name},		{"description", info.description},
		{"inputs", info.inputIds}, {"outputs", info.outputIds}, {"channels", channels},
		{"options", options}};
}

QString Acquisition_API::addDecoder(const QString &decoderId)
{
	AcqInstrumentController *c = ctrl();
	return c && c->decoders() ? c->decoders()->addDecoder(decoderId) : QString();
}

void Acquisition_API::removeDecoder(const QString &uid)
{
	if(decoderEditor(uid)) {
		ctrl()->decoders()->removeDecoder(uid);
	}
}

QVariantList Acquisition_API::getDecoders()
{
	QVariantList out;
	AcqInstrumentController *c = ctrl();
	if(!c || !c->decoders()) {
		return out;
	}
	for(const decoder::DecoderInstance &d : c->decoders()->manager()->decoders()) {
		out.append(QVariantMap{{"uid", d.uid},
				       {"stages", d.stageIds},
				       {"inputs", toStrings(d.orderedRawKeys)},
				       {"outputs", toStrings(d.outKeys)}});
	}
	return out;
}

bool Acquisition_API::setDecoderChannel(const QString &uid, const QString &role, const QString &key)
{
	decoder::DecoderEditor *e = decoderEditor(uid);
	return e && e->setChannel(role, key);
}

bool Acquisition_API::setDecoderOption(const QString &uid, const QString &option, const QString &value)
{
	decoder::DecoderEditor *e = decoderEditor(uid);
	return e && e->setOption(option, value);
}

bool Acquisition_API::setDecoderSampleRate(const QString &uid, double hz)
{
	decoder::DecoderEditor *e = decoderEditor(uid);
	return e && e->setSampleRate(hz);
}

// ---- datastore ----------------------------------------------------------------

QStringList Acquisition_API::getKeys()
{
	AcqInstrumentController *c = ctrl();
	return c ? toStrings(c->ui()->store()->keys()) : QStringList();
}

QStringList Acquisition_API::getDeclaredKeys()
{
	AcqInstrumentController *c = ctrl();
	return c ? toStrings(c->ui()->engine()->declaredKeys()) : QStringList();
}

bool Acquisition_API::hasKey(const QString &key)
{
	AcqInstrumentController *c = ctrl();
	return c && c->ui()->store()->contains(DataKey(key));
}

QVariantMap Acquisition_API::getKeyInfo(const QString &key)
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return {};
	}
	const DataKey k(key);
	const SampleBuffer buf = c->ui()->store()->snapshot(k);
	QVariantMap out{{"key", key},
			{"source", k.sourceId()},
			{"channel", k.channelId()},
			{"stage", k.stage()},
			{"present", c->ui()->store()->contains(k)},
			{"type", buf.type() ? sampleTypeName(*buf.type()) : QString()},
			{"depth", static_cast<int>(buf.depth())},
			{"capacity", static_cast<int>(buf.capacity())},
			{"chunkSize", static_cast<int>(buf.size())}};
	if(std::optional<StreamInfo> info = c->ui()->engine()->streamInfo(k)) {
		out["label"] = info->label;
		out["unit"] = info->unit;
		out["sampleRate"] = info->sampleRate;
		out["xKey"] = info->xKey.toString();
		out["kind"] = reprKindName(info->kind);
	}
	if(std::optional<AnnotationStreamInfo> ann = c->ui()->store()->annotationInfo(k)) {
		out["annotation"] = QVariantMap{{"producer", ann->producerId},
						{"radix", textRadixName(ann->textRadix)},
						{"sampleRate", ann->sampleRate},
						{"bitrate", static_cast<double>(ann->bitrate)}};
	}
	return out;
}

double Acquisition_API::getWriteCount()
{
	AcqInstrumentController *c = ctrl();
	return c ? static_cast<double>(c->ui()->store()->writeCount()) : 0;
}

QVariantList Acquisition_API::getLatest(const QString &key)
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return {};
	}
	std::optional<SampleVariant> v = c->ui()->store()->latest(DataKey(key));
	return v ? toList(*v) : QVariantList();
}

QVariantList Acquisition_API::getWindow(const QString &key, int n)
{
	AcqInstrumentController *c = ctrl();
	return c ? toList(c->ui()->store()->window(DataKey(key), n)) : QVariantList();
}

QVariantList Acquisition_API::getChunk(const QString &key, int index)
{
	AcqInstrumentController *c = ctrl();
	if(!c || index < 0) {
		return {};
	}
	return toList(c->ui()->store()->snapshot(DataKey(key)).sample(static_cast<std::size_t>(index)));
}

QVariantMap Acquisition_API::getStats(const QString &key, int n)
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return {};
	}
	const QVector<float> v = c->ui()->store()->windowFloat(DataKey(key), n);
	if(v.isEmpty()) {
		return {{"count", 0}};
	}
	double mn = v.first(), mx = v.first(), sum = 0, sq = 0;
	for(float s : v) {
		mn = std::min(mn, double(s));
		mx = std::max(mx, double(s));
		sum += s;
		sq += double(s) * s;
	}
	return {{"count", v.size()},
		{"min", mn},
		{"max", mx},
		{"mean", sum / v.size()},
		{"rms", std::sqrt(sq / v.size())},
		{"pp", mx - mn}};
}

QVariantList Acquisition_API::getAnnotations(const QString &key, int max)
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return {};
	}
	std::optional<QVector<Annotation>> anns = c->ui()->store()->latestAs<QVector<Annotation>>(DataKey(key));
	QVariantList out;
	if(!anns) {
		return out;
	}
	// Newest = last emitted: the chunk is in the decoder's output order, so the tail.
	const qsizetype first = max > 0 ? std::max<qsizetype>(0, anns->size() - max) : 0;
	for(qsizetype i = first; i < anns->size(); i++) {
		const Annotation &a = anns->at(i);
		QVariantMap m{{"start", static_cast<double>(a.startSample)},
			      {"end", static_cast<double>(a.endSample)},
			      {"decoder", a.decoder},
			      {"class", a.klass},
			      {"text", a.text},
			      {"severity", a.severity}};
		if(a.value) {
			m["value"] = static_cast<double>(*a.value);
		}
		out.append(m);
	}
	return out;
}

bool Acquisition_API::saveCsv(const QString &keys, int n, const QString &path)
{
	AcqInstrumentController *c = ctrl();
	QFile f(path);
	if(!c || !f.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	const QStringList names = keys.split(',', Qt::SkipEmptyParts);
	QList<QVector<float>> cols;
	int rows = 0;
	for(const QString &k : names) {
		cols.append(c->ui()->store()->windowFloat(DataKey(k.trimmed()), n));
		rows = std::max(rows, static_cast<int>(cols.last().size()));
	}
	QTextStream out(&f);
	// 9 significant digits round-trip any float; the default 6 do not.
	out.setRealNumberPrecision(9);
	out << names.join(',') << '\n';
	for(int r = 0; r < rows; r++) {
		for(int i = 0; i < cols.size(); i++) {
			if(i) {
				out << ',';
			}
			if(r < cols[i].size()) {
				out << cols[i][r];
			}
		}
		out << '\n';
	}
	return true;
}

void Acquisition_API::claimDepth(const QString &key, int chunks)
{
	if(AcqInstrumentController *c = ctrl()) {
		c->ui()->store()->claimDepth(DataKey(key), "js",
					     DataStore::Claim::chunks(static_cast<std::size_t>(std::max(chunks, 1))));
	}
}

void Acquisition_API::releaseDepth(const QString &key)
{
	if(AcqInstrumentController *c = ctrl()) {
		c->ui()->store()->releaseDepth(DataKey(key), "js");
	}
}

void Acquisition_API::clearStore()
{
	if(AcqInstrumentController *c = ctrl()) {
		c->ui()->store()->clear();
		// The sample-index ramp is a stream like any other, published only at a run's
		// start or on a resize; without this every default X axis stays empty until then.
		AcquisitionEngine *engine = c->ui()->engine();
		engine->setIndexRampLength(engine->indexRampLength());
	}
}

// ---- plots --------------------------------------------------------------------

QVariantList Acquisition_API::getPlots()
{
	QVariantList out;
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return out;
	}
	for(AcqPlot *p : c->plots()->plots()) {
		QVariantList chs;
		for(AcqChannel *ch : p->channels()) {
			chs.append(ch->uid());
		}
		out.append(QVariantMap{{"uuid", p->uuid()},
				       {"name", p->name()},
				       {"kind", p->kind() == AcqPlotKind::Waterfall ? "waterfall" : "basic"},
				       {"channels", chs}});
	}
	return out;
}

int Acquisition_API::addPlot(const QString &name, const QString &kind)
{
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return -1;
	}
	const AcqPlotKind k =
		kind.compare("waterfall", Qt::CaseInsensitive) == 0 ? AcqPlotKind::Waterfall : AcqPlotKind::Basic;
	return static_cast<int>(c->plots()->addPlot(name, k)->uuid());
}

void Acquisition_API::removePlot(int uuid)
{
	if(AcqInstrumentController *c = ctrl()) {
		c->plots()->removePlot(static_cast<quint32>(uuid));
	}
}

void Acquisition_API::setPlotName(int uuid, const QString &name)
{
	if(AcqPlot *p = plot(uuid)) {
		p->setName(name);
	}
}

void Acquisition_API::setPlotLegend(int uuid, bool on)
{
	if(AcqPlot *p = plot(uuid)) {
		p->setShowLegend(on);
	}
}

void Acquisition_API::setPlotLabels(int uuid, bool on)
{
	if(AcqPlot *p = plot(uuid)) {
		p->setShowLabels(on);
	}
}

void Acquisition_API::setPlotCursors(int uuid, bool on)
{
	if(AcqPlot *p = plot(uuid)) {
		p->setShowCursors(on);
	}
}

void Acquisition_API::setPlotSize(int n)
{
	if(AcqInstrumentController *c = ctrl()) {
		c->plots()->setPlotSize(n);
	}
}

int Acquisition_API::getPlotSize()
{
	AcqInstrumentController *c = ctrl();
	return c ? c->plots()->plotSize() : 0;
}

// ---- plot channels ------------------------------------------------------------

QVariantList Acquisition_API::getPlotChannels()
{
	QVariantList out;
	AcqInstrumentController *c = ctrl();
	if(!c) {
		return out;
	}
	for(AcqChannel *ch : c->plots()->channels()) {
		out.append(QVariantMap{{"uid", ch->uid()},
				       {"plot", ch->plotOwner() ? QVariant(ch->plotOwner()->uuid()) : QVariant()},
				       {"kind", ch->kindName().toLower()},
				       {"yKey", ch->key().toString()},
				       {"xKey", ch->xKey().toString()},
				       {"name", ch->name()},
				       {"color", ch->color().name()},
				       {"enabled", ch->isEnabled()},
				       {"keyPresent", ch->isKeyPresent()},
				       {"sampleRate", ch->sampleRate()}});
	}
	return out;
}

int Acquisition_API::addPlotChannel(int plotUuid, const QString &kind, const QString &yKey, const QString &xKey)
{
	AcqInstrumentController *c = ctrl();
	ReprKind k;
	if(!c || !reprKindFromString(kind, k)) {
		return -1;
	}
	AcqChannel *ch = c->plots()->addChannel(plot(plotUuid), k, DataKey(yKey), DataKey(xKey));
	return ch ? ch->uid() : -1;
}

void Acquisition_API::removePlotChannel(int uid)
{
	AcqChannel *ch = channel(uid);
	if(ch) {
		ctrl()->plots()->removeChannel(ch);
	}
}

void Acquisition_API::setPlotChannelEnabled(int uid, bool en)
{
	if(AcqChannel *ch = channel(uid)) {
		ch->setEnabled(en);
	}
}

void Acquisition_API::setPlotChannelName(int uid, const QString &name)
{
	if(AcqChannel *ch = channel(uid)) {
		ch->setName(name);
	}
}

void Acquisition_API::setPlotChannelColor(int uid, const QString &color)
{
	AcqChannel *ch = channel(uid);
	if(ch && QColor::isValidColorName(color)) {
		ch->setColor(QColor(color));
	}
}

// ---- plot channel axes --------------------------------------------------------

static AcqAxis *axisOf(AcqChannel *ch, const QString &axis)
{
	if(!ch) {
		return nullptr;
	}
	if(axis.compare("x", Qt::CaseInsensitive) == 0) {
		return ch->xAxis();
	}
	return axis.compare("y", Qt::CaseInsensitive) == 0 ? ch->yAxis() : nullptr;
}

void Acquisition_API::setAxisRange(int uid, const QString &axis, double min, double max)
{
	if(AcqAxis *a = axisOf(channel(uid), axis)) {
		a->setAutoscale(false);
		a->setInterval(min, max);
	}
}

void Acquisition_API::setAxisAutoscale(int uid, const QString &axis, bool on)
{
	if(AcqAxis *a = axisOf(channel(uid), axis)) {
		a->setAutoscale(on);
	}
}

QVariantMap Acquisition_API::getAxis(int uid, const QString &axis)
{
	AcqAxis *a = axisOf(channel(uid), axis);
	if(!a) {
		return {};
	}
	return {{"min", a->min()},
		{"max", a->max()},
		{"autoscale", a->autoscale()},
		{"unit", a->unit()},
		{"source", a->source().id()}};
}

#include "moc_acquisition_api.cpp"
