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

#include "component/component_export.h"
#include "core/callrecord.h"
#include "core/codeemitter.h"

#include <QList>
#include <QString>
#include <QStringList>

#include <memory>
#include <mutex>

namespace scopy::component {

// Code emitters per (library, language): turns command log entries into one script. The instance
// holds the libiio emitters (C / Python) of the API version of the linked IIO backend, so they
// match the calls it records; other libraries register theirs at plugin init() (e.g. remote-bus).
class COMPONENT_EXPORT CodeEmitterRegistry
{
public:
	// Consecutive repeats of a command sequence up to this many calls long fold into one loop.
	static constexpr int MAX_LOOP_PERIOD = 16;

	CodeEmitterRegistry(); // empty
	~CodeEmitterRegistry();
	CodeEmitterRegistry(const CodeEmitterRegistry &) = delete;
	CodeEmitterRegistry &operator=(const CodeEmitterRegistry &) = delete;

	static CodeEmitterRegistry *GetInstance();

	// Replaces an emitter of the same library and language.
	void add(std::shared_ptr<CodeEmitter> emitter);
	bool remove(const QString &library, const QString &language);
	std::shared_ptr<CodeEmitter> emitter(const QString &library, const QString &language) const;
	QStringList libraries() const;

	// Languages with an emitter for every one of `libraries` (C, C++, Python first, in that order);
	// empty for no libraries.
	QStringList languagesFor(const QStringList &libraries) const;
	// Libraries of the calls in `entries`, in order of first use.
	static QStringList librariesOf(const QList<CommandLogEntry> &entries);

	// One script repeating the calls of `entries` in time order: header comment (what is not
	// reproduced), one prologue per library, the calls (repeats as loops when options.loops), one
	// epilogue per library. Empty when the entries have no calls. A call of a library without an
	// emitter for `language` becomes a comment.
	QString exportScript(const QList<CommandLogEntry> &entries, const QString &language,
			     const EmitOptions &options = {}) const;

private:
	mutable std::mutex m_mutex;
	QList<std::shared_ptr<CodeEmitter>> m_emitters;
};

} // namespace scopy::component
