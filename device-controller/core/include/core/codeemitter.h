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

#pragma once

#include "core/callrecord.h"
#include "core/controller_core_export.h"

#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace scopy {

// Syntax of the target languages shared by every code emitter: literals, comments, loops and the
// file frame. Pure text; language ids are the ones CodeEmitter::language() returns.
namespace codegen {
inline constexpr auto C = "c";
inline constexpr auto Python = "python";
inline constexpr auto CSharp = "csharp";

// "C" / "Python" / "C#" (the id itself for an unknown language).
CONTROLLER_CORE_EXPORT QString languageName(const QString &language);
// "c" / "py" / "cs" ("txt" for an unknown language).
CONTROLLER_CORE_EXPORT QString fileExtension(const QString &language);
// One indentation level: a tab for C, four spaces otherwise.
CONTROLLER_CORE_EXPORT QString indentUnit(const QString &language);
// Prefixes every non-empty line of `text` with `prefix`.
CONTROLLER_CORE_EXPORT QString indent(const QString &text, const QString &prefix);

// Line comment ("# " for Python, "// " otherwise), one per line of `text`.
CONTROLLER_CORE_EXPORT QString comment(const QString &language, const QString &text);
// Escaped, double-quoted string literal of the language (UTF-8 bytes as octal escapes in C,
// \uXXXX for non-ASCII in Python / C#).
CONTROLLER_CORE_EXPORT QString stringLiteral(const QString &language, const QString &text);
// Python `bytes([0x01, 0x02])`, C# `new byte[] { 0x01, 0x02 }`, C array initializer `{ 0x01, 0x02 }`.
CONTROLLER_CORE_EXPORT QString bytesLiteral(const QString &language, const QByteArray &bytes);
CONTROLLER_CORE_EXPORT QString boolLiteral(const QString &language, bool value);
CONTROLLER_CORE_EXPORT QString hexLiteral(quint64 value);
// A value typed by its kind: Hex 0x50, Bool, String / Bytes literals, Int / OutLen / Handle as numbers.
CONTROLLER_CORE_EXPORT QString literal(const QString &language, const QVariant &value, CallArg::Kind kind);
// `for (int i = 0; i < N; i++) {` / `for _ in range(N):` and the closing line ("" for Python).
CONTROLLER_CORE_EXPORT QString loopBegin(const QString &language, int count);
CONTROLLER_CORE_EXPORT QString loopEnd(const QString &language);

// The whole file: header comment, header lines (#include / import / using), then the body inside the
// language's entry point (C main, C# Program.Main, Python top level).
CONTROLLER_CORE_EXPORT QString script(const QString &language, const QString &headerComment, const QStringList &headers,
				      const QString &body);
} // namespace codegen

// Options of one generated script.
struct CONTROLLER_CORE_EXPORT EmitOptions
{
	bool loops = true; // consecutive repeats of the same call sequence become a loop
};

// State of one script being generated, shared by the emitters of every library in it.
struct CONTROLLER_CORE_EXPORT EmitContext
{
	QString language;
	QString uri; // connection of the calls: opened in the prologue
	bool loops = true;

	QHash<quintptr, QString> vars;	   // handle → variable: the same object is always the same variable
	QHash<QString, QString> names;	   // emitter keys → variable, for objects found by name
	QHash<QString, QStringList> lists; // emitter bookkeeping (e.g. objects to release in the epilogue)

	// The command whose call is being emitted (its origin resolves names a record lacks); null
	// in prologue() / epilogue().
	const CommandLogEntry *entry = nullptr;
	// True while the calls of a loop body are emitted: a variable created there must be declared
	// before the loop (see declare()).
	bool inLoop = false;

	// "dev0", "dev1", ...: unique in the script.
	QString newVar(const QString &prefix);
	// A new variable for `handle` (a returned / first seen object); replaces an older binding.
	QString bind(quintptr handle, const QString &prefix);
	// Variable of `handle`; empty if unknown (or handle 0).
	QString var(quintptr handle) const;

	// Code the generator places before the current statement, at the top scope (before the loop
	// when inLoop): object lookups, scratch variables. declareOnce() skips a key already declared.
	void declare(const QString &code);
	bool declareOnce(const QString &key, const QString &code);
	QStringList takeDeclarations();

	QString comment(const QString &text) const { return codegen::comment(language, text); }
	QString str(const QString &text) const { return codegen::stringLiteral(language, text); }

private:
	QHash<QString, int> m_counters;
	QSet<QString> m_once;
	QStringList m_pending;
};

// Turns the recorded calls of one library into code of one language (pure text: no link to the
// library itself). Registered in the component's CodeEmitterRegistry; one per (library, language).
class CONTROLLER_CORE_EXPORT CodeEmitter
{
public:
	virtual ~CodeEmitter();

	virtual QString library() const = 0;  // CallRecord::library it handles: "libiio", "remote-bus"
	virtual QString language() const = 0; // codegen::C / Python / CSharp
	// Shown in the script header, e.g. "libiio v1 API (pylibiio 1.x)".
	virtual QString description() const { return library(); }

	// #include / import / using lines (called after the body: may depend on what was used).
	virtual QStringList headers(EmitContext &ctx) const = 0;
	// Statements before the calls: open the context / device.
	virtual QString prologue(EmitContext &ctx) const = 0;
	// One recorded call (ctx.entry is its command). An unmapped function returns a comment
	// ("<call>: no binding") so the script is always produced.
	virtual QString emitCall(const CallRecord &call, EmitContext &ctx) const = 0;
	// Statements after the calls: release what is still open, close the context.
	virtual QString epilogue(EmitContext &ctx) const = 0;

	// "<function>(<args>) = <rc>: no binding" as a comment.
	static QString unmapped(const CallRecord &call, EmitContext &ctx, const QString &why = {});
};

} // namespace scopy
