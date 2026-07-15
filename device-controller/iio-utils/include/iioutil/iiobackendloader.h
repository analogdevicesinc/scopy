#pragma once

#include "iioutil/handles.h"

namespace scopy::iio {

class IBackend;

class IIOBackendLoader
{
public:
    static IIOBackendLoader *instance();

    // Load the plugin for the given version.
    // LibiioVersion::Default → try V1, fall back to V0.
    // Returns true if a backend is available after this call.
    bool load(LibiioVersion v);

    // Unload the current plugin. Any IBackend* obtained via backend() becomes invalid.
    // Destroy all context/scan objects before calling this.
    void unload();

    bool isLoaded() const;
    LibiioVersion loadedVersion() const;

    // Non-null only after a successful load().
    IBackend *backend() const;

private:
    IIOBackendLoader() = default;
    ~IIOBackendLoader();

    bool loadPlugin(const char *soName);

    void *m_dlHandle = nullptr;
    IBackend *m_backend = nullptr;
    LibiioVersion m_version = LibiioVersion::Default;
};

} // namespace scopy::iio
