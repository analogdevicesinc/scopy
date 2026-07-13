#include "decoder/SigrokCliCatalog.h"

#include "common/scopyconfig.h"
#include "decoder/DecoderLogger.h"
#include "pluginbase/preferences.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace scopy {
namespace decoder {

static constexpr const char *kCatalogId = "sigrok-cli-catalog";

SigrokCliCatalog::SigrokCliCatalog()  = default;
SigrokCliCatalog::~SigrokCliCatalog() = default;

void SigrokCliCatalog::setExecutableOverride(const QString &path)
{
	m_exeOverride = path;
	m_cachedExe.clear();
}

QString SigrokCliCatalog::resolveCli() const
{
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

QString SigrokCliCatalog::runCli(const QStringList &args, bool *ok) const
{
	if(ok) *ok = false;
	const QString exe = resolveCli();
	if(exe.isEmpty()) {
		if(m_logger)
			m_logger->warning(kCatalogId, QStringLiteral("sigrok-cli not found"));
		return {};
	}

	QProcess proc;
	proc.setProcessChannelMode(QProcess::MergedChannels);
	proc.start(exe, args);
	if(!proc.waitForStarted(2000)) {
		if(m_logger)
			m_logger->warning(kCatalogId,
				QStringLiteral("failed to start sigrok-cli: ") + proc.errorString());
		return {};
	}
	if(!proc.waitForFinished(10000)) {
		if(m_logger)
			m_logger->warning(kCatalogId, QStringLiteral("sigrok-cli timed out"));
		proc.kill();
		proc.waitForFinished(500);
		return {};
	}
	if(proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		if(m_logger)
			m_logger->warning(kCatalogId,
				QStringLiteral("sigrok-cli exit=%1 args=%2")
					.arg(proc.exitCode())
					.arg(args.join(' ')));
		return QString::fromUtf8(proc.readAll());
	}

	if(ok) *ok = true;
	return QString::fromUtf8(proc.readAll());
}

QList<QString> SigrokCliCatalog::decoders() const
{
	if(m_listedOnce)
		return m_order;
	m_listedOnce = true;

	bool ok = false;
	const QString out = runCli({"-L"}, &ok);
	if(!ok || out.isEmpty()) return {};

	parseListing(out, m_order, m_shortDesc);
	if(m_logger)
		m_logger->info(kCatalogId,
			QStringLiteral("found %1 decoders").arg(m_order.size()));
	return m_order;
}

QString SigrokCliCatalog::shortDescription(const QString &decoderId) const
{
	if(!m_listedOnce) decoders();
	return m_shortDesc.value(decoderId);
}

DecoderInfo SigrokCliCatalog::info(const QString &decoderId) const
{
	if(m_info.contains(decoderId)) return m_info.value(decoderId);

	bool ok = false;
	const QString out = runCli({"--show", "-P", decoderId}, &ok);
	if(!ok || out.isEmpty()) return {};

	DecoderInfo di = parseShow(out, decoderId);
	// Fall back to catalog description if `--show` doesn't have one useful.
	if(di.description.isEmpty())
		di.description = shortDescription(decoderId);
	if(di.name.isEmpty())
		di.name = decoderId;
	m_info.insert(decoderId, di);
	return di;
}

// Parse `sigrok-cli -L` output.
//
// The listing prints two sections:
//   Supported hardware drivers:
//     <id>  <description>
//   Supported protocol decoders:
//     <id>  <description>
// We only want the protocol-decoders block.
void SigrokCliCatalog::parseListing(const QString &stdoutText,
                                    QList<QString> &orderOut,
                                    QHash<QString, QString> &descOut)
{
	orderOut.clear();
	descOut.clear();

	enum class Section { None, Decoders, Other };
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
		if(section != Section::Decoders) continue;
		if(!line.startsWith("  ")) continue; // section entries are indented

		// "  <id><padding><description>"
		const QString stripped = line.trimmed();
		const int space = stripped.indexOf(QRegularExpression("\\s+"));
		if(space < 0) continue;
		const QString id   = stripped.left(space);
		const QString desc = stripped.mid(space).trimmed();
		if(id.isEmpty()) continue;
		orderOut.append(id);
		descOut.insert(id, desc);
	}
}

// Parse `sigrok-cli --show -P <id>` output. Sections we care about:
//   Name:
//   Long name:
//   Description:
//   Required channels:
//     - <id> (<name>): <desc>
//   Optional channels:
//     - <id> (<name>): <desc>
//   Options:
//     - <id>: <content>
//   Annotation classes:
//     - <id>: <desc>
//   Annotation rows:
//     - <id> (<name>): <class1>[, <class2>...]
DecoderInfo SigrokCliCatalog::parseShow(const QString &stdoutText,
                                        const QString &decoderId)
{
	DecoderInfo di;
	di.id = decoderId;

	enum class Section {
		None,
		RequiredChannels,
		OptionalChannels,
		Options,
		AnnClasses,
		AnnRows,
		Documentation,
	};
	Section section = Section::None;
	QStringList docLines;

	static const QRegularExpression rxItem(
		R"(^-\s+(\S+?)(?:\s+\(([^)]*)\))?\s*:\s*(.*)$)");

	const QStringList lines = stdoutText.split('\n');
	for(const QString &raw : lines) {
		const QString line = raw;
		const QString trimmed = line.trimmed();

		// Section headers are unindented and end with ':'.
		if(!line.startsWith(' ') && !line.startsWith('\t')
		   && trimmed.endsWith(':')) {
			const QString head = trimmed.left(trimmed.size() - 1);
			if(head == "Required channels")      section = Section::RequiredChannels;
			else if(head == "Optional channels") section = Section::OptionalChannels;
			else if(head == "Options")           section = Section::Options;
			else if(head == "Annotation classes")section = Section::AnnClasses;
			else if(head == "Annotation rows")   section = Section::AnnRows;
			else if(head == "Documentation")     section = Section::Documentation;
			else                                 section = Section::None;
			continue;
		}

		// Documentation is a free-form multi-line block; capture verbatim
		// until EOF or the next unindented header (handled above).
		if(section == Section::Documentation) {
			docLines.append(line);
			continue;
		}

		// Single-value top-level fields.
		if(section == Section::None) {
			if(trimmed.startsWith("Name:"))
				di.name = trimmed.mid(5).trimmed();
			else if(trimmed.startsWith("Long name:"))
				di.name = trimmed.mid(10).trimmed();
			else if(trimmed.startsWith("Description:"))
				di.description = trimmed.mid(12).trimmed();
			continue;
		}

		if(trimmed == "None." || trimmed.isEmpty()) continue;
		if(!trimmed.startsWith("- ")) continue;

		switch(section) {
		case Section::RequiredChannels:
		case Section::OptionalChannels: {
			// "- rx (RX): UART receive line"
			const auto m = rxItem.match(trimmed);
			if(!m.hasMatch()) break;
			ChannelInfo ch;
			ch.id       = m.captured(1);
			ch.name     = m.captured(2);
			ch.desc     = m.captured(3);
			ch.required = (section == Section::RequiredChannels);
			if(ch.name.isEmpty()) ch.name = ch.id;
			di.channels.append(ch);
			break;
		}
		case Section::Options: {
			// "- baudrate: Baud rate (default 115200)"
			OptionInfo o = parseOptionLine(trimmed);
			if(!o.id.isEmpty()) di.options.append(o);
			break;
		}
		case Section::AnnClasses: {
			// "- rx-data: RX data"
			const int colon = trimmed.indexOf(':');
			if(colon < 0) break;
			const QString cid = trimmed.mid(2, colon - 2).trimmed();
			if(!cid.isEmpty()) di.annotationClasses.append(cid);
			break;
		}
		case Section::AnnRows: {
			// "- rx-data-vals (RX data): rx-data, rx-start, ..."
			di.annotationRows.append(trimmed.mid(2).trimmed());
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

// Parses one indented "- <id>: <content>" line from the Options section.
//
// Content shapes we handle (from real sigrok decoders):
//   "Baud rate (default 115200)"
//   "Parity ('none', 'odd', 'even', ..., default 'none')"
//   "Stop bits (0.0, 0.5, 1.0, 1.5, 2.0, default 1.0)"
//   "Word size (default 8)"
//   "Address format ('shifted', 'unshifted', default 'shifted')"
//
// Strategy:
//   1. strip leading "- <id>: " → id + content
//   2. name = content up to the first "(" (or full content)
//   3. content inside outermost parens = choices/default block
//   4. detect quoted enum values → Enum
//      else detect numeric values → Int or Double
//      else fallback to String
OptionInfo SigrokCliCatalog::parseOptionLine(const QString &content)
{
	OptionInfo o;

	// content is like: "- baudrate: Baud rate (default 115200)"
	if(!content.startsWith("- ")) return o;
	const int colon = content.indexOf(':');
	if(colon < 0) return o;
	o.id = content.mid(2, colon - 2).trimmed();
	const QString rhs = content.mid(colon + 1).trimmed();

	// Split off "(<parenthesized>)"
	int parenOpen  = rhs.indexOf('(');
	int parenClose = rhs.lastIndexOf(')');
	QString name, paren;
	if(parenOpen > 0 && parenClose > parenOpen) {
		name  = rhs.left(parenOpen).trimmed();
		paren = rhs.mid(parenOpen + 1, parenClose - parenOpen - 1).trimmed();
	} else {
		name = rhs;
	}
	o.name = name.isEmpty() ? o.id : name;

	if(paren.isEmpty()) {
		o.type = OptionType::String;
		return o;
	}

	// Extract default value first: look for "default <token>" at the end.
	QString defaultTok;
	{
		static const QRegularExpression rxDefault(
			R"((?:^|,\s*)default\s+(?:'([^']*)'|([^,]+))\s*$)");
		const auto m = rxDefault.match(paren);
		if(m.hasMatch()) {
			defaultTok = m.captured(1).isNull() ? m.captured(2).trimmed()
			                                   : m.captured(1);
		}
	}
	o.defaultValue = defaultTok;

	// If paren contains any quoted string: enum of quoted values.
	if(paren.contains('\'')) {
		o.type = OptionType::Enum;
		static const QRegularExpression rxQ(R"('([^']*)')");
		auto it = rxQ.globalMatch(paren);
		while(it.hasNext()) {
			const auto m = it.next();
			const QString v = m.captured(1);
			if(!o.choices.contains(v)) o.choices.append(v);
		}
		// Some enums list the same value as default too; that's fine.
		return o;
	}

	// No quotes: comma-separated tokens. Some are numeric-value enums
	// (e.g. stop_bits "0.0, 0.5, 1.0, 1.5, 2.0, default 1.0"), some are
	// bare "(default 8)".
	QStringList tokens;
	{
		const QStringList raw = paren.split(',', Qt::SkipEmptyParts);
		for(const QString &t : raw) {
			QString s = t.trimmed();
			if(s.startsWith("default ")) continue;
			if(s.isEmpty()) continue;
			tokens.append(s);
		}
	}

	auto isInt = [](const QString &s) {
		bool ok = false; s.toLongLong(&ok); return ok;
	};
	auto isDouble = [](const QString &s) {
		bool ok = false; s.toDouble(&ok); return ok;
	};

	if(tokens.size() >= 2) {
		// numeric enum
		bool anyDouble = false, allNumeric = true;
		for(const QString &t : tokens) {
			if(!isDouble(t)) { allNumeric = false; break; }
			if(!isInt(t))    anyDouble = true;
		}
		if(allNumeric) {
			o.type    = anyDouble ? OptionType::Double : OptionType::Int;
			o.choices = tokens; // treat as an enum of numeric strings
			return o;
		}
		// mixed → fall through to String enum
		o.type    = OptionType::Enum;
		o.choices = tokens;
		return o;
	}

	// Zero or one token (typically just "default N") → numeric or string.
	if(!defaultTok.isEmpty()) {
		if(isInt(defaultTok))       o.type = OptionType::Int;
		else if(isDouble(defaultTok)) o.type = OptionType::Double;
		else                          o.type = OptionType::String;
	} else {
		o.type = OptionType::String;
	}
	return o;
}

} // namespace decoder
} // namespace scopy
