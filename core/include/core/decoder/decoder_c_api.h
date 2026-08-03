#ifndef SCOPY_DECODER_C_API_H
#define SCOPY_DECODER_C_API_H

/*
 * Pure-C ABI for vendor decoder libraries loaded by DynamicLibBackend.
 * Only raw pointers and lengths cross this boundary. Configuration is a
 * JSON string; keys mirror scopy::decoder::DecoderConfig.
 *
 * decode() is stateless: one call decodes one buffer. Sample indices in
 * delivered annotations are buffer-local (0 .. n_samples-1).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Annotation record. Strings are backend-owned and only valid for the
 * duration of the callback.
 *
 * `text` is a display string, so it does not identify its own radix. A
 * library that knows the numeric payload must set has_value=1 and fill
 * `value`; consumers prefer it and only fall back to parsing `text` using
 * the radix declared in meta["annIn.stream.textRadix"].
 */
typedef struct {
	uint64_t    start;
	uint64_t    end;
	const char *decoder;
	const char *klass;
	const char *text;
	int32_t     severity;
	int32_t     has_value;
	uint64_t    value;
} scopy_decoder_annotation_t;

typedef void (*scopy_decoder_ann_cb)(const scopy_decoder_annotation_t *ann, void *user);

/*
 * One-shot samples-in decode.
 *
 * cfg_json:  JSON config (mirrors DecoderConfig: decoderId, sampleRate,
 *            numChannels, channels[], options{}, meta{}).
 * data:      packed samples; unitsize = ceil(numChannels/8), channel i
 *            in bit i, LSB-first within each byte.
 * n_samples: sample count (not bytes).
 * cb/user:   optional per-annotation callback (may be NULL).
 * err_buf:   optional NUL-terminated error message on failure.
 *
 * Returns 0 on success, non-zero on hard error.
 */
int scopy_decoder_decode(const char *cfg_json,
                         const uint8_t *data, size_t n_samples,
                         scopy_decoder_ann_cb cb, void *user,
                         char *err_buf, size_t err_buf_size);

/*
 * Return non-zero if scopy_decoder_decode_ann() is implemented.
 * A missing symbol is treated as 0 (samples-in only).
 */
int scopy_decoder_accepts_annotations(void);

/*
 * One-shot annotation-in decode (chained decoders). Same JSON schema as
 * scopy_decoder_decode. Delivered sample indices are on the upstream
 * timeline. Input strings valid only for the duration of the call.
 *
 * meta carries the "annIn." section. The producer's own description of the
 * incoming stream — trust these over any guess:
 *   annIn.stream.producerId   canonical id of the upstream decoder ("spi")
 *   annIn.stream.textRadix    radix of `text` when has_value is 0:
 *                             "hex" | "dec" | "oct" | "bin" | "ascii"
 *   annIn.stream.samplerate   timeline start/end are expressed on, Hz
 *   annIn.stream.bitrate      upstream line rate, 0 if not applicable
 * Consumer-side keys, all optional:
 *   annIn.upstreamId          override the extractor selection
 *   annIn.radix               override annIn.stream.textRadix
 *   annIn.samplerate          for backends that regenerate a waveform: rate
 *   annIn.bitrate             of the *fabricated* signal, not the captured one
 *   annIn.frameformat         UART frame format, e.g. "8n1"
 *   annIn.direction           SPI: "mosi" | "miso" | "both"
 *   annIn.<name>Klass         comma-separated annotation-class whitelist
 *                             override for the named extraction rule
 */
int scopy_decoder_decode_ann(const char *cfg_json,
                             const scopy_decoder_annotation_t *in,
                             size_t n_in,
                             scopy_decoder_ann_cb cb, void *user,
                             char *err_buf, size_t err_buf_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SCOPY_DECODER_C_API_H */
