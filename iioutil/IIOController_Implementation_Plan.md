# IIOController Implementation Plan

## Context

Currently, plugins access libiio directly through `ConnectionProvider` → `Connection` → raw `iio_context*`, then operate on raw `iio_device*`, `iio_channel*`, `iio_buffer*` structs. This creates:
- No centralized resource tracking (buffer conflicts between plugins)
- Tight coupling to libiio v0 API throughout the codebase
- No controlled access — any plugin can do anything with the raw context

The goal is to centralize all libiio operations behind a clean architecture that:
1. Hides raw libiio structs from plugins (operations via string-based identifiers)
2. Uses an interface pattern for libiio v0/v1 (not preprocessor guards)
3. Splits responsibilities across focused classes (not one monolithic controller)
4. Creates new classes — does NOT reuse `ConnectionProvider` or `Connection`

---

## Architecture Overview

```
                        ┌─────────────────┐
                        │  IIODeviceImpl   │  (owns IIOController, 1 per device)
                        └────────┬────────┘
                                 │
                        ┌────────▼────────┐
                        │  IIOController   │  (coordination: resource tracking,
                        │                  │   CommandQueue, ping, StatusBar)
                        └────────┬────────┘
                                 │ uses
                  ┌──────────────▼──────────────┐
                  │     IIOBackend (interface)    │  (pure libiio abstraction)
                  └──────────────┬──────────────┘
                        ┌────────┴────────┐
                   ┌────▼────┐      ┌─────▼────┐
                   │IIOv0    │      │IIOv1     │
                   │Backend  │      │Backend   │
                   └─────────┘      └──────────┘

Plugin → device->iioController() → controller->readDeviceAttr("ad9361", "sampling_freq")
                                 → controller->createBuffer("cf-ad9361-lpc", ...)
```

Plugins **never** touch `iio_device*`, `iio_channel*`, `iio_buffer*` directly. All operations go through IIOController methods using string identifiers (device name, channel name, attribute name). IIOController can provide raw struct getters as escape hatches during migration, but the best practice is through methods.

---

## Class 1: IIOBackend (Interface)

**Files:** `iioutil/include/iioutil/iiobackend.h`

Pure abstract interface defining all libiio operations. Two implementations: `IIOv0Backend` and `IIOv1Backend`.

```cpp
class IIOBackend {
public:
    virtual ~IIOBackend() = default;

    // --- Context ---
    virtual bool openContext(const QString &uri) = 0;
    virtual void closeContext() = 0;
    virtual bool isOpen() const = 0;
    virtual bool ping() = 0;

    // --- Context info ---
    virtual QString contextDescription() const = 0;
    virtual int contextAttrCount() const = 0;
    virtual QPair<QString, QString> contextAttr(int index) const = 0;

    // --- Device enumeration ---
    virtual int deviceCount() const = 0;
    virtual QString deviceName(int index) const = 0;
    virtual QString deviceId(int index) const = 0;

    // --- Device attributes ---
    virtual int deviceAttrCount(const QString &deviceName) const = 0;
    virtual QString deviceAttrName(const QString &deviceName, int index) const = 0;
    virtual QPair<QString, ssize_t> readDeviceAttr(const QString &deviceName,
                                                    const QString &attrName) const = 0;
    virtual ssize_t writeDeviceAttr(const QString &deviceName,
                                     const QString &attrName,
                                     const QString &value) = 0;

    // --- Channel enumeration ---
    virtual int channelCount(const QString &deviceName) const = 0;
    virtual QString channelName(const QString &deviceName, int index) const = 0;
    virtual QString channelId(const QString &deviceName, int index) const = 0;
    virtual bool isChannelOutput(const QString &deviceName, const QString &channelId) const = 0;

    // --- Channel attributes ---
    virtual int channelAttrCount(const QString &deviceName,
                                  const QString &channelId, bool output) const = 0;
    virtual QString channelAttrName(const QString &deviceName,
                                     const QString &channelId, bool output,
                                     int index) const = 0;
    virtual QPair<QString, ssize_t> readChannelAttr(const QString &deviceName,
                                                     const QString &channelId,
                                                     bool output,
                                                     const QString &attrName) const = 0;
    virtual ssize_t writeChannelAttr(const QString &deviceName,
                                      const QString &channelId,
                                      bool output,
                                      const QString &attrName,
                                      const QString &value) = 0;

    // --- Channel enable/disable ---
    virtual void enableChannel(const QString &deviceName,
                                const QString &channelId, bool output,
                                bool enable) = 0;

    // --- Buffer operations ---
    // Returns an opaque handle (int) instead of raw iio_buffer*
    virtual int createBuffer(const QString &deviceName,
                              size_t samplesCount, bool cyclic) = 0;
    virtual void destroyBuffer(int bufferHandle) = 0;
    virtual ssize_t refillBuffer(int bufferHandle) = 0;
    virtual ssize_t pushBuffer(int bufferHandle) = 0;
    virtual void cancelBuffer(int bufferHandle) = 0;

    // --- Register operations ---
    virtual QPair<uint32_t, ssize_t> readRegister(const QString &deviceName,
                                                    uint32_t address) const = 0;
    virtual ssize_t writeRegister(const QString &deviceName,
                                   uint32_t address, uint32_t value) = 0;

    // --- Trigger ---
    virtual QString getTrigger(const QString &deviceName) const = 0;
    virtual ssize_t setTrigger(const QString &deviceName,
                                const QString &triggerName) = 0;

    // --- Raw access (escape hatch for migration) ---
    virtual void *rawContext() const = 0;
    virtual void *rawDevice(const QString &deviceName) const = 0;
    virtual void *rawChannel(const QString &deviceName,
                              const QString &channelId, bool output) const = 0;
};
```

**Key design decisions:**
- All operations use string identifiers (device name, channel id, attr name)
- Buffer operations return opaque `int` handles, not `iio_buffer*`
- `rawContext()` / `rawDevice()` / `rawChannel()` are escape hatches returning `void*` — plugins that absolutely need raw pointers can cast, but this is discouraged
- Return types use `QPair<value, errorCode>` for operations that read data

---

## Class 2: IIOv0Backend

**Files:** `iioutil/include/iioutil/iiov0backend.h`, `iioutil/src/iiov0backend.cpp`

Implements `IIOBackend` using libiio v0 API calls.

```cpp
class IIOv0Backend : public IIOBackend {
public:
    bool openContext(const QString &uri) override {
        m_ctx = iio_create_context_from_uri(uri.toStdString().c_str());
        return m_ctx != nullptr;
    }

    QPair<QString, ssize_t> readDeviceAttr(const QString &deviceName,
                                            const QString &attrName) const override {
        iio_device *dev = iio_context_find_device(m_ctx, deviceName.toStdString().c_str());
        if (!dev) return {"", -ENODEV};
        char buf[1024];
        ssize_t ret = iio_device_attr_read(dev, attrName.toStdString().c_str(), buf, sizeof(buf));
        return {ret > 0 ? QString(buf) : "", ret};
    }

    int createBuffer(const QString &deviceName, size_t samplesCount, bool cyclic) override {
        iio_device *dev = iio_context_find_device(m_ctx, deviceName.toStdString().c_str());
        if (!dev) return -1;
        iio_buffer *buf = iio_device_create_buffer(dev, samplesCount, cyclic);
        if (!buf) return -1;
        int handle = m_nextHandle++;
        m_buffers[handle] = buf;
        return handle;
    }

    void enableChannel(const QString &deviceName, const QString &channelId,
                        bool output, bool enable) override {
        iio_device *dev = iio_context_find_device(m_ctx, deviceName.toStdString().c_str());
        iio_channel *chn = iio_device_find_channel(dev, channelId.toStdString().c_str(), output);
        if (enable) iio_channel_enable(chn);
        else iio_channel_disable(chn);
    }

    bool ping() override {
        unsigned int major, minor;
        char tag[32];
        int ret = iio_context_get_version(m_ctx, &major, &minor, tag);
        return ret == 0;
    }
    // ... remaining methods follow same pattern

private:
    iio_context *m_ctx = nullptr;
    QMap<int, iio_buffer*> m_buffers;
    int m_nextHandle = 1;
};
```

---

## Class 3: IIOv1Backend

**Files:** `iioutil/include/iioutil/iiov1backend.h`, `iioutil/src/iiov1backend.cpp`

Implements `IIOBackend` using libiio v1 API calls.

```cpp
class IIOv1Backend : public IIOBackend {
public:
    bool openContext(const QString &uri) override {
        struct iio_context_params *params = iio_context_params_create();
        m_ctx = iio_create_context(params, uri.toStdString().c_str());
        iio_context_params_destroy(params);
        return !iio_err(m_ctx);
    }

    QPair<QString, ssize_t> readDeviceAttr(const QString &deviceName,
                                            const QString &attrName) const override {
        const iio_device *dev = iio_context_find_device(m_ctx, deviceName.toStdString().c_str());
        if (!dev) return {"", -ENODEV};
        const iio_attr *attr = iio_device_find_attr(dev, attrName.toStdString().c_str());
        if (!attr) return {"", -ENOENT};
        char buf[1024];
        ssize_t ret = iio_attr_read_raw(attr, buf, sizeof(buf));
        return {ret > 0 ? QString(buf) : "", ret};
    }

    int createBuffer(const QString &deviceName, size_t samplesCount, bool cyclic) override {
        const iio_device *dev = iio_context_find_device(m_ctx, deviceName.toStdString().c_str());
        if (!dev) return -1;
        struct iio_channels_mask *mask = iio_create_channels_mask(iio_device_get_channels_count(dev));
        // Apply stored channel enable states to the mask
        struct iio_buffer *buf = iio_device_create_buffer(dev, 0, mask);
        if (iio_err(buf)) { iio_channels_mask_destroy(mask); return -1; }
        int handle = m_nextHandle++;
        m_buffers[handle] = buf;
        m_masks[handle] = mask;
        return handle;
    }

    void enableChannel(const QString &deviceName, const QString &channelId,
                        bool output, bool enable) override {
        // v1: channels are enabled on a mask, not directly
        // Store enable state, apply when buffer is created
        ChannelKey key{deviceName, channelId, output};
        m_channelStates[key] = enable;
    }

    bool ping() override {
        return iio_context_get_attrs_count(m_ctx) >= 0;
    }
    // ... remaining methods

private:
    iio_context *m_ctx = nullptr;
    QMap<int, iio_buffer*> m_buffers;
    QMap<int, iio_channels_mask*> m_masks;
    int m_nextHandle = 1;
    struct ChannelKey { QString dev; QString ch; bool output; };
    QMap<ChannelKey, bool> m_channelStates;
};
```

**Key v0 → v1 differences handled:**
| Operation | v0 | v1 |
|-----------|----|----|
| Context creation | `iio_create_context_from_uri(uri)` | `iio_create_context(params, uri)` |
| Attribute read | `iio_device_attr_read(dev, name, buf, len)` | `iio_attr_read_raw(iio_device_find_attr(dev, name), buf, len)` |
| Channel enable | `iio_channel_enable(chn)` | `iio_channel_enable(chn, mask)` — mask-based |
| Buffer create | `iio_device_create_buffer(dev, samples, cyclic)` | `iio_device_create_buffer(dev, idx, mask)` + blocks/streams |
| Error handling | `NULL` + `errno` | Pointer-encoded errors via `iio_err()` / `iio_ptr()` |
| Ping | `iio_context_get_version(ctx, ...)` | `iio_context_get_attrs_count(ctx)` |

---

## Class 4: IIOController

**Files:** `iioutil/include/iioutil/iiocontroller.h`, `iioutil/src/iiocontroller.cpp`

Coordination layer that owns an `IIOBackend` and adds resource management + CommandQueue integration. This is the class plugins interact with. It does NOT contain libiio calls — all libiio goes through the backend.

```cpp
class IIOController : public QObject {
    Q_OBJECT
public:
    explicit IIOController(const QString &uri, QObject *parent = nullptr);
    ~IIOController();

    // --- Context lifecycle ---
    bool openContext();
    void closeContext();
    bool isOpen() const;
    QString uri() const;

    // --- Backend access ---
    IIOBackend *backend() const;

    // --- CommandQueue control ---
    CommandQueue *commandQueue() const;
    void setUseCommandQueue(bool enable);  // Global toggle
    bool useCommandQueue() const;

    // --- Device enumeration (delegates to backend) ---
    int deviceCount() const;
    QString deviceName(int index) const;
    QString deviceId(int index) const;

    // --- Attribute operations ---
    // throughQueue parameter: overrides global setting per-call
    // - true: enqueue on CommandQueue (async, thread-safe)
    // - false: execute directly on calling thread (sync)
    // If not specified, uses the global useCommandQueue() setting
    QPair<QString, ssize_t> readDeviceAttr(const QString &deviceName,
                                            const QString &attrName,
                                            bool throughQueue = false) const;
    ssize_t writeDeviceAttr(const QString &deviceName,
                             const QString &attrName, const QString &value,
                             bool throughQueue = false);
    QPair<QString, ssize_t> readChannelAttr(const QString &deviceName,
                                             const QString &channelId,
                                             bool output,
                                             const QString &attrName,
                                             bool throughQueue = false) const;
    ssize_t writeChannelAttr(const QString &deviceName,
                              const QString &channelId, bool output,
                              const QString &attrName, const QString &value,
                              bool throughQueue = false);

    // --- Channel enable/disable ---
    void enableChannel(const QString &deviceName, const QString &channelId,
                        bool output, bool enable);

    // --- Buffer operations WITH resource tracking ---
    int createBuffer(const QString &deviceName, size_t samplesCount,
                      bool cyclic, const QString &owner,
                      bool throughQueue = false);
    void destroyBuffer(int bufferHandle, const QString &owner,
                        bool throughQueue = false);
    ssize_t refillBuffer(int bufferHandle, bool throughQueue = false);
    ssize_t pushBuffer(int bufferHandle, bool throughQueue = false);
    void cancelBuffer(int bufferHandle);
    bool isBufferInUse(const QString &deviceName) const;
    QString bufferOwner(const QString &deviceName) const;

    // --- Register operations ---
    QPair<uint32_t, ssize_t> readRegister(const QString &deviceName,
                                           uint32_t address,
                                           bool throughQueue = false) const;
    ssize_t writeRegister(const QString &deviceName,
                           uint32_t address, uint32_t value,
                           bool throughQueue = false);

    // --- Trigger operations ---
    QString getTrigger(const QString &deviceName) const;
    ssize_t setTrigger(const QString &deviceName, const QString &triggerName);

    // --- Ping ---
    bool ping();

    // --- Raw access (escape hatch, discouraged) ---
    void *rawContext() const;
    void *rawDevice(const QString &deviceName) const;

Q_SIGNALS:
    void contextOpened();
    void contextClosed();
    void contextError(const QString &error);
    void bufferCreated(const QString &deviceName, const QString &owner);
    void bufferDestroyed(const QString &deviceName);

private:
    QString m_uri;
    IIOBackend *m_backend = nullptr;
    CommandQueue *m_cmdQueue = nullptr;
    bool m_useCommandQueue = false;

    // Buffer tracking: deviceName → (bufferHandle, owner)
    struct BufferInfo {
        int handle;
        QString owner;
    };
    QMap<QString, BufferInfo> m_buffers;
};
```

### CommandQueue Integration

The developer controls whether operations go through the `CommandQueue` at two levels:

**1. Global toggle:**
```cpp
ctrl->setUseCommandQueue(true);  // All subsequent calls go through queue
ctrl->readDeviceAttr("dev", "attr");  // → queued
```

**2. Per-call override via `throughQueue` parameter:**
```cpp
ctrl->setUseCommandQueue(false);  // Global: direct
ctrl->readDeviceAttr("dev", "attr");  // → direct (sync)
ctrl->readDeviceAttr("dev", "attr", true);  // → queued (override)
```

**Implementation pattern for queued operations:**
```cpp
QPair<QString, ssize_t> IIOController::readDeviceAttr(const QString &deviceName,
                                                       const QString &attrName,
                                                       bool throughQueue) const {
    bool useQueue = throughQueue || m_useCommandQueue;

    if (useQueue) {
        // Create a Command-derived object that wraps backend->readDeviceAttr()
        auto *cmd = new IIOBackendCommand([this, deviceName, attrName]() {
            return m_backend->readDeviceAttr(deviceName, attrName);
        });
        m_cmdQueue->enqueue(cmd);
        // For sync-with-queue: wait for completion and return result
        // For async: connect to cmd->finished signal
        cmd->wait();
        return cmd->result();
    }

    // Direct call — synchronous on calling thread
    return m_backend->readDeviceAttr(deviceName, attrName);
}
```

**New helper: IIOBackendCommand**

A generic `Command` subclass that wraps any backend call as a lambda:

```cpp
// iioutil/include/iioutil/iiobackendcommand.h
template<typename ResultType>
class IIOBackendCommand : public Command {
public:
    IIOBackendCommand(std::function<ResultType()> operation, QObject *parent = nullptr)
        : m_operation(operation) {
        this->setParent(parent);
        m_cmdResult = new CommandResult();
    }

    void execute() override {
        Q_EMIT started(this);
        m_result = m_operation();
        m_cmdResult->errorCode = 0;
        Q_EMIT finished(this);
    }

    ResultType result() const { return m_result; }

private:
    std::function<ResultType()> m_operation;
    ResultType m_result;
};
```

This reuses the existing `Command` base class and `CommandQueue` infrastructure while wrapping backend operations generically.

### Buffer Conflict Example

```cpp
int IIOController::createBuffer(const QString &deviceName, size_t samplesCount,
                                 bool cyclic, const QString &owner,
                                 bool throughQueue) {
    // Resource tracking always happens in IIOController (not in queue)
    if (m_buffers.contains(deviceName)) {
        QString existing = m_buffers[deviceName].owner;
        StatusBarManager::pushMessage(
            QString("Buffer on %1 already in use by %2").arg(deviceName).arg(existing));
        Q_EMIT contextError(QString("Buffer conflict on %1").arg(deviceName));
        return -1;
    }

    // The actual backend call can go through queue or not
    int handle;
    if (throughQueue || m_useCommandQueue) {
        auto *cmd = new IIOBackendCommand<int>([this, deviceName, samplesCount, cyclic]() {
            return m_backend->createBuffer(deviceName, samplesCount, cyclic);
        });
        m_cmdQueue->enqueue(cmd);
        cmd->wait();
        handle = cmd->result();
    } else {
        handle = m_backend->createBuffer(deviceName, samplesCount, cyclic);
    }

    if (handle < 0) return -1;
    m_buffers[deviceName] = {handle, owner};
    Q_EMIT bufferCreated(deviceName, owner);
    return handle;
}
```

**Responsibilities (focused):**
1. Own and manage `IIOBackend` lifetime
2. Delegate all libiio operations to `IIOBackend`
3. Track buffer ownership — prevent conflicts
4. Own `CommandQueue`, let developer control queue usage per-call or globally
5. Notify via `StatusBarManager` when resource conflicts occur
6. Provide ping via backend

**What IIOController does NOT do:**
- No libiio calls directly (all through backend)
- No context creation logic (backend handles it)
- No v0/v1 API differences (backend handles it)
- No exclusive device access (not needed — no current use case in codebase)

---

## Multiple Devices with Same URI

**Finding:** `DeviceManager::createDevice()` (`core/src/devicemanager.cpp:68-72`) already prevents creating two devices with the same URI:
```cpp
for(Device *val : qAsConst(map)) {
    if(val->param() == param) {
        return "";  // Reject duplicate
    }
}
```

**Approach:** Since duplicate URIs are already blocked at DeviceManager level, each `IIOController` gets its own unique URI and creates its own `iio_context`. No shared context management needed. If this restriction changes in the future, `IIOController` could be extended with a shared backend pool (similar to how `ConnectionProvider` uses ref-counting), but this is not needed now.

---

## Integration: IIODeviceImpl

**Files to modify:** `core/include/core/iiodeviceimpl.h`, `core/src/iiodeviceimpl.cpp`

`IIODeviceImpl` owns the `IIOController` (one per device/URI). Plugins access it via `device->iioController()`.

```cpp
// iiodeviceimpl.h - add:
class IIOController;

class IIODeviceImpl : public DeviceImpl {
public:
    // ...existing...
    IIOController *iioController() const;

private:
    IIOController *m_iioController = nullptr;
};
```

```cpp
// iiodeviceimpl.cpp - modify init():
void IIODeviceImpl::init() {
    m_iioController = new IIOController(m_param, this);
    DeviceImpl::init();
}
```

### Ping Migration

Ping moves from plugin-level to `IIODeviceImpl` using `IIOController::ping()`:

```cpp
void IIODeviceImpl::setupPing() {
    m_pingTask = new LambdaPingTask([this]() {
        return m_iioController->ping();
    });
    m_cyclicalTask = new CyclicalTask(m_pingTask, this);
    m_cyclicalTask->start(2000);
}
```

Plugins no longer need to implement `pingTask()`.

---

## Plugin Access Example (ADC Plugin)

**Current pattern** (to be replaced):
```cpp
bool ADCPlugin::onConnect() {
    Connection *conn = ConnectionProvider::GetInstance()->open(m_param);
    iio_context *ctx = conn->context();
    iio_device *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");
    iio_channel *ch = iio_device_find_channel(dev, "voltage0", false);
    iio_channel_enable(ch);
    m_buffer = iio_device_create_buffer(dev, 1024, false);
    // ...
}
```

**New pattern:**
```cpp
bool ADCPlugin::onConnect() {
    IIOController *ctrl = dynamic_cast<IIODeviceImpl*>(m_device)->iioController();

    // All operations via string identifiers — no raw iio structs
    ctrl->enableChannel("cf-ad9361-lpc", "voltage0", false, true);

    // Create buffer with ownership tracking, through CommandQueue
    int bufHandle = ctrl->createBuffer("cf-ad9361-lpc", 1024, false, "ADCPlugin", true);
    if (bufHandle < 0) return false;  // conflict reported via StatusBar
    m_bufferHandle = bufHandle;

    // Read attribute directly (no queue needed for one-shot reads)
    auto [sampleRate, ret] = ctrl->readDeviceAttr("cf-ad9361-lpc", "sampling_frequency");
    if (ret > 0) m_sampleRate = sampleRate.toDouble();

    return true;
}
```

**Note:** Full plugin porting is out of scope. This is a reference example only.

---

## Backend Selection

The correct backend (v0 or v1) is selected at compile time:

```cpp
IIOController::IIOController(const QString &uri, QObject *parent)
    : QObject(parent), m_uri(uri)
{
#if LIBIIO_VERSION_MAJOR >= 1
    m_backend = new IIOv1Backend();
#else
    m_backend = new IIOv0Backend();
#endif
    m_cmdQueue = new CommandQueue(this);
}
```

This is the only preprocessor guard. Each backend implementation is compiled conditionally (only one per build).

---

## New Files Summary

| File | Class | Description |
|------|-------|-------------|
| `iioutil/include/iioutil/iiobackend.h` | `IIOBackend` | Pure abstract interface for libiio operations |
| `iioutil/include/iioutil/iiov0backend.h` | `IIOv0Backend` | libiio v0 implementation |
| `iioutil/src/iiov0backend.cpp` | | |
| `iioutil/include/iioutil/iiov1backend.h` | `IIOv1Backend` | libiio v1 implementation |
| `iioutil/src/iiov1backend.cpp` | | |
| `iioutil/include/iioutil/iiocontroller.h` | `IIOController` | Coordination: resource tracking, CommandQueue, ping |
| `iioutil/src/iiocontroller.cpp` | | |
| `iioutil/include/iioutil/iiobackendcommand.h` | `IIOBackendCommand<T>` | Generic Command wrapper for backend calls |

## Modified Files

| File | Change |
|------|--------|
| `core/include/core/iiodeviceimpl.h` | Add `IIOController*` member + accessor |
| `core/src/iiodeviceimpl.cpp` | Create IIOController in `init()`, setup ping |
| `iioutil/CMakeLists.txt` | Add new files, conditional compilation for v0/v1 |

---

## Implementation Order

1. **IIOBackend interface** — Define all method signatures
2. **IIOv0Backend** — Implement using current libiio v0 API
3. **IIOBackendCommand** — Generic Command wrapper for CommandQueue integration
4. **IIOController** — Coordination layer with resource tracking + CommandQueue control
5. **IIODeviceImpl integration** — Own IIOController, expose to plugins, migrate ping
6. **IIOv1Backend** — Implement using libiio v1 API (can be done later, same interface)
7. **One plugin example** — Migrate ADC plugin as reference

---

## Verification

1. **Unit test**: Create IIOController with IIOv0Backend against `iio-emu adalm2000`, verify context opens, device/channel enumeration, attribute read/write
2. **Buffer tracking test**: Two createBuffer calls on same device → verify second fails with StatusBar message
3. **CommandQueue test**: Same operation with `throughQueue=true` and `throughQueue=false` → verify both produce correct results
4. **Ping test**: Verify IIOController::ping() returns true with connected device
5. **Integration test**: Modify one plugin (ADC) to use IIOController, verify it connects and functions normally
6. **Build test**: Verify `iioutil` CMakeLists compiles with new files
