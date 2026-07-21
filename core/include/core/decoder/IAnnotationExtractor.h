#pragma once

#include "scopy-core_export.h"
#include "IDecoderBackend.h"

#include <QString>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace scopy {
namespace decoder {

// One symbol produced by an extractor. `tag` classifies the symbol so the
// downstream encoder can frame it (e.g. i2c uses "start"/"stop").
// `upstreamAnnIndex` lets downstream annotations be remapped onto the
// upstream timeline.
struct ExtractedSymbol
{
	uint8_t byte{0};
	QString tag; // data | addr-r | addr-w | start | stop | mosi | miso
	int     upstreamAnnIndex{-1};
};

// Turns an upstream decoder's flat annotations into ExtractedSymbols.
// Registered by canonical upstream decoder id ("uart", "i2c"...). Stateless.
class SCOPY_CORE_EXPORT IAnnotationExtractor
{
public:
	virtual ~IAnnotationExtractor() = default;

	virtual QString upstreamId() const = 0;

	// opts = flat "annIn.*" section of DecoderConfig::meta (prefix stripped).
	virtual bool extract(const std::vector<AnnotationC> &in,
	                     const std::map<std::string, std::string> &opts,
	                     std::vector<ExtractedSymbol> &out,
	                     QString *err) = 0;
};

} // namespace decoder
} // namespace scopy
