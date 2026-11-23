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

#include "libm2kemitter.h"

#include "m2ktrace.h"

#include <QHash>

using namespace scopy;
using namespace scopy::adalm2000;

namespace {

// The libm2k objects a script uses: one variable each, obtained from the context (the trigger
// from the analog input).
struct ObjectInfo
{
	const char *var;
	const char *getter; // on `from`
	const char *from;   // class of the object it is obtained from
	const char *cppType;
};

const QHash<QString, ObjectInfo> &objects()
{
	static const QHash<QString, ObjectInfo> map = {
		{QStringLiteral("M2kAnalogIn"), {"ain", "getAnalogIn", "M2k", "libm2k::analog::M2kAnalogIn"}},
		{QStringLiteral("M2kAnalogOut"), {"aout", "getAnalogOut", "M2k", "libm2k::analog::M2kAnalogOut"}},
		{QStringLiteral("M2kPowerSupply"), {"ps", "getPowerSupply", "M2k", "libm2k::analog::M2kPowerSupply"}},
		{QStringLiteral("M2kDigital"), {"dig", "getDigital", "M2k", "libm2k::digital::M2kDigital"}},
		{QStringLiteral("M2kHardwareTrigger"),
		 {"trig", "getTrigger", "M2kAnalogIn", "libm2k::M2kHardwareTrigger"}},
	};
	return map;
}

// Class of the object a getter returns ("M2k::getAnalogIn" -> "M2kAnalogIn"); empty for others.
QString objectReturnedBy(const QString &method)
{
	for(auto it = objects().cbegin(); it != objects().cend(); ++it) {
		if(method == QLatin1String(it->getter)) {
			return it.key();
		}
	}
	return {};
}

class Gen
{
public:
	Gen(const QString &language, EmitContext &ctx)
		: ctx(ctx)
		, py(language == QLatin1String(codegen::Python))
	{}

	EmitContext &ctx;
	const bool py;

	QString st(const QString &code) const { return py ? code : code + QLatin1Char(';'); }
	QString member(const QString &object, const QString &method, const QString &args) const
	{
		return object + (py ? QStringLiteral(".") : QStringLiteral("->")) + method + QLatin1Char('(') + args +
			QLatin1Char(')');
	}

	// Variable of a libm2k object of class `cls`, declared on first use.
	QString object(const QString &cls)
	{
		if(cls == QLatin1String("M2k")) {
			return QStringLiteral("ctx");
		}
		const auto it = objects().constFind(cls);
		if(it == objects().cend()) {
			return {};
		}
		const QString from = object(QString::fromLatin1(it->from));
		const QString var = QString::fromLatin1(it->var);
		const QString get = member(from, QString::fromLatin1(it->getter), {});
		ctx.declareOnce(QStringLiteral("libm2k:") + var,
				py ? var + QStringLiteral(" = ") + get
				   : QStringLiteral("%1 *%2 = %3;").arg(QString::fromLatin1(it->cppType), var, get));
		return var;
	}

	QString number(const QVariant &v) const
	{
		if(v.typeId() == QMetaType::Double || v.typeId() == QMetaType::Float) {
			return QString::number(v.toDouble(), 'g', 15);
		}
		return QString::number(v.toLongLong());
	}

	// A data buffer of `size` elements for a call whose data was not logged.
	QString buffer(const QString &cls, const QString &method, int argCount, qint64 size)
	{
		const QString var = ctx.newVar(QStringLiteral("data"));
		const QString note = QStringLiteral("samples were not logged: fill in the data");
		QString code;
		if(cls == QLatin1String("M2kDigital")) {
			code = py ? QStringLiteral("%1 = [0] * %2").arg(var).arg(size)
				  : QStringLiteral("std::vector<unsigned short> %1(%2);").arg(var).arg(size);
		} else if(cls == QLatin1String("M2kAnalogOut") && method == QLatin1String("push") && argCount == 1) {
			// One buffer per channel: the size logged is the channel count.
			code = py
				? QStringLiteral("%1 = [[0.0] * 1024 for _ in range(%2)]").arg(var).arg(size)
				: QStringLiteral("std::vector<std::vector<double>> %1(%2, std::vector<double>(1024));")
					  .arg(var)
					  .arg(size);
		} else {
			code = py ? QStringLiteral("%1 = [0.0] * %2").arg(var).arg(size)
				  : QStringLiteral("std::vector<double> %1(%2);").arg(var).arg(size);
		}
		ctx.declare(ctx.comment(note) + QLatin1Char('\n') + code);
		return var;
	}

	QString arg(const CallArg &a, const QString &cls, const QString &method, int argCount)
	{
		if(a.name.startsWith(QLatin1String(m2ktrace::ENUM_PREFIX))) {
			const QString type = a.name.mid(int(qstrlen(m2ktrace::ENUM_PREFIX)));
			const QString symbol = a.value.toString();
			if(py) {
				return QStringLiteral("libm2k.") + symbol;
			}
			return type.section(QStringLiteral("::"), 0, -2) + QStringLiteral("::") + symbol;
		}
		switch(a.kind) {
		case CallArg::Bool:
			return codegen::boolLiteral(ctx.language, a.value.toBool());
		case CallArg::String:
			return ctx.str(a.value.toString());
		case CallArg::OutLen:
			return buffer(cls, method, argCount, a.value.toLongLong());
		case CallArg::Hex:
		case CallArg::Handle:
			return codegen::hexLiteral(a.value.toULongLong());
		case CallArg::Bytes:
			return codegen::bytesLiteral(ctx.language, a.value.toByteArray());
		case CallArg::Int:
			return number(a.value);
		}
		return a.value.toString();
	}
};

} // namespace

Libm2kEmitter::Libm2kEmitter(const QString &language)
	: m_language(language)
{}

Libm2kEmitter::~Libm2kEmitter() = default;

QList<std::shared_ptr<CodeEmitter>> Libm2kEmitter::create()
{
	return {std::make_shared<Libm2kEmitter>(QString::fromLatin1(codegen::Cpp)),
		std::make_shared<Libm2kEmitter>(QString::fromLatin1(codegen::Python))};
}

QString Libm2kEmitter::library() const { return QString::fromLatin1(LIBRARY); }

QString Libm2kEmitter::language() const { return m_language; }

QString Libm2kEmitter::description() const
{
	return m_language == QLatin1String(codegen::Python) ? QStringLiteral("libm2k Python bindings (module libm2k)")
							    : QStringLiteral("libm2k C++ API");
}

QStringList Libm2kEmitter::headers(EmitContext &ctx) const
{
	Q_UNUSED(ctx)
	if(m_language == QLatin1String(codegen::Python)) {
		return {QStringLiteral("import libm2k")};
	}
	return {QStringLiteral("#include <libm2k/analog/m2kanalogin.hpp>"),
		QStringLiteral("#include <libm2k/analog/m2kanalogout.hpp>"),
		QStringLiteral("#include <libm2k/analog/m2kpowersupply.hpp>"),
		QStringLiteral("#include <libm2k/contextbuilder.hpp>"),
		QStringLiteral("#include <libm2k/digital/m2kdigital.hpp>"),
		QStringLiteral("#include <libm2k/m2k.hpp>"),
		QStringLiteral("#include <libm2k/m2khardwaretrigger.hpp>"),
		QStringLiteral("#include <iostream>"),
		QStringLiteral("#include <vector>")};
}

QString Libm2kEmitter::prologue(EmitContext &ctx) const
{
	Gen g(m_language, ctx);
	QString uri = ctx.uri;
	QStringList out;
	if(uri.isEmpty()) {
		uri = QStringLiteral("ip:192.168.2.1");
		out << ctx.comment(QStringLiteral("uri not recorded: set it here"));
	}
	if(g.py) {
		out << QStringLiteral("ctx = libm2k.m2kOpen(%1)").arg(ctx.str(uri)) << QStringLiteral("if ctx is None:")
		    << QStringLiteral("    raise SystemExit(\"cannot open the M2K context\")");
	} else {
		out << QStringLiteral("libm2k::context::M2k *ctx = libm2k::context::m2kOpen(%1);").arg(ctx.str(uri))
		    << QStringLiteral("if (!ctx) {")
		    << QStringLiteral("\tstd::cerr << \"cannot open the M2K context\" << std::endl;")
		    << QStringLiteral("\treturn 1;") << QStringLiteral("}");
	}
	return out.join(QLatin1Char('\n'));
}

QString Libm2kEmitter::emitCall(const CallRecord &call, EmitContext &ctx) const
{
	Gen g(m_language, ctx);
	const QString cls = call.function.section(QStringLiteral("::"), 0, 0);
	const QString method = call.function.section(QStringLiteral("::"), 1);
	const QString object = g.object(cls);
	if(object.isEmpty() || method.isEmpty()) {
		return unmapped(call, ctx, QStringLiteral("unknown libm2k class"));
	}

	// Getting an object (ain = ctx.getAnalogIn()) is its declaration.
	const QString returned = objectReturnedBy(method);
	if(!returned.isEmpty() && call.outKind == CallArg::Handle) {
		const QString var = g.object(returned);
		ctx.vars.insert(call.out.value<quintptr>(), var);
		return {};
	}

	QStringList args;
	for(const CallArg &a : call.args) {
		args << g.arg(a, cls, method, int(call.args.size()));
	}
	const QString expr = g.member(object, method, args.join(QStringLiteral(", ")));

	QStringList out;
	if(call.failed()) {
		out << ctx.comment(QStringLiteral("failed in the log (%1): %2").arg(call.rc).arg(call.out.toString()));
	}
	const bool value = call.out.isValid() && !call.failed();
	if(!value) {
		out << g.st(expr);
		return out.join(QLatin1Char('\n'));
	}
	const QString label = ctx.str(call.function + QStringLiteral(" ="));
	switch(call.outKind) {
	case CallArg::Handle:
	case CallArg::OutLen: {
		// Sample buffers (pointers) and lists (vectors): kept in a variable, not printed.
		const QString var = ctx.newVar(call.outKind == CallArg::Handle ? QStringLiteral("samples")
									       : QStringLiteral("values"));
		out << (g.py ? var + QStringLiteral(" = ") + expr : QStringLiteral("auto %1 = %2;").arg(var, expr));
		break;
	}
	case CallArg::String:
		if(g.py) {
			const QString var = ctx.newVar(QStringLiteral("val"));
			out << var + QStringLiteral(" = ") + expr << QStringLiteral("print(%1, %2)").arg(label, var);
		} else {
			// An enum, a string or a pair: the value read is shown as a comment.
			const QString var = ctx.newVar(QStringLiteral("val"));
			out << QStringLiteral("auto %1 = %2; // %3").arg(var, expr, call.out.toString());
		}
		break;
	default: {
		const QString var = ctx.newVar(QStringLiteral("val"));
		if(g.py) {
			out << var + QStringLiteral(" = ") + expr << QStringLiteral("print(%1, %2)").arg(label, var);
		} else {
			out << QStringLiteral("auto %1 = %2;").arg(var, expr)
			    << QStringLiteral("std::cout << %1 << \" \" << %2 << std::endl;").arg(label, var);
		}
		break;
	}
	}
	return out.join(QLatin1Char('\n'));
}

QString Libm2kEmitter::epilogue(EmitContext &ctx) const
{
	Gen g(m_language, ctx);
	return g.py ? QStringLiteral("libm2k.contextClose(ctx)")
		    : QStringLiteral("libm2k::context::contextClose(ctx);");
}
