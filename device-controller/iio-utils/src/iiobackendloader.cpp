#include "iioutil/iiobackendloader.h"
#include "iioutil/ibackend.h"
#include <QDebug>
#include <dlfcn.h>

namespace scopy::iio {

using CreateFn = IBackend *(*)();

IIOBackendLoader *IIOBackendLoader::instance()
{
    static IIOBackendLoader s_instance;
    return &s_instance;
}

IIOBackendLoader::~IIOBackendLoader() { unload(); }

bool IIOBackendLoader::loadPlugin(const char *soName)
{
    void *handle = dlopen(soName, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    if(!handle) {
        qWarning() << "IIOBackendLoader: dlopen failed for" << soName << ":" << dlerror();
        return false;
    }

    dlerror(); // clear previous error
    auto *createFn = reinterpret_cast<CreateFn>(dlsym(handle, "createIIOBackend"));
    const char *err = dlerror();
    if(err) {
        qWarning() << "IIOBackendLoader: dlsym(createIIOBackend) failed:" << err;
        dlclose(handle);
        return false;
    }

    IBackend *backend = createFn();
    if(!backend) {
        qWarning() << "IIOBackendLoader: createIIOBackend() returned nullptr";
        dlclose(handle);
        return false;
    }

    m_dlHandle = handle;
    m_backend = backend;
    return true;
}

bool IIOBackendLoader::load(LibiioVersion v)
{
    if(m_backend && m_version == v) {
        return true;
    }

    if(m_backend) {
        unload();
    }

    if(v == LibiioVersion::V1) {
        if(loadPlugin("libiio-backend-v1.so")) {
            m_version = LibiioVersion::V1;
            return true;
        }
        return false;
    }

    if(v == LibiioVersion::V0) {
        if(loadPlugin("libiio-backend-v0.so")) {
            m_version = LibiioVersion::V0;
            return true;
        }
        return false;
    }

    // LibiioVersion::Default: try V1, fall back to V0
    if(loadPlugin("libiio-backend-v1.so")) {
        m_version = LibiioVersion::V1;
        return true;
    }
    if(loadPlugin("libiio-backend-v0.so")) {
        m_version = LibiioVersion::V0;
        return true;
    }

    qWarning() << "IIOBackendLoader: no libiio backend plugin found";
    return false;
}

void IIOBackendLoader::unload()
{
    delete m_backend;
    m_backend = nullptr;

    if(m_dlHandle) {
        dlclose(m_dlHandle);
        m_dlHandle = nullptr;
    }

    m_version = LibiioVersion::Default;
}

bool IIOBackendLoader::isLoaded() const { return m_backend != nullptr; }

LibiioVersion IIOBackendLoader::loadedVersion() const { return m_version; }

IBackend *IIOBackendLoader::backend() const { return m_backend; }

} // namespace scopy::iio
