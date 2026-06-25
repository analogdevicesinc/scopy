#pragma once

#include "scopy-core_export.h"

#include <cstdint>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace scopy {
namespace decoder {

// ChannelMap: associates a logical decoder role (e.g. "tx", "scl") with
// a bit position (LSB-first) inside the packed sample stream.
struct ChannelMap
{
	std::string role;
	int         bitIndex{0};
};

// Configuration passed to IDecoderBackend::decode(). Backends may stack
// multiple decoders by interpreting decoderId as comma-separated.
struct DecoderConfig
{
	std::string                        decoderId;
	double                             sampleRate{1.0e6};
	int                                numChannels{1};
	std::vector<ChannelMap>            channels;
	std::map<std::string, std::string> options;
	std::map<std::string, std::string> meta;
};

// Annotation produced by a backend for a single buffer. Sample indices
// are buffer-local (0 .. nSamples-1 of the buffer just passed to decode()).
struct AnnotationC
{
	uint64_t    start{0};
	uint64_t    end{0};
	std::string decoder;
	std::string klass;
	std::string text;
	int         severity{0};
};

// One-shot pluggable decoder backend.
//
// Threading: an instance is owned and called exclusively from the engine
// worker thread (the thread that calls ProcessorBlock::process()). Each
// decode() call is fully self-contained — no state is preserved across
// calls. Backends spawning child processes (e.g. sigrok-cli) create and
// tear them down within a single decode() invocation.
class SCOPY_CORE_EXPORT IDecoderBackend
{
public:
	virtual ~IDecoderBackend() = default;

	// Decode one buffer of packed samples.
	//
	// data: packed samples, unitsize = ceil(numChannels / 8) bytes/sample,
	//       channel i = bit i, LSB-first within each byte.
	// nSamples: number of samples in 'data' (NOT bytes).
	// out:  cleared by the backend; populated with all annotations the
	//       decoder produces for exactly this buffer.
	//
	// Returns true on success (including the case where 'out' is empty),
	// false on hard error (in which case lastError() is populated).
	virtual bool decode(const DecoderConfig &cfg,
	                    const uint8_t *data, std::size_t nSamples,
	                    std::vector<AnnotationC> &out) = 0;

	virtual std::string lastError() const = 0;

	// Optional introspection (used by future settings panel).
	virtual std::vector<std::string> supportedDecoders() const { return {}; }
	virtual std::map<std::string, std::string>
	decoderOptions(const std::string & /*decoderId*/) const { return {}; }
};

} // namespace decoder
} // namespace scopy
