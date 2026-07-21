#pragma once

#include "scopy-core_export.h"

#include <cstdint>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace scopy {
namespace decoder {

// Maps a decoder role (e.g. "tx", "scl") to a bit position (LSB-first)
// in the packed sample stream.
struct ChannelMap
{
	std::string role;
	int         bitIndex{0};
};

// One stage in a decoder stack. stage[0] reads raw logic; stage[i>0]
// consumes the previous stage's output and usually has no channel bindings.
struct DecoderStage
{
	std::string                        decoderId;
	std::vector<ChannelMap>            channels;
	std::map<std::string, std::string> options;
};

// DigitalSamples → decode() with packed bits; Annotations → decodeAnnotations()
// reads a producing DecoderInstance's annotation output DataKey.
enum class RootInput { DigitalSamples, Annotations };

// Reference to another DecoderInstance's stage output; resolved to a DataKey.
struct AnnotationInputRef
{
	std::string sourceUid;
	int         sourceStageIndex{0};
};

// Passed to IDecoderBackend::decode(). Always describes a linear stack
// (single-decoder = stack.size() == 1). When rootInput == Annotations,
// meta carries "annIn.*" codec params (upstreamId, samplerate, bitrate,
// frameformat, textinput, ...).
struct DecoderConfig
{
	double                             sampleRate{1.0e6};
	int                                numChannels{1};
	std::vector<DecoderStage>          stack;
	std::map<std::string, std::string> meta;

	RootInput          rootInput{RootInput::DigitalSamples};
	AnnotationInputRef annotationInput;
};

// Annotation produced by a backend for a single buffer. start/end are
// buffer-local sample indices.
struct AnnotationC
{
	uint64_t    start{0};
	uint64_t    end{0};
	std::string decoder;   // canonical decoderId (no "-N" stack suffix)
	std::string klass;
	std::string text;
	int         severity{0};
	int         stageIndex{0}; // 0 = root
};

// One-shot pluggable decoder backend. Owned and called exclusively from
// the engine worker thread; each decode() call is self-contained (no
// state kept across calls). Backends that spawn children (e.g. sigrok-cli)
// spin them up and tear them down within one call.
class SCOPY_CORE_EXPORT IDecoderBackend
{
public:
	virtual ~IDecoderBackend() = default;

	// data: packed samples, unitsize = ceil(numChannels/8) bytes/sample,
	//       channel i in bit i, LSB-first per byte. nSamples is samples, not bytes.
	// out:  cleared then filled. Returns false on hard error (lastError() populated).
	virtual bool decode(const DecoderConfig &cfg,
	                    const uint8_t *data, std::size_t nSamples,
	                    std::vector<AnnotationC> &out) = 0;

	// True iff this backend can consume annotations as input under cfg.
	virtual bool acceptsAnnotationInput(const DecoderConfig &cfg) const
	{
		(void)cfg;
		return false;
	}

	// Semantics mirror decode(). Sample indices in 'out' must be on the
	// source (upstream) timeline.
	virtual bool decodeAnnotations(const DecoderConfig &cfg,
	                               const std::vector<AnnotationC> &in,
	                               std::vector<AnnotationC> &out)
	{
		(void)cfg;
		(void)in;
		out.clear();
		return false;
	}

	virtual std::string lastError() const = 0;
};

} // namespace decoder
} // namespace scopy
