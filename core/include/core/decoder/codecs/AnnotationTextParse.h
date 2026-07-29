#pragma once

#include <QChar>
#include <QString>

#include <cstdint>

namespace scopy {
namespace decoder {
namespace codecs {

// Parse a sigrok annotation text into a byte. Sigrok formats data as
// "<label>: <value>" or bare "<value>"; take the last whitespace token,
// strip optional "0x" prefix, then convert as `radix`. Returns false if
// the token isn't a valid number in that radix.
inline bool parseAnnotationByte(const QString &text, int radix, uint8_t &out)
{
	const QString trimmed = text.trimmed();
	QString tok = trimmed;
	const int sp = trimmed.lastIndexOf(QLatin1Char(' '));
	if(sp >= 0) tok = trimmed.mid(sp + 1);
	if(tok.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
		tok = tok.mid(2);

	bool ok = false;
	const unsigned v = tok.toUInt(&ok, radix);
	if(!ok) return false;
	out = static_cast<uint8_t>(v & 0xFFu);
	return true;
}

// Map "hex"/"dec"/"bin" (case-insensitive) to a numeric radix. Anything
// else falls back to `def`.
inline int radixFromString(const QString &s, int def = 16)
{
	const QString r = s.toLower();
	if(r == QStringLiteral("hex")) return 16;
	if(r == QStringLiteral("dec")) return 10;
	if(r == QStringLiteral("bin")) return 2;
	return def;
}

} // namespace codecs
} // namespace decoder
} // namespace scopy
