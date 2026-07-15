#pragma once

#include <QString>
#include <QVector>

namespace scopy::iio {

struct ContextHandle {
	void *ptr = nullptr;
};
struct DeviceHandle {
	void *ptr = nullptr;
};
struct ChannelHandle {
	void *ptr = nullptr;
};
struct AttrHandle {
	void *ptr = nullptr;
};
struct BufferHandle {
	void *ptr = nullptr;
};
struct BlockHandle {
	void *ptr = nullptr;
};
struct EventStreamHandle {
	void *ptr = nullptr;
};
struct ScanHandle {
	void *ptr = nullptr;
};
struct ChannelsMaskHandle {
	void *ptr = nullptr;
};

enum class LibiioVersion { V0, V1, Default };

struct ContextParams {
    int timeoutMs = 0; // 0 = backend default; -1 = infinite; INT_MIN = non-blocking
    // Additional params (out, err, logLevel, stderrLevel, timestampLevel, flags)
    // can be added here when needed — map to iio_context_params fields in v1contextops.cpp;
    // v0 ignores anything beyond timeoutMs.
};

struct ScanResult {
	QString uri;
	QString description;
};

} // namespace scopy::iio
