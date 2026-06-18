#pragma once

#include <core/acq_engine/SourceBlock.h>
#include <core/acq_engine/DataStore.h>

#include <atomic>

struct iio_context;
struct iio_device;
struct iio_buffer;

namespace scopy {
namespace adc {
namespace sim {

// Source block that reads I/Q samples from a PlutoSDR via the
// cf-ad9361-lpc IIO buffer device.
//
// The buffer-capable RX device has two input channels:
//   voltage0  (I, index 0, format le:S12/16>>0)
//   voltage1  (Q, index 1, format le:S12/16>>0)
//
// Samples are written to DataStore as QVector<float> normalised to [-1, +1].
// Channel keys: DataKey::raw(id(), "voltage0") / DataKey::raw(id(), "voltage1")
//
// Typical usage:
//   auto *src = new PlutoIIOSource(ctx, "pluto", "cf-ad9361-lpc", parent);
//   src->enableChannel("voltage0", true);
//   src->enableChannel("voltage1", true);
//   engine->addSource(src);
//   engine->run();
class PlutoIIOSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	explicit PlutoIIOSource(iio_context *ctx,
				const QString &id      = "pluto",
				const QString &devName = "cf-ad9361-lpc",
				QObject       *parent  = nullptr);
	~PlutoIIOSource() override;

	// Called by the engine on the main thread before the worker starts.
	// Finds the device, enables channels, creates the IIO buffer.
	void onStart() override;

	// Called by the engine on the main thread when stopping.
	// Cancels and destroys the IIO buffer (unblocks any blocked acquire()).
	void onStop() override;

	// Called on the worker thread. Blocks until one full buffer is ready,
	// then writes each enabled channel to DataStore.
	void acquire(scopy::acq::DataStore *store) override;

private:
	iio_context *m_ctx;
	QString      m_devName;
	iio_device  *m_dev{nullptr};
	std::atomic<iio_buffer *> m_buf{nullptr};
};

} // namespace sim
} // namespace adc
} // namespace scopy
