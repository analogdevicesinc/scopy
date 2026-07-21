#pragma once

#include "scopy-core_export.h"

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

namespace scopy {
namespace decoder {

// Backend-agnostic decoder option metadata. Consumed by DecoderPanel/Editor
// without further knowledge of the source (sigrok-cli, libsigrok, ...).
enum class OptionType
{
	Enum,   // combo box
	Int,
	Double,
	String,
};

struct SCOPY_CORE_EXPORT OptionInfo
{
	QString      id;            // e.g. "baudrate"
	QString      name;
	OptionType   type{OptionType::String};
	QString      defaultValue;
	QStringList  choices;       // populated when type == Enum
	double       minVal{0.0};   // unused unless numeric range known
	double       maxVal{0.0};
};

struct SCOPY_CORE_EXPORT ChannelInfo
{
	QString id;                 // "rx", "scl"
	QString name;
	QString desc;
	bool    required{false};
};

struct SCOPY_CORE_EXPORT DecoderInfo
{
	QString              id;                // "uart"
	QString              name;
	QString              description;
	QString              documentation;
	QStringList          inputIds;          // consumed decoder types
	QStringList          outputIds;         // produced decoder types
	QList<ChannelInfo>   channels;          // required first, then optional
	QList<OptionInfo>    options;
	QStringList          annotationRows;
	QStringList          annotationClasses;
};

// Read-only view over available decoders. Called from the main thread.
// Implementations may block on first access (CLI spawn, library scan) and
// should cache internally.
class SCOPY_CORE_EXPORT IDecoderCatalog
{
public:
	virtual ~IDecoderCatalog() = default;

	// False → decoders() empty, info() returns an invalid DecoderInfo.
	virtual bool           isAvailable() const = 0;

	virtual QList<QString> decoders() const = 0;
	virtual QString        shortDescription(const QString &decoderId) const = 0;

	// Invalid DecoderInfo (isValid() == false) on error / unknown id.
	virtual DecoderInfo    info(const QString &decoderId) const = 0;

	// Eagerly fill the metadata cache. Blocking; call from a worker thread
	// if latency matters. Backends override to parallelize.
	virtual void loadAll() const
	{
		for(const QString &id : decoders()) (void)info(id);
	}

	static bool isValid(const DecoderInfo &d) { return !d.id.isEmpty(); }
};

} // namespace decoder
} // namespace scopy
