#ifndef SCOPY_DECODER_C_API_H
#define SCOPY_DECODER_C_API_H

/*
 * Pure-C ABI shim for closed-source vendor decoder libraries.
 *
 * No Qt, no STL, no C++ types cross this boundary — only raw pointers and
 * lengths. Configuration is passed as a JSON string for forward compatibility;
 * keys mirror scopy::decoder::DecoderConfig.
 *
 * A vendor library implementing this ABI is loaded via QLibrary by
 * scopy::decoder::DynamicLibBackend and adapted to IDecoderBackend.
 *
 * Threading / lifecycle: scopy_decoder_decode() is fully self-contained.
 * No handles, no state. Every call decodes exactly one buffer; vendor
 * implementations spin up and tear down any internal resources within
 * a single invocation. Sample indices in delivered annotations are
 * buffer-local (0 .. n_samples-1).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * POD annotation record passed to user callbacks. All const char* pointers
 * are owned by the backend and remain valid only for the duration of the
 * callback invocation.
 */
typedef struct {
	uint64_t    start;
	uint64_t    end;
	const char *decoder;
	const char *klass;
	const char *text;
	int32_t     severity;
} scopy_decoder_annotation_t;

typedef void (*scopy_decoder_ann_cb)(const scopy_decoder_annotation_t *ann, void *user);

/*
 * One-shot decode entry point.
 *
 * cfg_json:  UTF-8 JSON configuration string. Keys mirror
 *            scopy::decoder::DecoderConfig (decoderId, sampleRate,
 *            numChannels, channels[], options{}, meta{}).
 * data:      packed samples, unitsize = ceil(numChannels / 8) bytes/sample,
 *            channel i = bit i, LSB-first within each byte.
 * n_samples: number of samples in 'data' (NOT bytes).
 * cb:        callback invoked once per produced annotation. May be NULL,
 *            in which case the backend still runs and reports errors but
 *            no annotations are surfaced.
 * user:      opaque pointer forwarded to 'cb'.
 * err_buf:   optional buffer for a human-readable error message on failure.
 *            May be NULL. The backend writes a NUL-terminated string of at
 *            most (err_buf_size - 1) bytes when returning a non-zero value.
 *
 * Returns 0 on success (including empty result), non-zero on hard error.
 */
int scopy_decoder_decode(const char *cfg_json,
                         const uint8_t *data, size_t n_samples,
                         scopy_decoder_ann_cb cb, void *user,
                         char *err_buf, size_t err_buf_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SCOPY_DECODER_C_API_H */
