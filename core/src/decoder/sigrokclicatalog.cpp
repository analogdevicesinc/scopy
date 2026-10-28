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

#include "decoder/sigrokclicatalog.h"

#include "common/scopyconfig.h"
#include "decoder/decoderlogger.h"
#include "pluginbase/preferences.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>

#include <algorithm>
#include <memory>
#include <vector>

namespace scopy {
namespace decoder {

SigrokCliCatalog::SigrokCliCatalog()
	: m_kCatalogId("sigrok-cli-catalog")
	, m_kMaxParallel(8)
{
#if defined __APPLE__
	const QString decoders = QCoreApplication::applicationDirPath() + "/../Resources/decoders";
#elif defined(__appimage__)
	const QString decoders = QCoreApplication::applicationDirPath() + "/../lib/decoders";
#else
	const QString decoders = QCoreApplication::applicationDirPath() + "/decoders";
#endif
	if(!qEnvironmentVariableIsSet("SIGROKDECODE_DIR") && QDir(decoders).exists())
		qputenv("SIGROKDECODE_DIR", QDir::cleanPath(decoders).toLocal8Bit());
}

SigrokCliCatalog::~SigrokCliCatalog()
{
	m_stopping = true;
	m_prime.waitForFinished();
}

void SigrokCliCatalog::setExecutableOverride(const QString &path)
{
	QMutexLocker lk(&m_mutex);
	m_exeOverride = path;
	m_cachedExe.clear();
}

QString SigrokCliCatalog::resolveCli() const
{
	QMutexLocker lk(&m_mutex);
	if(!m_cachedExe.isEmpty() && QFile::exists(m_cachedExe))
		return m_cachedExe;

	if(!m_exeOverride.isEmpty() && QFile::exists(m_exeOverride)) {
		m_cachedExe = m_exeOverride;
		return m_cachedExe;
	}

	const QString prefPath = Preferences::get("sigrok_cli_path").toString();
	if(!prefPath.isEmpty() && QFile::exists(prefPath)) {
		m_cachedExe = prefPath;
		return m_cachedExe;
	}

	const QString folder = scopy::config::executableFolderPath();
	if(!folder.isEmpty()) {
		const QString candidate = QDir(folder).absoluteFilePath(
#ifdef Q_OS_WIN
			"sigrok-cli.exe"
#else
			"sigrok-cli"
#endif
		);
		if(QFile::exists(candidate)) {
			m_cachedExe = candidate;
			return m_cachedExe;
		}
	}

	m_cachedExe = QStandardPaths::findExecutable("sigrok-cli");
	return m_cachedExe;
}

bool SigrokCliCatalog::isAvailable() const { return !resolveCli().isEmpty(); }

bool SigrokCliCatalog::run(const QStringList &args, const QByteArray &input, QByteArray &out, QString &err) const
{
	out.clear();
	err.clear();
	const QString exe = resolveCli();
	if(exe.isEmpty()) {
		err = QStringLiteral("sigrok-cli executable not found");
		return false;
	}

	QProcess proc;
	proc.start(exe, args);
	if(!proc.waitForStarted(2000)) {
		err = QStringLiteral("failed to start sigrok-cli: ") + proc.errorString();
		return false;
	}
	proc.write(input);
	proc.closeWriteChannel();

	const int timeoutMs = 10000 + static_cast<int>(std::min<qint64>(input.size() / 100, 600000));
	if(!proc.waitForFinished(timeoutMs)) {
		proc.kill();
		proc.waitForFinished(500);
		err = QStringLiteral("sigrok-cli timed out");
		return false;
	}

	out = proc.readAllStandardOutput();
	err = QString::fromUtf8(proc.readAllStandardError().trimmed());
	if(proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		if(err.isEmpty())
			err = QStringLiteral("sigrok-cli exited with status %1").arg(proc.exitCode());
		return false;
	}
	return true;
}

QList<QString> SigrokCliCatalog::decoders() const
{
	{
		QMutexLocker lk(&m_mutex);
		if(m_listedOnce)
			return m_order;
	}

	QByteArray out;
	QString err;
	if(!run({"-L"}, {}, out, err)) {
		if(m_logger)
			m_logger->warning(m_kCatalogId, QStringLiteral("-L: ") + err);
		return {};
	}

	QList<QString> order;
	QHash<QString, QString> desc;
	parseListing(QString::fromUtf8(out), order, desc);
	if(m_logger)
		m_logger->info(m_kCatalogId, QStringLiteral("found %1 decoders").arg(order.size()));

	QMutexLocker lk(&m_mutex);
	m_order = order;
	m_shortDesc = desc;
	m_listedOnce = true;
	return m_order;
}

QString SigrokCliCatalog::shortDescription(const QString &decoderId) const
{
	decoders();
	QMutexLocker lk(&m_mutex);
	return m_shortDesc.value(decoderId);
}

DecoderInfo SigrokCliCatalog::info(const QString &decoderId) const
{
	{
		QMutexLocker lk(&m_mutex);
		const auto it = m_info.constFind(decoderId);
		if(it != m_info.constEnd())
			return *it;
	}

	QByteArray out;
	QString err;
	if(!run({"--show", "-P", decoderId}, {}, out, err)) {
		if(m_logger)
			m_logger->warning(m_kCatalogId, QStringLiteral("--show %1: %2").arg(decoderId, err));
		return {};
	}
	if(out.isEmpty())
		return {};

	DecoderInfo di = parseShow(QString::fromUtf8(out), decoderId);
	// Fall back to catalog description if `--show` doesn't have one useful.
	if(di.description.isEmpty())
		di.description = shortDescription(decoderId);
	if(di.name.isEmpty())
		di.name = decoderId;
	QMutexLocker lk(&m_mutex);
	m_info.insert(decoderId, di);
	return di;
}

void SigrokCliCatalog::loadAll() const
{
	QMutexLocker lk(&m_mutex);
	if(m_prime.isRunning())
		return;
	m_prime = QtConcurrent::run([this]() {
		QList<QString> ids = decoders();
		QThreadPool pool;
		pool.setMaxThreadCount(m_kMaxParallel);
		QtConcurrent::blockingMap(&pool, ids, [this](const QString &id) {
			if(!m_stopping)
				(void)info(id);
		});
		if(m_logger) {
			QMutexLocker cacheLock(&m_mutex);
			m_logger->info(m_kCatalogId,
				       QStringLiteral("loadAll: cached %1 / %2").arg(m_info.size()).arg(ids.size()));
		}
	});
}

// Parse `sigrok-cli -L`: pick the "Supported protocol decoders:" block only.
void SigrokCliCatalog::parseListing(const QString &stdoutText, QList<QString> &orderOut,
				    QHash<QString, QString> &descOut)
{
	orderOut.clear();
	descOut.clear();

	enum class Section
	{
		None,
		Decoders,
		Other
	};
	Section section = Section::None;

	const QStringList lines = stdoutText.split('\n');
	for(const QString &raw : lines) {
		const QString line = raw;

		if(line.startsWith("Supported protocol decoders")) {
			section = Section::Decoders;
			continue;
		}
		if(line.startsWith("Supported ") && section == Section::Decoders) {
			section = Section::Other;
			continue;
		}
		if(section != Section::Decoders)
			continue;
		if(!line.startsWith("  "))
			continue; // section entries are indented

		// "  <id><padding><description>"
		const QString stripped = line.trimmed();
		const int space = stripped.indexOf(QRegularExpression("\\s+"));
		if(space < 0)
			continue;
		const QString id = stripped.left(space);
		const QString desc = stripped.mid(space).trimmed();
		if(id.isEmpty())
			continue;
		orderOut.append(id);
		descOut.insert(id, desc);
	}
}

// Parse `sigrok-cli --show -P <id>` into a DecoderInfo (name/desc/channels/
// options/annotation classes+rows/documentation).
DecoderInfo SigrokCliCatalog::parseShow(const QString &stdoutText, const QString &decoderId)
{
	DecoderInfo di;
	di.id = decoderId;

	enum class Section
	{
		None,
		InputIds,
		OutputIds,
		RequiredChannels,
		OptionalChannels,
		Options,
		AnnClasses,
		AnnRows,
		Documentation,
	};
	Section section = Section::None;
	QStringList docLines;

	static const QRegularExpression rxItem(R"(^-\s+(\S+?)(?:\s+\(([^)]*)\))?\s*:\s*(.*)$)");

	const QStringList lines = stdoutText.split('\n');
	for(const QString &raw : lines) {
		const QString line = raw;
		const QString trimmed = line.trimmed();

		// Section headers: unindented, end with ':'.
		if(!line.startsWith(' ') && !line.startsWith('\t') && trimmed.endsWith(':')) {
			const QString head = trimmed.left(trimmed.size() - 1);
			if(head == "Required channels")
				section = Section::RequiredChannels;
			else if(head == "Optional channels")
				section = Section::OptionalChannels;
			else if(head == "Options")
				section = Section::Options;
			else if(head == "Annotation classes")
				section = Section::AnnClasses;
			else if(head == "Annotation rows")
				section = Section::AnnRows;
			else if(head == "Documentation")
				section = Section::Documentation;
			else if(head == "Possible decoder input IDs")
				section = Section::InputIds;
			else if(head == "Possible decoder output IDs")
				section = Section::OutputIds;
			else
				section = Section::None;
			continue;
		}

		if(section == Section::Documentation) {
			docLines.append(line);
			continue;
		}

		if(section == Section::None) {
			if(trimmed.startsWith("Name:"))
				di.name = trimmed.mid(5).trimmed();
			else if(trimmed.startsWith("Long name:"))
				di.name = trimmed.mid(10).trimmed();
			else if(trimmed.startsWith("Description:"))
				di.description = trimmed.mid(12).trimmed();
			continue;
		}

		if(trimmed == "None." || trimmed.isEmpty())
			continue;
		if(!trimmed.startsWith("- "))
			continue;

		switch(section) {
		case Section::RequiredChannels:
		case Section::OptionalChannels: {
			const auto m = rxItem.match(trimmed);
			if(!m.hasMatch())
				break;
			ChannelInfo ch;
			ch.id = m.captured(1);
			ch.name = m.captured(2);
			ch.desc = m.captured(3);
			ch.required = (section == Section::RequiredChannels);
			if(ch.name.isEmpty())
				ch.name = ch.id;
			di.channels.append(ch);
			break;
		}
		case Section::Options: {
			OptionInfo o = parseOptionLine(trimmed);
			if(!o.id.isEmpty())
				di.options.append(o);
			break;
		}
		case Section::AnnClasses: {
			const int colon = trimmed.indexOf(':');
			if(colon < 0)
				break;
			const QString cid = trimmed.mid(2, colon - 2).trimmed();
			if(!cid.isEmpty())
				di.annotationClasses.append(cid);
			break;
		}
		case Section::AnnRows:
			di.annotationRows.append(trimmed.mid(2).trimmed());
			break;
		case Section::InputIds: {
			const QString tok = trimmed.mid(2).trimmed();
			if(!tok.isEmpty())
				di.inputIds.append(tok);
			break;
		}
		case Section::OutputIds: {
			const QString tok = trimmed.mid(2).trimmed();
			if(!tok.isEmpty())
				di.outputIds.append(tok);
			break;
		}
		case Section::Documentation:
		case Section::None:
			break;
		}
	}

	while(!docLines.isEmpty() && docLines.last().trimmed().isEmpty())
		docLines.removeLast();
	while(!docLines.isEmpty() && docLines.first().trimmed().isEmpty())
		docLines.removeFirst();
	di.documentation = docLines.join('\n');

	return di;
}

// Parses one "- <id>: <name> (choices?, default X)" option line.
// Detects Enum (quoted), numeric Enum, Int/Double, or String.
OptionInfo SigrokCliCatalog::parseOptionLine(const QString &content)
{
	OptionInfo o;
	if(!content.startsWith("- "))
		return o;
	const int colon = content.indexOf(':');
	if(colon < 0)
		return o;
	o.id = content.mid(2, colon - 2).trimmed();
	const QString rhs = content.mid(colon + 1).trimmed();

	int parenOpen = rhs.indexOf('(');
	int parenClose = rhs.lastIndexOf(')');
	QString name, paren;
	if(parenOpen > 0 && parenClose > parenOpen) {
		name = rhs.left(parenOpen).trimmed();
		paren = rhs.mid(parenOpen + 1, parenClose - parenOpen - 1).trimmed();
	} else {
		name = rhs;
	}
	o.name = name.isEmpty() ? o.id : name;

	if(paren.isEmpty()) {
		o.type = OptionType::String;
		return o;
	}

	// "default <token>" at the end of the paren block.
	QString defaultTok;
	{
		static const QRegularExpression rxDefault(R"((?:^|,\s*)default\s+(?:'([^']*)'|([^,]+))\s*$)");
		const auto m = rxDefault.match(paren);
		if(m.hasMatch()) {
			defaultTok = m.captured(1).isNull() ? m.captured(2).trimmed() : m.captured(1);
		}
	}
	o.defaultValue = defaultTok;

	if(paren.contains('\'')) {
		o.type = OptionType::Enum;
		static const QRegularExpression rxQ(R"('([^']*)')");
		auto it = rxQ.globalMatch(paren);
		while(it.hasNext()) {
			const auto m = it.next();
			const QString v = m.captured(1);
			if(!o.choices.contains(v))
				o.choices.append(v);
		}
		return o;
	}

	QStringList tokens;
	{
		const QStringList raw = paren.split(',', Qt::SkipEmptyParts);
		for(const QString &t : raw) {
			QString s = t.trimmed();
			if(s.startsWith("default "))
				continue;
			if(s.isEmpty())
				continue;
			tokens.append(s);
		}
	}

	auto isInt = [](const QString &s) {
		bool ok = false;
		s.toLongLong(&ok);
		return ok;
	};
	auto isDouble = [](const QString &s) {
		bool ok = false;
		s.toDouble(&ok);
		return ok;
	};

	if(tokens.size() >= 2) {
		bool anyDouble = false, allNumeric = true;
		for(const QString &t : tokens) {
			if(!isDouble(t)) {
				allNumeric = false;
				break;
			}
			if(!isInt(t))
				anyDouble = true;
		}
		if(allNumeric) {
			o.type = anyDouble ? OptionType::Double : OptionType::Int;
			o.choices = tokens;
			return o;
		}
		o.type = OptionType::Enum;
		o.choices = tokens;
		return o;
	}

	if(!defaultTok.isEmpty()) {
		if(isInt(defaultTok))
			o.type = OptionType::Int;
		else if(isDouble(defaultTok))
			o.type = OptionType::Double;
		else
			o.type = OptionType::String;
	} else {
		o.type = OptionType::String;
	}
	return o;
}

} // namespace decoder
} // namespace scopy
