#include "iioutil/iiobackendloader.h"
#include "iioutil/ibackend.h"
#include <QDebug>
#include <dlfcn.h>

namespace scopy::iio {

using CreateFn = IBackend *(*)();

static const char *soName(LibiioVersion v)
{
    switch(v) {
    case LibiioVersion::V1:
        return "libiio-backend-v1.so";
    case LibiioVersion::V0:
        return "libiio-backend-v0.so";
    default:
        return nullptr;
    }
}

IIOBackendLoader *IIOBackendLoader::instance()
{
    static IIOBackendLoader s_instance;
    return &s_instance;
}

IIOBackendLoader::~IIOBackendLoader()
{
    QMutexLocker locker(&m_mutex);
    for(IBackend *b : m_backends) {
        delete b;
    }
    for(void *h : m_dlHandles) {
        if(h) {
            dlclose(h);
        }
    }
    m_backends.clear();
    m_dlHandles.clear();
}

IBackend *IIOBackendLoader::ensureLoaded(LibiioVersion v)
{
    auto it = m_backends.constFind(v);
    if(it != m_backends.constEnd()) {
        return it.value();
    }

    const char *name = soName(v);
    if(!name) {
        return nullptr;
    }

    void *handle = dlopen(name, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    if(!handle) {
        qWarning() << "IIOBackendLoader: dlopen failed for" << name << ":" << dlerror();
        return nullptr;
    }

    dlerror(); // clear previous error
    auto *createFn = reinterpret_cast<CreateFn>(dlsym(handle, "createIIOBackend"));
    const char *err = dlerror();
    if(err) {
        qWarning() << "IIOBackendLoader: dlsym(createIIOBackend) failed:" << err;
        dlclose(handle);
        return nullptr;
    }

    IBackend *backend = createFn();
    if(!backend) {
        qWarning() << "IIOBackendLoader: createIIOBackend() returned nullptr";
        dlclose(handle);
        return nullptr;
    }

    m_dlHandles.insert(v, handle);
    m_backends.insert(v, backend);
    return backend;
}

IBackend *IIOBackendLoader::resolve(LibiioVersion requested)
{
    if(requested == LibiioVersion::V0 || requested == LibiioVersion::V1) {
        return ensureLoaded(requested);
    }

    // Default: prefer an already-loaded backend (V1 first), then try to load.
    if(auto it = m_backends.constFind(LibiioVersion::V1); it != m_backends.constEnd()) {
        return it.value();
    }
    if(auto it = m_backends.constFind(LibiioVersion::V0); it != m_backends.constEnd()) {
        return it.value();
    }
    if(IBackend *b = ensureLoaded(LibiioVersion::V1)) {
        return b;
    }
    if(IBackend *b = ensureLoaded(LibiioVersion::V0)) {
        return b;
    }
    qWarning() << "IIOBackendLoader: no libiio backend plugin found";
    return nullptr;
}

IBackend *IIOBackendLoader::backend(LibiioVersion requested)
{
    QMutexLocker locker(&m_mutex);
    return resolve(requested);
}

bool IIOBackendLoader::isLoaded(LibiioVersion v) const
{
    QMutexLocker locker(&m_mutex);
    return m_backends.contains(v);
}

} // namespace scopy::iio
