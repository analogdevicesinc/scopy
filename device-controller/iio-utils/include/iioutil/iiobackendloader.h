#pragma once

#include "iioutil/handles.h"

#include <QMap>
#include <QMutex>

namespace scopy::iio {

class IBackend;

class IIOBackendLoader
{
public:
    static IIOBackendLoader *instance();

    IBackend *backend(LibiioVersion requested);

    bool isLoaded(LibiioVersion v) const;

private:
    IIOBackendLoader() = default;
    ~IIOBackendLoader();

    // Loads the plugin for a concrete version (V0/V1) and caches it. Returns the
    // backend, or nullptr on failure. Caller holds m_mutex.
    IBackend *ensureLoaded(LibiioVersion v);
    // Resolves Default to a concrete version, loading if needed. Caller holds m_mutex.
    IBackend *resolve(LibiioVersion requested);

    mutable QMutex m_mutex;
    QMap<LibiioVersion, void *> m_dlHandles;    // V0 / V1 dlopen handles
    QMap<LibiioVersion, IBackend *> m_backends; // V0 / V1 backend instances
};

} // namespace scopy::iio
