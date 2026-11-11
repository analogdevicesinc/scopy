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

#include "component/codeemitterregistry.h"

#include "component/backends/iio/iioprovider.h"
#include "iioutil/ibackend.h"
#include "iioutil/libiioemitter.h"

#include <QDateTime>
#include <QHash>
#include <QTimeZone>

#include <algorithm>

using namespace scopy;
using namespace scopy::component;

namespace {
const QStringList &knownLanguages()
{
	static const QStringList l = {QString::fromLatin1(codegen::C), QString::fromLatin1(codegen::Python),
				      QString::fromLatin1(codegen::CSharp)};
	return l;
}

QString utcTime(qint64 ns)
{
	return QDateTime::fromMSecsSinceEpoch(ns / 1000000, QTimeZone::UTC)
		.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
}

QString shortClass(const QString &cls)
{
	const qsizetype sep = cls.lastIndexOf(QLatin1String("::"));
	return sep >= 0 ? cls.mid(sep + 2) : cls;
}

// Python needs a statement in a block that would otherwise hold only comments.
QString pythonBlock(const QString &body)
{
	for(const QString &line : body.split(QLatin1Char('\n'))) {
		const QString t = line.trimmed();
		if(!t.isEmpty() && !t.startsWith(QLatin1Char('#'))) {
			return body;
		}
	}
	return body.isEmpty() ? QStringLiteral("pass") : body + QStringLiteral("\npass");
}
} // namespace

CodeEmitterRegistry::CodeEmitterRegistry() = default;

CodeEmitterRegistry::~CodeEmitterRegistry() = default;

CodeEmitterRegistry *CodeEmitterRegistry::GetInstance()
{
	// The libiio emitters match the backend linked into `component`: it records the calls.
	static CodeEmitterRegistry *s_instance = [] {
		auto *r = new CodeEmitterRegistry();
		for(const auto &e : scopy::iio::LibiioEmitter::create(iio::IIOProvider::defaultBackend()->version())) {
			r->add(e);
		}
		return r;
	}();
	return s_instance;
}

void CodeEmitterRegistry::add(std::shared_ptr<CodeEmitter> emitter)
{
	if(!emitter) {
		return;
	}
	std::lock_guard<std::mutex> lock(m_mutex);
	for(auto &e : m_emitters) {
		if(e->library() == emitter->library() && e->language() == emitter->language()) {
			e = std::move(emitter);
			return;
		}
	}
	m_emitters.append(std::move(emitter));
}

bool CodeEmitterRegistry::remove(const QString &library, const QString &language)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	for(qsizetype i = 0; i < m_emitters.size(); ++i) {
		if(m_emitters[i]->library() == library && m_emitters[i]->language() == language) {
			m_emitters.removeAt(i);
			return true;
		}
	}
	return false;
}

std::shared_ptr<CodeEmitter> CodeEmitterRegistry::emitter(const QString &library, const QString &language) const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	for(const auto &e : m_emitters) {
		if(e->library() == library && e->language() == language) {
			return e;
		}
	}
	return {};
}

QStringList CodeEmitterRegistry::libraries() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	QStringList out;
	for(const auto &e : m_emitters) {
		if(!out.contains(e->library())) {
			out << e->library();
		}
	}
	return out;
}

QStringList CodeEmitterRegistry::languagesFor(const QStringList &libraries) const
{
	if(libraries.isEmpty()) {
		return {};
	}
	QStringList candidates = knownLanguages();
	QStringList others;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for(const auto &e : m_emitters) {
			if(!candidates.contains(e->language()) && !others.contains(e->language())) {
				others << e->language();
			}
		}
	}
	others.sort();
	candidates << others;

	QStringList out;
	for(const QString &lang : std::as_const(candidates)) {
		const bool all = std::all_of(libraries.cbegin(), libraries.cend(),
					     [&](const QString &lib) { return emitter(lib, lang) != nullptr; });
		if(all) {
			out << lang;
		}
	}
	return out;
}

QStringList CodeEmitterRegistry::librariesOf(const QList<CommandLogEntry> &entries)
{
	QStringList out;
	for(const CommandLogEntry &e : entries) {
		for(const CallRecord &c : e.calls) {
			if(!c.library.isEmpty() && !out.contains(c.library)) {
				out << c.library;
			}
		}
	}
	return out;
}

QString CodeEmitterRegistry::exportScript(const QList<CommandLogEntry> &entries, const QString &language,
					  const EmitOptions &options) const
{
	QList<CommandLogEntry> sorted = entries;
	std::stable_sort(sorted.begin(), sorted.end(),
			 [](const CommandLogEntry &a, const CommandLogEntry &b) { return a.startNs < b.startNs; });

	struct Item
	{
		int entry;
		int call;
	};
	QList<Item> items;
	for(int e = 0; e < sorted.size(); ++e) {
		for(int c = 0; c < sorted[e].calls.size(); ++c) {
			items.append({e, c});
		}
	}
	if(items.isEmpty()) {
		return {};
	}
	auto callAt = [&](int i) -> const CallRecord & { return sorted[items[i].entry].calls[items[i].call]; };

	const QStringList libs = librariesOf(sorted);
	QHash<QString, std::shared_ptr<CodeEmitter>> emitters;
	for(const QString &lib : libs) {
		emitters.insert(lib, emitter(lib, language));
	}

	EmitContext ctx;
	ctx.language = language;
	ctx.loops = options.loops;
	for(const CommandLogEntry &e : std::as_const(sorted)) {
		if(!e.uri.isEmpty()) {
			ctx.uri = e.uri;
			break;
		}
	}

	auto emitOne = [&](const CallRecord &rec, const CommandLogEntry *entry) {
		ctx.entry = entry;
		const auto em = emitters.value(rec.library);
		const QString text = em
			? em->emitCall(rec, ctx)
			: CodeEmitter::unmapped(rec, ctx,
						QStringLiteral("no %1 emitter for %2")
							.arg(codegen::languageName(language), rec.library));
		ctx.entry = nullptr;
		return text;
	};

	// Prologues.
	QStringList prologue;
	for(const QString &lib : libs) {
		if(const auto em = emitters.value(lib)) {
			const QString p = em->prologue(ctx);
			prologue << ctx.takeDeclarations();
			if(!p.isEmpty()) {
				prologue << p;
			}
		}
	}

	// Calls, in time order; repeats of a call sequence fold into a loop.
	const int n = items.size();
	auto samePeriod = [&](int a, int b, int p) {
		for(int q = 0; q < p; ++q) {
			if(!callAt(a + q).sameCall(callAt(b + q))) {
				return false;
			}
		}
		return true;
	};
	const QString unit = codegen::indentUnit(language);
	const bool python = language == QLatin1String(codegen::Python);
	QStringList body;
	int lastEntry = -1;
	for(int i = 0; i < n;) {
		int bestP = 0, bestK = 1;
		// A loop covers whole commands (it starts at a command's first call and each repeat ends at
		// a command boundary: a refill repeats as "enqueue; dequeue", not as a shifted pair), or
		// stays inside one command (a command repeating one call).
		if(options.loops) {
			const int entry = items[i].entry;
			auto boundary = [&](int j) { return j == n || items[j].call == 0; };
			for(int p = 1; p <= MAX_LOOP_PERIOD && i + 2 * p <= n; ++p) {
				const bool aligned = items[i].call == 0 && boundary(i + p);
				if(!aligned && items[i + 2 * p - 1].entry != entry) {
					continue;
				}
				auto fits = [&](int end) {
					return aligned ? boundary(end) : items[end - 1].entry == entry;
				};
				int k = 1;
				while(i + (k + 1) * p <= n && fits(i + (k + 1) * p) && samePeriod(i, i + k * p, p)) {
					++k;
				}
				if(k >= 2 && p * k > bestP * bestK) {
					bestP = p;
					bestK = k;
				}
			}
		}
		if(bestK >= 2) {
			QStringList origins;
			QStringList loopBody;
			ctx.inLoop = true;
			for(int q = 0; q < bestP; ++q) {
				const CommandLogEntry &entry = sorted[items[i + q].entry];
				if(!origins.contains(entry.origin)) {
					origins << entry.origin;
				}
				const QString text = emitOne(callAt(i + q), &entry);
				if(!text.isEmpty()) {
					loopBody << text;
				}
			}
			ctx.inLoop = false;
			body << ctx.comment(
				QStringLiteral("%1 times: %2").arg(QString::number(bestK), origins.join(", ")));
			body << ctx.takeDeclarations();
			QString inner = loopBody.join(QLatin1Char('\n'));
			if(python) {
				inner = pythonBlock(inner);
			}
			body << codegen::loopBegin(language, bestK) << codegen::indent(inner, unit);
			const QString end = codegen::loopEnd(language);
			if(!end.isEmpty()) {
				body << end;
			}
			i += bestP * bestK;
			lastEntry = items[i - 1].entry;
			continue;
		}
		const Item &it = items[i];
		const CommandLogEntry &entry = sorted[it.entry];
		if(it.entry != lastEntry) {
			lastEntry = it.entry;
			QString what = entry.origin.isEmpty() ? QStringLiteral("(no origin)") : entry.origin;
			if(!entry.commandClass.isEmpty()) {
				what += QStringLiteral(" (%1)").arg(shortClass(entry.commandClass));
			}
			body << ctx.comment(what);
		}
		const QString text = emitOne(callAt(i), &entry);
		body << ctx.takeDeclarations();
		if(!text.isEmpty()) {
			body << text;
		}
		++i;
	}

	// Epilogues, last library first.
	QStringList epilogue;
	for(auto lib = libs.crbegin(); lib != libs.crend(); ++lib) {
		if(const auto em = emitters.value(*lib)) {
			const QString e = em->epilogue(ctx);
			epilogue << ctx.takeDeclarations();
			if(!e.isEmpty()) {
				epilogue << e;
			}
		}
	}

	QStringList headers;
	QStringList described;
	for(const QString &lib : libs) {
		const auto em = emitters.value(lib);
		if(!em) {
			described << QStringLiteral("%1: no %2 emitter, its calls are comments")
					     .arg(lib, codegen::languageName(language));
			continue;
		}
		described << em->description();
		for(const QString &h : em->headers(ctx)) {
			if(!headers.contains(h)) {
				headers << h;
			}
		}
	}

	QStringList note;
	note << QStringLiteral("Generated by Scopy (DC Debugger) from %1 logged command(s), %2 - %3 UTC.")
			.arg(QString::number(sorted.size()), utcTime(sorted.first().startNs),
			     utcTime(sorted.last().startNs));
	if(!ctx.uri.isEmpty()) {
		note << QStringLiteral("Connection: %1").arg(ctx.uri);
	}
	note << described;
	note << QStringLiteral("It repeats the logged library calls only: state configured before logging started")
	     << QStringLiteral("(or by commands outside the selection) is not reproduced.");

	QStringList all = prologue;
	if(!body.isEmpty()) {
		all << QString() << body;
	}
	if(!epilogue.isEmpty()) {
		all << QString() << epilogue;
	}
	return codegen::script(language, codegen::comment(language, note.join(QLatin1Char('\n'))), headers,
			       all.join(QLatin1Char('\n')));
}
