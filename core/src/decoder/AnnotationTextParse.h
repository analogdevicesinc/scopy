#pragma once

#include <core/acq_engine/SampleBuffer.h>

#include <QString>

#include <cstdint>

namespace scopy {
namespace decoder {

// Recover a byte from an annotation's *display* text.
//
// This is a fallback. Annotation::value is the reliable path; text parsing
// only exists for producers that scrape a child process's stdout and never
// had the numeric value in hand. It is inherently lossy: "105" is 0x69 in
// decimal and 0x105 in hex, and nothing in the string says which the producer
// meant — hence the explicit radix, which must come from the producer's
// AnnotationStreamInfo rather than a guess.
//
// Why a parse failed. The distinction matters: a payload class whose text
// holds no numeral at all is normal (sigrok's i2c PD reuses "address-write"
// for the R/W-bit row, text "Write"), whereas a numeral that will not read in
// the declared radix means the producer's declaration is wrong and every byte
// it did accept is suspect. Lumping them together would fire that warning on
// every i2c capture and train the reader to ignore it.
enum class ByteParse
{
	Ok,
	NotNumeric,    // no numeral present; this row carries no payload
	RadixMismatch, // looks like a numeral, but not one in `radix`
};

// Sigrok formats data as "<label>: <value>" or bare "<value>"; take the last
// whitespace-separated token and strip an optional "0x"/"0b" prefix. Ascii
// instead takes the single remaining character's code point.
inline ByteParse parseAnnotationByteEx(const QString &text, acq::TextRadix radix,
                                       uint8_t &out)
{
	const QString trimmed = text.trimmed();
	QString       tok     = trimmed;
	const int     sp      = trimmed.lastIndexOf(QLatin1Char(' '));
	if(sp >= 0)
		tok = trimmed.mid(sp + 1);
	if(tok.isEmpty())
		return ByteParse::NotNumeric;

	if(radix == acq::TextRadix::Ascii) {
		// Producers print the character itself; anything longer than one
		// code unit is not a byte we can recover.
		if(tok.size() != 1)
			return ByteParse::NotNumeric;
		const ushort u = tok.at(0).unicode();
		if(u > 0xFF)
			return ByteParse::NotNumeric;
		out = static_cast<uint8_t>(u);
		return ByteParse::Ok;
	}

	int base = 16;
	switch(radix) {
	case acq::TextRadix::Hex: base = 16; break;
	case acq::TextRadix::Dec: base = 10; break;
	case acq::TextRadix::Oct: base = 8;  break;
	case acq::TextRadix::Bin: base = 2;  break;
	case acq::TextRadix::Ascii: break; // handled above
	}

	if(base == 16 && tok.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
		tok = tok.mid(2);
	else if(base == 2 && tok.startsWith(QStringLiteral("0b"), Qt::CaseInsensitive))
		tok = tok.mid(2);

	bool           ok = false;
	const unsigned v  = tok.toUInt(&ok, base);
	if(ok) {
		out = static_cast<uint8_t>(v & 0xFFu);
		return ByteParse::Ok;
	}

	// Distinguish "not a number" from "not a number *in this base*": retry in
	// the widest base. "1f" reads in 16 but not 10, so a dec declaration on a
	// hex producer lands here; "Write" reads in nothing and is just a label.
	bool okHex = false;
	(void)tok.toUInt(&okHex, 16);
	return okHex ? ByteParse::RadixMismatch : ByteParse::NotNumeric;
}

// Convenience overload: any failure is just a failure.
inline bool parseAnnotationByte(const QString &text, acq::TextRadix radix, uint8_t &out)
{
	return parseAnnotationByteEx(text, radix, out) == ByteParse::Ok;
}

} // namespace decoder
} // namespace scopy
