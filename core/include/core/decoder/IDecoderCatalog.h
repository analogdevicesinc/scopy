#pragma once

#include "scopy-core_export.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {

// Metadata about one decoder option, parsed from the underlying decoder CLI /
// library. Backend-agnostic: any catalog implementation (sigrok-cli, libsigrok,
// a bespoke Rust CLI, …) fills these structs and DecoderPanel/DecoderEditor
// consume them without further knowledge.
enum class OptionType
{
	Enum,   // finite set of string choices → combo box
	Int,    // integer (may be signed)
	Double, // floating-point
	String, // free-form
};

struct SCOPY_CORE_EXPORT OptionInfo
{
	QString      id;             // e.g. "baudrate"
	QString      name;           // e.g. "Baud rate"
	OptionType   type{OptionType::String};
	QString      defaultValue;   // stringified default
	QStringList  choices;        // populated when type == Enum
	double       minVal{0.0};    // unused unless numeric range known
	double       maxVal{0.0};
};

struct SCOPY_CORE_EXPORT ChannelInfo
{
	QString id;      // e.g. "rx", "scl"
	QString name;    // e.g. "RX", "SCL"
	QString desc;    // long description
	bool    required{false};
};

struct SCOPY_CORE_EXPORT DecoderInfo
{
	QString              id;          // "uart"
	QString              name;        // "Universal Asynchronous Receiver/Transmitter"
	QString              description; // short description
	QString              documentation; // multi-line free-form docs
	QList<ChannelInfo>   channels;    // required first, then optional
	QList<OptionInfo>    options;
	QStringList          annotationRows;    // e.g. "rx-data-vals (RX data)"…
	QStringList          annotationClasses; // e.g. "rx-data", "tx-data"…
};

// Read-only view over "what decoders are available and what do they look
// like". Owned and driven by the composition root (e.g. a plugin controller);
// consumed by DecoderPanel/DecoderEditor.
//
// Implementations may block on I/O (spawning a CLI, reading a library) —
// callers should treat method calls as potentially slow on first invocation
// per decoder id and cache-friendly thereafter (the SigrokCliCatalog
// implementation caches internally).
//
// Threading: consumers call from the main thread. Implementations may not
// assume they run on any particular thread beyond that.
class SCOPY_CORE_EXPORT IDecoderCatalog
{
public:
	virtual ~IDecoderCatalog() = default;

	// True if the backing tool/library is present and usable. When false,
	// decoders() returns an empty list and info() returns a default-
	// constructed DecoderInfo (isValid() == false).
	virtual bool           isAvailable() const = 0;

	// Full list of decoder ids in a stable, backend-defined order.
	virtual QList<QString> decoders() const = 0;

	// Short one-line description shown in the picker list next to the id.
	// Empty string is acceptable if the backend has no such summary.
	virtual QString        shortDescription(const QString &decoderId) const = 0;

	// Full metadata for a single decoder. Returns a default-constructed
	// DecoderInfo (isValid() == false) on error / unknown id.
	virtual DecoderInfo    info(const QString &decoderId) const = 0;

	static bool isValid(const DecoderInfo &d) { return !d.id.isEmpty(); }
};

} // namespace decoder
} // namespace scopy
