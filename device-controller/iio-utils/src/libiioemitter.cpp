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

#include "iioutil/libiioemitter.h"

using namespace scopy;
using namespace scopy::iio;

namespace {

constexpr auto RESOURCES = "libiio/resources"; // "kind|var" in creation order

// What an attribute call acts on, resolved from the record (v0) or the origin scope (v1).
struct AttrTarget
{
	enum Scope
	{
		None,
		Context,
		Device,
		Channel,
		Debug,
		Buffer
	};
	Scope scope = None;
	QString device;
	QString channel;
	bool output = false;
	int bufferIndex = 0;
	QString attr;
	QString why; // when scope == None

	QString label() const
	{
		QStringList parts;
		if(scope != Context && !device.isEmpty()) {
			parts << device;
		}
		if(scope == Channel) {
			parts << channel;
		}
		if(scope == Buffer) {
			parts << QStringLiteral("buffer%1").arg(bufferIndex);
		}
		parts << attr;
		const QString l = parts.join(QLatin1Char('/'));
		return scope == Debug ? QStringLiteral("debug ") + l : l;
	}
};

// One emitter call: language / version switches and the shared helpers.
class Gen
{
public:
	Gen(LibiioVersion v, const QString &language, EmitContext &ctx)
		: v1(v == LibiioVersion::V1)
		, c(language == QLatin1String(codegen::C))
		, py(language == QLatin1String(codegen::Python))
		, lang(language)
		, ctx(ctx)
	{}

	const bool v1;
	const bool c, py;
	const QString lang;
	EmitContext &ctx;

	// --- text ---
	QString s(const QString &text) const { return codegen::stringLiteral(lang, text); }
	QString b(bool value) const { return codegen::boolLiteral(lang, value); }
	QString hex(quint64 value) const { return codegen::hexLiteral(value); }
	QString st(const QString &code) const { return py ? code : code + QLatin1Char(';'); }
	QString comment(const QString &text) const { return ctx.comment(text); }
	QString unit() const { return codegen::indentUnit(lang); }
	QString null() const { return py ? QStringLiteral("None") : QStringLiteral("NULL"); }

	// "Type var = expr;" (Python "var = expr"); in a loop body the variable is declared before the
	// loop and only assigned here, so it outlives the loop (C scopes).
	QString create(const QString &type, const QString &var, const QString &expr) const
	{
		if(py) {
			return var + QStringLiteral(" = ") + expr;
		}
		const QString decl = type.endsWith(QLatin1Char('*')) ? type + var : type + QLatin1Char(' ') + var;
		if(ctx.inLoop) {
			ctx.declare(st(decl + QStringLiteral(" = ") + null()));
			return st(var + QStringLiteral(" = ") + expr);
		}
		return st(decl + QStringLiteral(" = ") + expr);
	}

	// Scratch variables, declared once at the top scope.
	void scratch(const QString &name)
	{
		if(c) {
			if(name == QLatin1String("ret")) {
				ctx.declareOnce(QStringLiteral("libiio/c/ret"), QStringLiteral("ssize_t ret;"));
			} else if(name == QLatin1String("val")) {
				ctx.declareOnce(QStringLiteral("libiio/c/val"), QStringLiteral("char val[4096];"));
			} else if(name == QLatin1String("cval")) {
				ctx.declareOnce(QStringLiteral("libiio/c/cval"), QStringLiteral("const char *cval;"));
			} else if(name == QLatin1String("reg")) {
				ctx.declareOnce(QStringLiteral("libiio/c/reg"), QStringLiteral("uint32_t reg;"));
			} else if(name == QLatin1String("trig")) {
				ctx.declareOnce(QStringLiteral("libiio/c/trig"),
						QStringLiteral("const struct iio_device *trig;"));
			}
		}
	}

	// A recorded failure: C keeps going (comment), Python catches what the binding throws.
	QString failure(const QStringList &stmts, int rc) const
	{
		const QString note = QStringLiteral("recorded: failed with rc = %1").arg(rc);
		if(c) {
			return (QStringList{comment(note)} + stmts).join(QLatin1Char('\n'));
		}
		const QString u = unit();
		QStringList out;
		out << QStringLiteral("try:") << codegen::indent(stmts.join(QLatin1Char('\n')), u)
		    << QStringLiteral("except OSError as e:")
		    << u + QStringLiteral("print(%1, e)").arg(s(QStringLiteral("expected failure (%1):").arg(note)));
		return out.join(QLatin1Char('\n'));
	}
	QString done(const CallRecord &r, const QStringList &stmts) const
	{
		return r.failed() ? failure(stmts, r.rc) : stmts.join(QLatin1Char('\n'));
	}

	// --- record access ---
	static const CallArg *arg(const CallRecord &r, const char *name)
	{
		for(const CallArg &a : r.args) {
			if(a.name == QLatin1String(name)) {
				return &a;
			}
		}
		return nullptr;
	}
	static QString str(const CallRecord &r, const char *name)
	{
		const CallArg *a = arg(r, name);
		return a ? a->value.toString() : QString();
	}
	static qint64 num(const CallRecord &r, const char *name, qint64 fallback = 0)
	{
		const CallArg *a = arg(r, name);
		return a ? a->value.toLongLong() : fallback;
	}
	static bool flag(const CallRecord &r, const char *name)
	{
		const CallArg *a = arg(r, name);
		return a && a->value.toBool();
	}
	QVariantMap scope() const { return ctx.entry ? ctx.entry->originScope : QVariantMap(); }
	QString scopeStr(const char *key) const { return scope().value(QLatin1String(key)).toString(); }
	bool scopeBool(const char *key) const { return scope().value(QLatin1String(key)).toBool(); }

	// --- resources released by the epilogue ---
	void own(const QString &kind, const QString &var)
	{
		ctx.lists[QLatin1String(RESOURCES)] << kind + QLatin1Char('|') + var;
	}
	void release(const QString &var)
	{
		QStringList &l = ctx.lists[QLatin1String(RESOURCES)];
		for(int i = l.size() - 1; i >= 0; --i) {
			if(l[i].section(QLatin1Char('|'), 1) == var) {
				l.removeAt(i);
				break;
			}
		}
	}

	// --- objects found by name (declared once, before the statement using them) ---
	QString ctxVar() const { return QStringLiteral("ctx"); }

	QString device(const QString &name, quintptr handle = 0)
	{
		const QString known = ctx.var(handle);
		if(!known.isEmpty()) {
			return known;
		}
		if(name.isEmpty()) {
			return {};
		}
		const QString key = QStringLiteral("libiio/dev/") + name;
		QString v = ctx.names.value(key);
		if(v.isEmpty()) {
			v = ctx.newVar(QStringLiteral("dev"));
			ctx.names.insert(key, v);
			QString expr;
			if(c) {
				expr = QStringLiteral("iio_context_find_device(ctx, %1)").arg(s(name));
			} else {
				expr = QStringLiteral("ctx.find_device(%1)").arg(s(name));
			}
			ctx.declare(lookup(QStringLiteral("struct iio_device *"), v, expr));
		}
		if(handle) {
			ctx.vars.insert(handle, v);
		}
		return v;
	}

	QString channel(const QString &devVar, const QString &devName, const QString &id, bool output,
			quintptr handle = 0)
	{
		const QString known = ctx.var(handle);
		if(!known.isEmpty()) {
			return known;
		}
		if(devVar.isEmpty() || id.isEmpty()) {
			return {};
		}
		const QString key = QStringLiteral("libiio/ch/%1/%2/%3").arg(devName, id, output ? "out" : "in");
		QString v = ctx.names.value(key);
		if(v.isEmpty()) {
			v = ctx.newVar(QStringLiteral("ch"));
			ctx.names.insert(key, v);
			QString expr;
			if(c) {
				expr = QStringLiteral("iio_device_find_channel(%1, %2, %3)")
					       .arg(devVar, s(id), b(output));
			} else {
				expr = QStringLiteral("%1.find_channel(%2, %3)").arg(devVar, s(id), b(output));
			}
			ctx.declare(lookup(QStringLiteral("struct iio_channel *"), v, expr));
		}
		if(handle) {
			ctx.vars.insert(handle, v);
		}
		return v;
	}

	// v1: the device's buffer `index` (iio_device_get_buffer).
	QString buffer(const QString &devVar, const QString &devName, int index, quintptr handle = 0)
	{
		const QString known = ctx.var(handle);
		if(!known.isEmpty()) {
			return known;
		}
		if(devVar.isEmpty()) {
			return {};
		}
		const QString key = QStringLiteral("libiio/buf/%1/%2").arg(devName, QString::number(index));
		QString v = ctx.names.value(key);
		if(v.isEmpty()) {
			v = ctx.newVar(QStringLiteral("buf"));
			ctx.names.insert(key, v);
			QString expr;
			if(c) {
				expr = QStringLiteral("iio_device_get_buffer(%1, %2)").arg(devVar).arg(index);
			} else {
				expr = QStringLiteral("%1.get_buffer(%2)").arg(devVar).arg(index);
			}
			ctx.declare(lookup(QStringLiteral("struct iio_buffer *"), v, expr));
		}
		if(handle) {
			ctx.vars.insert(handle, v);
		}
		return v;
	}

	// v1: a channels mask first seen as an argument: created with the stream (not traced), over
	// every channel of the stream's device, like the backend does.
	QString mask(quintptr handle)
	{
		const QString known = ctx.var(handle);
		if(!known.isEmpty()) {
			return known;
		}
		const QString devName = scopeStr(originscope::Device);
		const QString dev = device(devName);
		if(dev.isEmpty()) {
			return {};
		}
		const QString v = ctx.bind(handle, QStringLiteral("mask"));
		QString expr;
		if(c) {
			expr = QStringLiteral("iio_create_channels_mask(iio_device_get_channels_count(%1))").arg(dev);
		} else {
			expr = QStringLiteral("iio.ChannelsMask(%1)").arg(dev);
		}
		ctx.declare(comment(QStringLiteral("%1: channels mask of %2 (created with the stream, not logged)")
					    .arg(v, devName)));
		ctx.declare(lookup(QStringLiteral("struct iio_channels_mask *"), v, expr));
		own(QStringLiteral("mask"), v);
		return v;
	}

	QString lookup(const QString &typePrefix, const QString &var, const QString &expr) const
	{
		return py ? var + QStringLiteral(" = ") + expr : st(typePrefix + var + QStringLiteral(" = ") + expr);
	}

	// --- attributes ---
	AttrTarget target(const CallRecord &r) const
	{
		AttrTarget t;
		t.attr = str(r, "attr");
		const QString &fn = r.function;
		if(!v1) {
			if(fn == QLatin1String("iio_context_get_attr_value")) {
				t.scope = AttrTarget::Context;
			} else if(fn.startsWith(QLatin1String("iio_device_debug_attr_"))) {
				t.scope = AttrTarget::Debug;
				t.device = str(r, "device");
			} else if(fn.startsWith(QLatin1String("iio_device_buffer_attr_"))) {
				t.scope = AttrTarget::Buffer;
				t.device = str(r, "device");
			} else if(fn.startsWith(QLatin1String("iio_device_attr_"))) {
				t.scope = AttrTarget::Device;
				t.device = str(r, "device");
			} else if(fn.startsWith(QLatin1String("iio_channel_attr_"))) {
				t.scope = AttrTarget::Channel;
				t.channel = str(r, "channel");
				t.device = scopeStr(originscope::Device);
				t.output = scopeBool(originscope::Output);
			}
		} else {
			const QVariantMap sc = scope();
			if(sc.value(QLatin1String(originscope::Attribute)).toString() != t.attr) {
				t.why = QStringLiteral("the command's origin is not this attribute");
				return t;
			}
			t.device = sc.value(QLatin1String(originscope::Device)).toString();
			const QString of = sc.value(QLatin1String(originscope::AttributeOf)).toString();
			if(of == QLatin1String("context")) {
				t.scope = AttrTarget::Context;
			} else if(of == QLatin1String("device")) {
				t.scope = sc.value(QLatin1String(originscope::Debug)).toBool() ? AttrTarget::Debug
											       : AttrTarget::Device;
			} else if(of == QLatin1String("channel")) {
				t.scope = AttrTarget::Channel;
				t.channel = sc.value(QLatin1String(originscope::Channel)).toString();
				t.output = sc.value(QLatin1String(originscope::Output)).toBool();
			} else if(of == QLatin1String("buffer")) {
				t.scope = AttrTarget::Buffer;
				t.bufferIndex = sc.value(QLatin1String(originscope::BufferIndex)).toInt();
			}
		}
		if(t.attr.isEmpty()) {
			t.scope = AttrTarget::None;
			t.why = QStringLiteral("attribute name not recorded");
		} else if(t.scope == AttrTarget::None) {
			t.why = QStringLiteral("the attribute's device / channel is not known");
		} else if(t.scope != AttrTarget::Context && t.device.isEmpty()) {
			t.scope = AttrTarget::None;
			t.why = QStringLiteral("the attribute's device is not known");
		} else if(t.scope == AttrTarget::Channel && t.channel.isEmpty()) {
			t.scope = AttrTarget::None;
			t.why = QStringLiteral("the attribute's channel is not known");
		}
		return t;
	}

	// Variable of the object the attribute belongs to (device / channel / v1 buffer / ctx).
	QString parent(const AttrTarget &t, quintptr v0Handle)
	{
		switch(t.scope) {
		case AttrTarget::Context:
			return ctxVar();
		case AttrTarget::Device:
		case AttrTarget::Debug:
			return device(t.device, v0Handle);
		case AttrTarget::Buffer: {
			const QString dev = device(t.device, v0Handle);
			return v1 ? buffer(dev, t.device, t.bufferIndex) : dev;
		}
		case AttrTarget::Channel:
			return channel(device(t.device), t.device, t.channel, t.output, v0Handle);
		case AttrTarget::None:
			break;
		}
		return {};
	}

	// Attribute object variable (C v1, Python); empty for a Python context attribute
	// (a plain string in ctx.attrs).
	QString attrVar(const AttrTarget &t, const QString &parentVar, quintptr v1Handle)
	{
		if(t.scope == AttrTarget::Context && !c) {
			return {};
		}
		const QString key = QStringLiteral("libiio/attr/%1/%2/%3").arg(parentVar).arg(int(t.scope)).arg(t.attr);
		QString v = ctx.names.value(key);
		if(v.isEmpty()) {
			v = ctx.newVar(QStringLiteral("attr"));
			ctx.names.insert(key, v);
			QString expr;
			const QString n = s(t.attr);
			if(c) {
				static const char *fns[] = {"",
							    "iio_context_find_attr",
							    "iio_device_find_attr",
							    "iio_channel_find_attr",
							    "iio_device_find_debug_attr",
							    "iio_buffer_find_attr"};
				expr = QStringLiteral("%1(%2, %3)").arg(QLatin1String(fns[t.scope]), parentVar, n);
			} else {
				static const char *dicts[] = {"", "", "attrs", "attrs", "debug_attrs", "attrs"};
				const char *dict =
					(!v1 && t.scope == AttrTarget::Buffer) ? "buffer_attrs" : dicts[t.scope];
				expr = QStringLiteral("%1.%2[%3]").arg(parentVar, QLatin1String(dict), n);
			}
			ctx.declare(lookup(QStringLiteral("const struct iio_attr *"), v, expr));
		}
		if(v1Handle) {
			ctx.vars.insert(v1Handle, v);
		}
		return v;
	}

	QString attrRead(const CallRecord &r)
	{
		const QString existing = v1 ? ctx.var(r.handle) : QString();
		AttrTarget t = target(r);
		if(existing.isEmpty() && t.scope == AttrTarget::None) {
			return CodeEmitter::unmapped(r, ctx,
						     QStringLiteral("cannot resolve the attribute (%1)").arg(t.why));
		}
		if(t.scope == AttrTarget::None) {
			t.scope = AttrTarget::Device; // label only
		}
		const QString label = t.label();
		QString p, a;
		if(existing.isEmpty()) {
			p = parent(t, v1 ? 0 : r.handle);
			if(c && !v1) {
				a = QString();
			} else {
				a = attrVar(t, p, v1 ? r.handle : 0);
			}
		} else {
			a = existing;
		}

		QStringList out;
		if(c) {
			const bool ctxAttr = !v1 && t.scope == AttrTarget::Context;
			QString fmt = label;
			fmt.replace(QLatin1Char('%'), QStringLiteral("%%"));
			if(ctxAttr) {
				scratch(QStringLiteral("cval"));
				out << st(QStringLiteral("cval = iio_context_get_attr_value(ctx, %1)").arg(s(t.attr)));
				out << st(
					QStringLiteral("printf(%1, cval ? cval : \"(null)\")").arg(s(fmt + " = %s\n")));
			} else {
				scratch(QStringLiteral("ret"));
				scratch(QStringLiteral("val"));
				if(v1) {
					out << st(
						QStringLiteral("ret = iio_attr_read_raw(%1, val, sizeof(val))").arg(a));
				} else {
					out << st(QStringLiteral("ret = %1(%2, %3, val, sizeof(val))")
							  .arg(r.function, p, s(t.attr)));
				}
				if(!r.failed()) {
					out << st(QStringLiteral("printf(%1, val)").arg(s(fmt + " = %s\n")));
				}
			}
		} else {
			const QString expr = a.isEmpty() ? QStringLiteral("ctx.attrs[%1]").arg(s(t.attr))
							 : a + QStringLiteral(".value");
			out << QStringLiteral("val = ") + expr;
			if(!r.failed()) {
				out << QStringLiteral("print(%1, val)").arg(s(label + " ="));
			}
		}
		return done(r, out);
	}

	QString attrWrite(const CallRecord &r)
	{
		const QString existing = v1 ? ctx.var(r.handle) : QString();
		const AttrTarget t = target(r);
		if(existing.isEmpty() && t.scope == AttrTarget::None) {
			return CodeEmitter::unmapped(r, ctx,
						     QStringLiteral("cannot resolve the attribute (%1)").arg(t.why));
		}
		if(existing.isEmpty() && t.scope == AttrTarget::Context) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("context attributes are read-only"));
		}
		const QString value = str(r, "src");
		QString p, a = existing;
		if(a.isEmpty()) {
			p = parent(t, v1 ? 0 : r.handle);
			if(!(c && !v1)) {
				a = attrVar(t, p, v1 ? r.handle : 0);
			}
		}
		QStringList out;
		if(c) {
			scratch(QStringLiteral("ret"));
			if(v1) {
				const qint64 len = Gen::arg(r, "len") ? num(r, "len") : value.toUtf8().size();
				out << st(QStringLiteral("ret = iio_attr_write_raw(%1, %2, %3)")
						  .arg(a, s(value), QString::number(len)));
			} else {
				out << st(
					QStringLiteral("ret = %1(%2, %3, %4)").arg(r.function, p, s(t.attr), s(value)));
			}
		} else {
			out << QStringLiteral("%1.value = %2").arg(a, s(value));
		}
		return done(r, out);
	}

	// --- registers / trigger ---
	QString regRead(const CallRecord &r)
	{
		const QString dev = device(str(r, "device"), r.handle);
		if(dev.isEmpty()) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not recorded"));
		}
		const quint64 addr = static_cast<quint64>(num(r, "address"));
		const QString label = QStringLiteral("%1 reg %2").arg(str(r, "device"), hex(addr));
		QStringList out;
		scratch(QStringLiteral("reg"));
		if(c) {
			scratch(QStringLiteral("ret"));
			out << st(QStringLiteral("ret = iio_device_reg_read(%1, %2, &reg)").arg(dev, hex(addr)));
			if(!r.failed()) {
				QString fmt = label;
				fmt.replace(QLatin1Char('%'), QStringLiteral("%%"));
				out << st(QStringLiteral("printf(%1, (unsigned int)reg)").arg(s(fmt + " = 0x%x\n")));
			}
		} else {
			out << QStringLiteral("reg = %1.reg_read(%2)").arg(dev, hex(addr));
			if(!r.failed()) {
				out << QStringLiteral("print(%1, hex(reg))").arg(s(label + " ="));
			}
		}
		return done(r, out);
	}

	QString regWrite(const CallRecord &r)
	{
		const QString dev = device(str(r, "device"), r.handle);
		if(dev.isEmpty()) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not recorded"));
		}
		const QString addr = hex(static_cast<quint64>(num(r, "address")));
		const QString value = hex(static_cast<quint64>(num(r, "value")));
		QStringList out;
		if(c) {
			scratch(QStringLiteral("ret"));
			out << st(QStringLiteral("ret = iio_device_reg_write(%1, %2, %3)").arg(dev, addr, value));
		} else {
			out << st(QStringLiteral("%1.reg_write(%2, %3)").arg(dev, addr, value));
		}
		return done(r, out);
	}

	QString getTrigger(const CallRecord &r)
	{
		const QString name = str(r, "device");
		const QString dev = device(name, r.handle);
		if(dev.isEmpty()) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not recorded (connection check)"));
		}
		const QString label = name + QStringLiteral(" trigger");
		QStringList out;
		scratch(QStringLiteral("trig"));
		if(c) {
			QString fmt = label;
			fmt.replace(QLatin1Char('%'), QStringLiteral("%%"));
			if(v1) {
				out << st(QStringLiteral("trig = iio_device_get_trigger(%1)").arg(dev));
				out << st(QStringLiteral(
						  "printf(%1, iio_err(trig) ? \"(none)\" : iio_device_get_name(trig))")
						  .arg(s(fmt + " = %s\n")));
			} else {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_device_get_trigger(%1, &trig)").arg(dev));
				out << st(
					QStringLiteral(
						"printf(%1, ret == 0 && trig ? iio_device_get_name(trig) : \"(none)\")")
						.arg(s(fmt + " = %s\n")));
			}
		} else {
			out << QStringLiteral("trig = %1.trigger").arg(dev);
			out << QStringLiteral("print(%1, trig.name if trig else None)").arg(s(label + " ="));
		}
		return done(r, out);
	}

	QString setTrigger(const CallRecord &r)
	{
		const QString dev = device(str(r, "device"), r.handle);
		if(dev.isEmpty()) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not recorded"));
		}
		const QString trigName = str(r, "trigger");
		const QString trig = trigName.isEmpty() ? null() : device(trigName);
		QStringList out;
		if(c) {
			scratch(QStringLiteral("ret"));
			out << st(QStringLiteral("ret = iio_device_set_trigger(%1, %2)").arg(dev, trig));
		} else {
			out << QStringLiteral("%1.trigger = %2").arg(dev, trig);
		}
		return done(r, out);
	}

	// --- channels ---
	QString channelEnable(const CallRecord &r, bool enable)
	{
		const QString devName = scopeStr(originscope::Device);
		const QString ch =
			channel(device(devName), devName, str(r, "channel"), scopeBool(originscope::Output), r.handle);
		if(ch.isEmpty()) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("cannot resolve the channel's device"));
		}
		QStringList out;
		if(!v1) {
			if(c) {
				out << st(QStringLiteral("%1(%2)").arg(r.function, ch));
			} else {
				out << QStringLiteral("%1.enabled = %2").arg(ch, b(enable));
			}
			return done(r, out);
		}
		const CallArg *m = Gen::arg(r, "mask");
		const QString mk = m ? mask(m->value.value<quintptr>()) : QString();
		if(mk.isEmpty()) {
			return CodeEmitter::unmapped(r, ctx, QStringLiteral("channels mask not known"));
		}
		if(c) {
			out << st(QStringLiteral("%1(%2, %3)").arg(r.function, ch, mk));
		} else {
			out << (enable ? QStringLiteral("%1.channels = %1.channels + [%2]").arg(mk, ch)
				       : QStringLiteral("%1.channels = [c for c in %1.channels if c != %2]")
						 .arg(mk, ch));
		}
		return done(r, out);
	}

	// --- data ---
	// The payload of a push / enqueue written into the buffer / block before the call; a payload
	// captured only in part is not written (comment).
	QStringList writePayload(const CallRecord &r, const QString &target, bool block)
	{
		const CallArg *d = Gen::arg(r, "data");
		if(!d) {
			return {};
		}
		const QByteArray data = d->value.toByteArray();
		const qint64 size = num(r, "size", data.size());
		if(data.isEmpty() || data.size() < size) {
			return {comment(QStringLiteral("payload: %1 of %2 bytes logged, not written (turn on \"capture "
						       "data\" to reproduce it)")
						.arg(data.size())
						.arg(size))};
		}
		if(c) {
			const QString arr = ctx.newVar(QStringLiteral("data"));
			ctx.declare(QStringLiteral("static const uint8_t %1[] = %2;")
					    .arg(arr, codegen::bytesLiteral(lang, data)));
			const QString start = block ? QStringLiteral("iio_block_start(%1)").arg(target)
						    : QStringLiteral("iio_buffer_start(%1)").arg(target);
			return {st(QStringLiteral("memcpy(%1, %2, sizeof(%2))").arg(start, arr))};
		}
		return {QStringLiteral("%1.write(bytearray(%2))").arg(target, codegen::bytesLiteral(lang, data))};
	}

	// Python: copy the samples out after a refill / dequeue that returned a payload.
	QStringList readPayload(const CallRecord &r, const QString &source)
	{
		if(r.failed() || r.outKind != CallArg::Bytes || c) {
			return {};
		}
		return {QStringLiteral("data = %1.read()").arg(source)};
	}

	QString unknownHandle(const CallRecord &r, const QString &what)
	{
		return CodeEmitter::unmapped(
			r, ctx,
			QStringLiteral("%1 opened outside the selection (select its stream open too)").arg(what));
	}

	// --- v0 buffers ---
	QString v0Buffer(const CallRecord &r)
	{
		const QString &fn = r.function;
		const QString devName = scopeStr(originscope::Device);
		QStringList out;
		if(fn == QLatin1String("iio_device_set_kernel_buffers_count")) {
			const QString dev = device(devName, r.handle);
			if(dev.isEmpty()) {
				return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not known"));
			}
			const qint64 n = num(r, "nb_buffers");
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_device_set_kernel_buffers_count(%1, %2)")
						  .arg(dev)
						  .arg(n));
			} else {
				out << st(QStringLiteral("%1.set_kernel_buffers_count(%2)").arg(dev).arg(n));
			}
			return done(r, out);
		}
		if(fn == QLatin1String("iio_device_create_buffer")) {
			const QString dev = device(devName, r.handle);
			if(dev.isEmpty()) {
				return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not known"));
			}
			const QString buf = ctx.bind(r.out.value<quintptr>(), QStringLiteral("buf"));
			const qint64 samples = num(r, "samples_count");
			const QString cyclic = b(flag(r, "cyclic"));
			QString expr;
			if(c) {
				expr = QStringLiteral("iio_device_create_buffer(%1, %2, %3)")
					       .arg(dev)
					       .arg(samples)
					       .arg(cyclic);
			} else {
				expr = QStringLiteral("iio.Buffer(%1, %2, %3)").arg(dev).arg(samples).arg(cyclic);
			}
			out << create(QStringLiteral("struct iio_buffer *"), buf, expr);
			if(!r.failed()) {
				own(QStringLiteral("buffer"), buf);
			}
			return done(r, out);
		}
		const QString buf = ctx.var(r.handle);
		if(buf.isEmpty()) {
			return unknownHandle(r, QStringLiteral("buffer"));
		}
		if(fn == QLatin1String("iio_buffer_refill")) {
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_buffer_refill(%1)").arg(buf));
			} else {
				out << st(QStringLiteral("%1.refill()").arg(buf));
			}
			out << readPayload(r, buf);
		} else if(fn == QLatin1String("iio_buffer_push")) {
			out << writePayload(r, buf, false);
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_buffer_push(%1)").arg(buf));
			} else {
				out << st(QStringLiteral("%1.push()").arg(buf));
			}
		} else if(fn == QLatin1String("iio_buffer_cancel")) {
			out << st(c ? QStringLiteral("iio_buffer_cancel(%1)").arg(buf)
				    : QStringLiteral("%1.cancel()").arg(buf));
		} else if(fn == QLatin1String("iio_buffer_destroy")) {
			out << release(buf, QStringLiteral("iio_buffer_destroy"));
		} else {
			return CodeEmitter::unmapped(r, ctx);
		}
		return done(r, out);
	}

	// "iio_x_destroy(v);" / "del v", and v is no longer released by the epilogue.
	QString release(const QString &var, const QString &cFunction)
	{
		release(var);
		if(c) {
			return st(QStringLiteral("%1(%2)").arg(cFunction, var));
		}
		return QStringLiteral("del ") + var;
	}

	// --- v1 buffers / streams / blocks ---
	QString v1Buffer(const CallRecord &r)
	{
		const QString &fn = r.function;
		const QString devName = scopeStr(originscope::Device);
		QStringList out;
		if(fn == QLatin1String("iio_device_get_buffer")) {
			const QString dev = device(devName, r.handle);
			if(dev.isEmpty()) {
				return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not known"));
			}
			// The lookup is the call: declared here (before the statement), bound to the returned buffer.
			const QString buf = buffer(dev, devName, int(num(r, "index")));
			if(!r.failed()) {
				ctx.vars.insert(r.out.value<quintptr>(), buf);
				return {};
			}
			return comment(QStringLiteral("recorded: iio_device_get_buffer failed with rc = %1").arg(r.rc));
		}
		if(fn == QLatin1String("iio_create_channels_mask")) {
			const qint64 n = num(r, "nb_channels");
			const QString dev = device(devName);
			if(py && dev.isEmpty()) {
				return CodeEmitter::unmapped(r, ctx, QStringLiteral("device not known"));
			}
			const QString mk = ctx.bind(r.out.value<quintptr>(), QStringLiteral("mask"));
			QString expr;
			if(c) {
				expr = QStringLiteral("iio_create_channels_mask(%1)").arg(n);
			} else {
				expr = QStringLiteral("iio.ChannelsMask(%1)").arg(dev);
			}
			out << create(QStringLiteral("struct iio_channels_mask *"), mk, expr);
			if(!r.failed()) {
				own(QStringLiteral("mask"), mk);
			}
			return done(r, out);
		}
		if(fn == QLatin1String("iio_device_get_sample_size")) {
			const QString dev = device(devName, r.handle);
			const CallArg *m = Gen::arg(r, "mask");
			const QString mk = m ? mask(m->value.value<quintptr>()) : QString();
			if(dev.isEmpty() || mk.isEmpty()) {
				return CodeEmitter::unmapped(r, ctx,
							     QStringLiteral("device / channels mask not known"));
			}
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_device_get_sample_size(%1, %2)").arg(dev, mk));
			} else {
				out << QStringLiteral("sample_size = %1.sample_size").arg(mk);
			}
			return done(r, out);
		}
		if(fn == QLatin1String("iio_buffer_open")) {
			QString buf = ctx.var(r.handle);
			if(buf.isEmpty()) {
				buf = buffer(device(devName), devName,
					     scope().value(QLatin1String(originscope::BufferIndex)).toInt(), r.handle);
			}
			const CallArg *m = Gen::arg(r, "mask");
			const QString mk = m ? mask(m->value.value<quintptr>()) : QString();
			if(buf.isEmpty() || mk.isEmpty()) {
				return CodeEmitter::unmapped(r, ctx,
							     QStringLiteral("buffer / channels mask not known"));
			}
			const QString stream = ctx.bind(r.out.value<quintptr>(), QStringLiteral("stream"));
			QString expr = c ? QStringLiteral("iio_buffer_open(%1, %2)").arg(buf, mk)
					 : QStringLiteral("%1.open(%2)").arg(buf, mk);
			out << create(QStringLiteral("struct iio_buffer_stream *"), stream, expr);
			if(!r.failed()) {
				own(QStringLiteral("stream"), stream);
			}
			return done(r, out);
		}
		if(fn == QLatin1String("iio_buffer_stream_create_block")) {
			const QString stream = ctx.var(r.handle);
			if(stream.isEmpty()) {
				return unknownHandle(r, QStringLiteral("buffer stream"));
			}
			const QString blk = ctx.bind(r.out.value<quintptr>(), QStringLiteral("blk"));
			const qint64 size = num(r, "size");
			const QString expr = c
				? QStringLiteral("iio_buffer_stream_create_block(%1, %2)").arg(stream).arg(size)
				: QStringLiteral("%1.create_block(%2)").arg(stream).arg(size);
			out << create(QStringLiteral("struct iio_block *"), blk, expr);
			if(!r.failed()) {
				own(QStringLiteral("block"), blk);
			}
			return done(r, out);
		}

		const bool onStream = fn.startsWith(QLatin1String("iio_buffer_"));
		const QString obj = ctx.var(r.handle);
		if(obj.isEmpty()) {
			return unknownHandle(r, onStream ? QStringLiteral("buffer stream") : QStringLiteral("block"));
		}
		if(fn == QLatin1String("iio_block_enqueue")) {
			out << writePayload(r, obj, true);
			const qint64 used = num(r, "bytes_used");
			const bool cyclic = flag(r, "cyclic");
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_block_enqueue(%1, %2, %3)")
						  .arg(obj)
						  .arg(used)
						  .arg(b(cyclic)));
			} else {
				QStringList args;
				if(used) {
					args << QString::number(used);
				}
				if(cyclic) {
					args << QStringLiteral("cyclic=True");
				}
				out << QStringLiteral("%1.enqueue(%2)").arg(obj, args.join(QStringLiteral(", ")));
			}
		} else if(fn == QLatin1String("iio_block_dequeue")) {
			const bool nonblock = flag(r, "nonblock");
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = iio_block_dequeue(%1, %2)").arg(obj, b(nonblock)));
			} else {
				out << st(QStringLiteral("%1.dequeue(%2)").arg(obj, nonblock ? b(true) : QString()));
			}
			out << readPayload(r, obj);
		} else if(fn == QLatin1String("iio_buffer_stream_start") ||
			  fn == QLatin1String("iio_buffer_stream_stop")) {
			const bool start = fn.endsWith(QLatin1String("start"));
			if(c) {
				scratch(QStringLiteral("ret"));
				out << st(QStringLiteral("ret = %1(%2)").arg(fn, obj));
			} else {
				out << st(QStringLiteral("%1.started = %2").arg(obj, b(start)));
			}
		} else if(fn == QLatin1String("iio_buffer_stream_cancel")) {
			out << st(c ? QStringLiteral("iio_buffer_stream_cancel(%1)").arg(obj)
				    : QStringLiteral("%1.cancel()").arg(obj));
		} else if(fn == QLatin1String("iio_block_destroy")) {
			out << release(obj, QStringLiteral("iio_block_destroy"));
		} else if(fn == QLatin1String("iio_buffer_close")) {
			out << release(obj, QStringLiteral("iio_buffer_close"));
		} else if(fn == QLatin1String("iio_channels_mask_destroy")) {
			out << release(obj, QStringLiteral("iio_channels_mask_destroy"));
		} else {
			return CodeEmitter::unmapped(r, ctx);
		}
		return done(r, out);
	}

	// --- context ---
	QString context(const CallRecord &r)
	{
		const QString &fn = r.function;
		if(fn == QLatin1String("iio_create_context_from_uri") || fn == QLatin1String("iio_create_context")) {
			if(!r.failed()) {
				ctx.vars.insert(r.out.value<quintptr>(), ctxVar());
			}
			return comment(QStringLiteral("%1(%2): the context is opened at the top of the script")
					       .arg(fn, r.argsString()));
		}
		if(fn == QLatin1String("iio_context_destroy")) {
			return comment(QStringLiteral(
				"iio_context_destroy: the context is destroyed at the end of the script"));
		}
		if(fn == QLatin1String("iio_context_set_timeout")) {
			const qint64 ms = num(r, "timeout_ms");
			if(c) {
				scratch(QStringLiteral("ret"));
				return done(r, {st(QStringLiteral("ret = iio_context_set_timeout(ctx, %1)").arg(ms))});
			}
			return done(r, {st(QStringLiteral("ctx.set_timeout(%1)").arg(ms))});
		}
		return CodeEmitter::unmapped(r, ctx);
	}

	QString dispatch(const CallRecord &r)
	{
		const QString &fn = r.function;
		if(fn.startsWith(QLatin1String("iio_create_context")) || fn.startsWith(QLatin1String("iio_context_"))) {
			if(fn != QLatin1String("iio_context_get_attr_value")) {
				return context(r);
			}
		}
		if(fn == QLatin1String("iio_device_reg_read")) {
			return regRead(r);
		}
		if(fn == QLatin1String("iio_device_reg_write")) {
			return regWrite(r);
		}
		if(fn == QLatin1String("iio_device_get_trigger")) {
			return getTrigger(r);
		}
		if(fn == QLatin1String("iio_device_set_trigger")) {
			return setTrigger(r);
		}
		if(fn == QLatin1String("iio_channel_enable") || fn == QLatin1String("iio_channel_disable")) {
			return channelEnable(r, fn.endsWith(QLatin1String("enable")));
		}
		if(v1) {
			if(fn == QLatin1String("iio_attr_read_raw")) {
				return attrRead(r);
			}
			if(fn == QLatin1String("iio_attr_write_raw")) {
				return attrWrite(r);
			}
			static const QStringList buffers = {"iio_device_get_buffer",
							    "iio_create_channels_mask",
							    "iio_device_get_sample_size",
							    "iio_buffer_open",
							    "iio_buffer_stream_create_block",
							    "iio_block_enqueue",
							    "iio_block_dequeue",
							    "iio_buffer_stream_start",
							    "iio_buffer_stream_stop",
							    "iio_buffer_stream_cancel",
							    "iio_block_destroy",
							    "iio_buffer_close",
							    "iio_channels_mask_destroy"};
			if(buffers.contains(fn)) {
				return v1Buffer(r);
			}
			return CodeEmitter::unmapped(r, ctx);
		}
		static const QStringList reads = {"iio_context_get_attr_value", "iio_device_attr_read",
						  "iio_channel_attr_read", "iio_device_debug_attr_read",
						  "iio_device_buffer_attr_read"};
		static const QStringList writes = {"iio_device_attr_write", "iio_channel_attr_write",
						   "iio_device_debug_attr_write", "iio_device_buffer_attr_write"};
		if(reads.contains(fn)) {
			return attrRead(r);
		}
		if(writes.contains(fn)) {
			return attrWrite(r);
		}
		static const QStringList buffers = {"iio_device_set_kernel_buffers_count",
						    "iio_device_create_buffer",
						    "iio_buffer_refill",
						    "iio_buffer_push",
						    "iio_buffer_cancel",
						    "iio_buffer_destroy"};
		if(buffers.contains(fn)) {
			return v0Buffer(r);
		}
		return CodeEmitter::unmapped(r, ctx);
	}
};

} // namespace

LibiioEmitter::LibiioEmitter(LibiioVersion version, const QString &language)
	: m_version(version)
	, m_language(language)
{}

LibiioEmitter::~LibiioEmitter() = default;

QList<std::shared_ptr<CodeEmitter>> LibiioEmitter::create(LibiioVersion version)
{
	return {std::make_shared<LibiioEmitter>(version, QString::fromLatin1(codegen::C)),
		std::make_shared<LibiioEmitter>(version, QString::fromLatin1(codegen::Python))};
}

QString LibiioEmitter::library() const { return QString::fromLatin1(LIBRARY); }

QString LibiioEmitter::language() const { return m_language; }

QString LibiioEmitter::description() const
{
	const bool v1 = m_version == LibiioVersion::V1;
	if(m_language == QLatin1String(codegen::Python)) {
		return v1 ? QStringLiteral("libiio v1 API, Python bindings (pylibiio 1.x, module iio)")
			  : QStringLiteral("libiio v0 API, Python bindings (pylibiio 0.2x, module iio)");
	}
	return v1 ? QStringLiteral("libiio v1 API (link with -liio)")
		  : QStringLiteral("libiio v0 API (link with -liio)");
}

QStringList LibiioEmitter::headers(EmitContext &) const
{
	if(m_language == QLatin1String(codegen::Python)) {
		return {QStringLiteral("import iio")};
	}
	return {QStringLiteral("#include <stdbool.h>"), QStringLiteral("#include <stdint.h>"),
		QStringLiteral("#include <stdio.h>"), QStringLiteral("#include <string.h>"),
		m_version == LibiioVersion::V1 ? QStringLiteral("#include <iio/iio.h>")
					       : QStringLiteral("#include <iio.h>")};
}

QString LibiioEmitter::prologue(EmitContext &ctx) const
{
	Gen g(m_version, m_language, ctx);
	QString uri = ctx.uri;
	QStringList out;
	if(uri.isEmpty()) {
		uri = QStringLiteral("local:");
		out << g.comment(QStringLiteral("uri not recorded: set it here"));
	}
	if(g.py) {
		out << QStringLiteral("ctx = iio.Context(%1)").arg(g.s(uri));
	} else if(g.v1) {
		out << QStringLiteral("struct iio_context *ctx = iio_create_context(NULL, %1);").arg(g.s(uri))
		    << QStringLiteral("if (iio_err(ctx)) {")
		    << QStringLiteral("\tfprintf(stderr, \"cannot open the context: %d\\n\", iio_err(ctx));")
		    << QStringLiteral("\treturn 1;") << QStringLiteral("}");
	} else {
		out << QStringLiteral("struct iio_context *ctx = iio_create_context_from_uri(%1);").arg(g.s(uri))
		    << QStringLiteral("if (!ctx) {")
		    << QStringLiteral("\tfprintf(stderr, \"cannot open the context\\n\");")
		    << QStringLiteral("\treturn 1;") << QStringLiteral("}");
	}
	return out.join(QLatin1Char('\n'));
}

QString LibiioEmitter::emitCall(const CallRecord &call, EmitContext &ctx) const
{
	Gen g(m_version, m_language, ctx);
	return g.dispatch(call);
}

QString LibiioEmitter::epilogue(EmitContext &ctx) const
{
	Gen g(m_version, m_language, ctx);
	static const QHash<QString, QString> cRelease = {
		{QStringLiteral("block"), QStringLiteral("iio_block_destroy")},
		{QStringLiteral("stream"), QStringLiteral("iio_buffer_close")},
		{QStringLiteral("mask"), QStringLiteral("iio_channels_mask_destroy")},
		{QStringLiteral("buffer"), QStringLiteral("iio_buffer_destroy")}};
	QStringList out;
	const QStringList resources = ctx.lists.value(QLatin1String(RESOURCES));
	for(auto it = resources.crbegin(); it != resources.crend(); ++it) {
		const QString kind = it->section(QLatin1Char('|'), 0, 0);
		const QString var = it->section(QLatin1Char('|'), 1);
		if(g.c) {
			out << g.st(QStringLiteral("%1(%2)").arg(cRelease.value(kind), var));
		} else {
			out << QStringLiteral("del ") + var;
		}
	}
	if(g.c) {
		out << QStringLiteral("iio_context_destroy(ctx);");
	}
	return out.join(QLatin1Char('\n'));
}
