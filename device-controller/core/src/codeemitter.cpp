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

#include "core/codeemitter.h"

using namespace scopy;

namespace {
bool isC(const QString &l) { return l == QLatin1String(codegen::C); }
bool isPython(const QString &l) { return l == QLatin1String(codegen::Python); }

bool isPrintableAscii(uint c) { return c >= 0x20 && c < 0x7f; }

QString cString(const QString &text)
{
	QString out;
	const QByteArray utf8 = text.toUtf8();
	char prev = 0;
	for(char ch : utf8) {
		const auto u = static_cast<unsigned char>(ch);
		switch(ch) {
		case '\\':
			out += QStringLiteral("\\\\");
			break;
		case '"':
			out += QStringLiteral("\\\"");
			break;
		case '\n':
			out += QStringLiteral("\\n");
			break;
		case '\r':
			out += QStringLiteral("\\r");
			break;
		case '\t':
			out += QStringLiteral("\\t");
			break;
		case '?':
			// "??x" would be a trigraph.
			out += prev == '?' ? QStringLiteral("\\?") : QStringLiteral("?");
			break;
		default:
			if(isPrintableAscii(u)) {
				out += QLatin1Char(ch);
			} else {
				// Octal: always 3 digits, so a following digit is not swallowed (unlike \x).
				out += QStringLiteral("\\%1").arg(u, 3, 8, QLatin1Char('0'));
			}
			break;
		}
		prev = ch;
	}
	return QLatin1Char('"') + out + QLatin1Char('"');
}

QString commonEscape(uint c)
{
	switch(c) {
	case '\\':
		return QStringLiteral("\\\\");
	case '"':
		return QStringLiteral("\\\"");
	case '\n':
		return QStringLiteral("\\n");
	case '\r':
		return QStringLiteral("\\r");
	case '\t':
		return QStringLiteral("\\t");
	default:
		return {};
	}
}

QString pythonString(const QString &text)
{
	QString out;
	for(uint c : text.toUcs4()) {
		const QString esc = commonEscape(c);
		if(!esc.isEmpty()) {
			out += esc;
		} else if(isPrintableAscii(c)) {
			out += QChar(c);
		} else if(c < 0x100) {
			out += QStringLiteral("\\x%1").arg(c, 2, 16, QLatin1Char('0'));
		} else if(c < 0x10000) {
			out += QStringLiteral("\\u%1").arg(c, 4, 16, QLatin1Char('0'));
		} else {
			out += QStringLiteral("\\U%1").arg(c, 8, 16, QLatin1Char('0'));
		}
	}
	return QLatin1Char('"') + out + QLatin1Char('"');
}
} // namespace

QString codegen::languageName(const QString &language)
{
	if(isC(language)) {
		return QStringLiteral("C");
	}
	if(isPython(language)) {
		return QStringLiteral("Python");
	}
	return language;
}

QString codegen::fileExtension(const QString &language)
{
	if(isC(language)) {
		return QStringLiteral("c");
	}
	if(isPython(language)) {
		return QStringLiteral("py");
	}
	return QStringLiteral("txt");
}

QString codegen::indentUnit(const QString &language)
{
	return isC(language) ? QStringLiteral("\t") : QStringLiteral("    ");
}

QString codegen::indent(const QString &text, const QString &prefix)
{
	if(text.isEmpty()) {
		return text;
	}
	QStringList lines = text.split(QLatin1Char('\n'));
	for(QString &l : lines) {
		if(!l.isEmpty()) {
			l.prepend(prefix);
		}
	}
	return lines.join(QLatin1Char('\n'));
}

QString codegen::comment(const QString &language, const QString &text)
{
	const QString lead = isPython(language) ? QStringLiteral("#") : QStringLiteral("//");
	QStringList out;
	for(QString line : text.split(QLatin1Char('\n'))) {
		line.remove(QLatin1Char('\r'));
		if(line.endsWith(QLatin1Char('\\'))) {
			line += QLatin1Char('.'); // a trailing backslash would continue a C line comment
		}
		out << (line.isEmpty() ? lead : lead + QLatin1Char(' ') + line);
	}
	return out.join(QLatin1Char('\n'));
}

QString codegen::stringLiteral(const QString &language, const QString &text)
{
	if(isPython(language)) {
		return pythonString(text);
	}
	return cString(text);
}

QString codegen::bytesLiteral(const QString &language, const QByteArray &bytes)
{
	static constexpr int PER_LINE = 16;
	QStringList items;
	items.reserve(bytes.size());
	for(char b : bytes) {
		items << QStringLiteral("0x%1").arg(static_cast<unsigned char>(b), 2, 16, QLatin1Char('0'));
	}
	QString open, close;
	if(isPython(language)) {
		open = QStringLiteral("bytes([");
		close = QStringLiteral("])");
	} else {
		open = QStringLiteral("{");
		close = QStringLiteral("}");
	}
	const bool braces = !isPython(language);
	if(items.size() <= PER_LINE) {
		if(items.isEmpty()) {
			return open + close;
		}
		const QString pad = braces ? QStringLiteral(" ") : QString();
		return open + pad + items.join(QStringLiteral(", ")) + pad + close;
	}
	QStringList lines;
	for(int i = 0; i < items.size(); i += PER_LINE) {
		lines << indentUnit(language) + items.mid(i, PER_LINE).join(QStringLiteral(", ")) + QLatin1Char(',');
	}
	lines.last().chop(1);
	return open + QLatin1Char('\n') + lines.join(QLatin1Char('\n')) + QLatin1Char('\n') + close;
}

QString codegen::boolLiteral(const QString &language, bool value)
{
	if(isPython(language)) {
		return value ? QStringLiteral("True") : QStringLiteral("False");
	}
	return value ? QStringLiteral("true") : QStringLiteral("false");
}

QString codegen::hexLiteral(quint64 value) { return QStringLiteral("0x%1").arg(value, 0, 16); }

QString codegen::literal(const QString &language, const QVariant &value, CallArg::Kind kind)
{
	switch(kind) {
	case CallArg::Hex:
	case CallArg::Handle:
		return hexLiteral(value.toULongLong());
	case CallArg::Bool:
		return boolLiteral(language, value.toBool());
	case CallArg::String:
		return stringLiteral(language, value.toString());
	case CallArg::Bytes:
		return bytesLiteral(language, value.toByteArray());
	case CallArg::Int:
	case CallArg::OutLen:
		return QString::number(value.toLongLong());
	}
	return value.toString();
}

QString codegen::loopBegin(const QString &language, int count)
{
	if(isPython(language)) {
		return QStringLiteral("for _ in range(%1):").arg(count);
	}
	return QStringLiteral("for (int i = 0; i < %1; i++) {").arg(count);
}

QString codegen::loopEnd(const QString &language) { return isPython(language) ? QString() : QStringLiteral("}"); }

QString codegen::script(const QString &language, const QString &headerComment, const QStringList &headers,
			const QString &body)
{
	QStringList out;
	if(isPython(language)) {
		out << QStringLiteral("#!/usr/bin/env python3");
	}
	if(!headerComment.isEmpty()) {
		out << headerComment;
	}
	if(!headers.isEmpty()) {
		out << QString() << headers;
	}
	out << QString();

	if(isC(language)) {
		out << QStringLiteral("int main(void)") << QStringLiteral("{");
		if(!body.isEmpty()) {
			out << indent(body, QStringLiteral("\t")) << QString();
		}
		out << QStringLiteral("\treturn 0;") << QStringLiteral("}");
	} else {
		if(!body.isEmpty()) {
			out << body;
		}
	}
	return out.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

// --- EmitContext ---

QString EmitContext::newVar(const QString &prefix)
{
	int &n = m_counters[prefix];
	return prefix + QString::number(n++);
}

QString EmitContext::bind(quintptr handle, const QString &prefix)
{
	const QString v = newVar(prefix);
	if(handle) {
		vars.insert(handle, v);
	}
	return v;
}

QString EmitContext::var(quintptr handle) const { return handle ? vars.value(handle) : QString(); }

void EmitContext::declare(const QString &code)
{
	if(!code.isEmpty()) {
		m_pending << code;
	}
}

bool EmitContext::declareOnce(const QString &key, const QString &code)
{
	if(m_once.contains(key)) {
		return false;
	}
	m_once.insert(key);
	declare(code);
	return true;
}

QStringList EmitContext::takeDeclarations()
{
	QStringList out;
	out.swap(m_pending);
	return out;
}

// --- CodeEmitter ---

CodeEmitter::~CodeEmitter() = default;

QString CodeEmitter::unmapped(const CallRecord &call, EmitContext &ctx, const QString &why)
{
	const QString text = QStringLiteral("%1(%2) = %3: %4")
				     .arg(call.function, call.argsString(), QString::number(call.rc),
					  why.isEmpty() ? QStringLiteral("no binding") : why);
	return ctx.comment(text);
}
