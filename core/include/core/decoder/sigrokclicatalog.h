/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "scopy-core_export.h"
#include "idecodercatalog.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {

class DecoderLogger;

// IDecoderCatalog backed by `sigrok-cli`. Lazy in-memory cache; blocking
// QProcess calls (call from a worker thread if latency matters).
class SCOPY_CORE_EXPORT SigrokCliCatalog : public IDecoderCatalog
{
public:
	SigrokCliCatalog();
	~SigrokCliCatalog() override;

	// Resolution order: override, "sigrok_cli_path" pref, scopy exe dir, $PATH.
	void setExecutableOverride(const QString &path);
	QString resolveCli() const;

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	// IDecoderCatalog
	bool isAvailable() const override;
	QList<QString> decoders() const override;
	QString shortDescription(const QString &decoderId) const override;
	DecoderInfo info(const QString &decoderId) const override;

	// Parallel eager cache prime via a small QProcess pool. Blocking.
	void loadAll() const override;

private:
	QString runCli(const QStringList &args, bool *ok = nullptr) const;
	static void parseListing(const QString &stdoutText, QList<QString> &orderOut, QHash<QString, QString> &descOut);
	static DecoderInfo parseShow(const QString &stdoutText, const QString &decoderId);
	static OptionInfo parseOptionLine(const QString &content);

	const char *m_kCatalogId;
	int m_kMaxParallel;

	mutable QString m_exeOverride;
	mutable QString m_cachedExe;

	mutable bool m_listedOnce{false};
	mutable QList<QString> m_order;		     // decoder ids
	mutable QHash<QString, QString> m_shortDesc; // id → short desc
	mutable QHash<QString, DecoderInfo> m_info;  // id → info (lazy)

	DecoderLogger *m_logger{nullptr};
};

} // namespace decoder
} // namespace scopy
