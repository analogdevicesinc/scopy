#pragma once

#include "scopy-core_export.h"

#include <deque>
#include <optional>
#include <variant>
#include <QString>
#include <QVector>

namespace scopy {
namespace acq {

struct Annotation
{
	quint64 startSample{0};
	quint64 endSample{0};
	QString decoder; // e.g. "uart-1"
	QString klass;   // annotation class / row
	QString text;
	int     severity{0};

	// Numeric payload, when the producer knows it. `text` is a *display*
	// string, so recovering a byte from it means guessing the radix the
	// producer chose to print in — which silently corrupts data when the
	// guess is wrong. Producers that have the value should set it here;
	// consumers should prefer it and fall back to parsing `text` only for
	// producers that cannot (e.g. scraping a child process's stdout).
	std::optional<quint64> value;
};

// Radix of an annotation's `text` field. Only needed for the fallback path
// where `Annotation::value` is unset.
enum class TextRadix
{
	Hex = 0,
	Dec,
	Oct,
	Bin,
	Ascii, // text is the character itself, not a numeral
};

SCOPY_CORE_EXPORT const char *textRadixName(TextRadix r);
// Parses "hex"/"16"/"dec"/"10"/"oct"/"8"/"bin"/"2"/"ascii"/"char"
// (case-insensitive).
SCOPY_CORE_EXPORT bool textRadixFromString(const QString &s, TextRadix &out);

// Stream-level description of an annotation stream, held once per DataKey
// rather than repeated on every Annotation (a stream carries thousands of
// records per cycle and these fields are constant across them).
//
// Producers declare this; consumers read it. That keeps consumers free of
// any knowledge of *which* decoder produced the stream — a vendor decoder
// publishes the same fields as the built-in sigrok path.
struct AnnotationStreamInfo
{
	// Canonical id of the producing decoder, e.g. "spi". Selects the
	// extractor when this stream is fed to another decoder as input.
	QString producerId;

	// Radix of `Annotation::text` for records with no `value` set.
	TextRadix textRadix{TextRadix::Hex};

	// Timeline that startSample/endSample are expressed on.
	double sampleRate{0.0};

	// Line rate, when the protocol has one. 0 = unknown/not applicable.
	quint64 bitrate{0};
};

enum class SampleType { Float32, Float64, Int32, Int16, Int8, UInt8, Annotation };

using SampleVariant = std::variant<
	QVector<float>,        // index 0 -> SampleType::Float32
	QVector<double>,       // index 1 -> SampleType::Float64
	QVector<qint32>,       // index 2 -> SampleType::Int32
	QVector<qint16>,       // index 3 -> SampleType::Int16
	QVector<qint8>,        // index 4 -> SampleType::Int8
	QVector<quint8>,       // index 5 -> SampleType::UInt8
	QVector<Annotation>>;  // index 6 -> SampleType::Annotation

// sampleTypeOf() casts variant.index() to SampleType, so the two sequences
// above must stay in sync.
static_assert(std::variant_size_v<SampleVariant> == 7,
	      "SampleVariant and SampleType enum must have the same number of alternatives");

inline SampleType sampleTypeOf(const SampleVariant &v) noexcept
{
	return static_cast<SampleType>(v.index());
}

SCOPY_CORE_EXPORT QString sampleTypeName(SampleType t);

// Non-owning float view over a chunk. Points into the chunk itself when it is
// already Float32, otherwise into `scratch`. Valid only while both outlive it.
struct FloatView
{
	const float *data{nullptr};
	int          size{0};
};

// Numeric conversions. Annotation chunks convert to empty (they carry no
// numeric samples); every other alternative converts elementwise.
SCOPY_CORE_EXPORT QVector<float>  toFloat(const SampleVariant &v);
SCOPY_CORE_EXPORT FloatView       toFloatView(const SampleVariant &v, QVector<float> &scratch);
SCOPY_CORE_EXPORT QVector<quint8> toBits(const SampleVariant &v); // != 0 -> 1

// Drops everything past the first `n` samples. Annotation chunks pass through
// untouched — use shiftAnnotations() for those.
//
// Turns a right-anchored window of length n+extra into one ending `extra` samples
// earlier, i.e. centred on a sample sitting `extra` positions from its right end.
SCOPY_CORE_EXPORT SampleVariant truncateWindow(const SampleVariant &v, int n);

// Offsets every annotation by `delta` and drops those leaving [0, n). Annotations
// are produced against their own window, so re-anchoring them to a different one
// is a shift rather than a truncation. Non-annotation chunks pass through.
SCOPY_CORE_EXPORT SampleVariant shiftAnnotations(const SampleVariant &v, int n, int delta);

// Bounded chunk history for one stream, newest at index 0. Capacity is owned by
// the DataStore (see DataStore::claimDepth); pushing past it drops the oldest.
class SCOPY_CORE_EXPORT SampleBuffer
{
public:
	void        setCapacity(std::size_t n);
	std::size_t capacity() const { return m_capacity; }

	void push(SampleVariant vec);

	// Drop every chunk but keep capacity and declared type.
	void clearChunks() { m_history.clear(); }

	// Empty chunk (Float32) when `index` is out of range, so callers can read
	// unconditionally; use depth() to distinguish absent from empty.
	const SampleVariant &sample(std::size_t index = 0) const;

	std::size_t depth() const { return m_history.size(); }
	bool        empty() const { return m_history.empty(); }

	// Sample count of the newest chunk.
	std::size_t size() const;

	// Type of the stream, remembered across clearChunks(). nullopt until the
	// first push — an unwritten stream has no type to report.
	std::optional<SampleType> type() const { return m_type; }

	// The newest `plotSize` samples, oldest-first, concatenated across as many
	// chunks as needed and right-anchored (a partial window is left-padded by
	// nothing — it is simply shorter). Annotation streams are returned as-is
	// from the newest chunk since annotations carry their own sample offsets.
	SampleVariant window(int plotSize) const;

private:
	std::size_t               m_capacity{1};
	std::deque<SampleVariant> m_history;
	std::optional<SampleType> m_type;
};

} // namespace acq
} // namespace scopy
