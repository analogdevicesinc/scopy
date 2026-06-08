#include "PlutoIIOSource.h"
#include "DataKey.h"

#include <iio.h>
#include <stdexcept>

namespace scopy {
namespace adc {
namespace sim {

static constexpr float FULL_SCALE = 2048.0f;

PlutoIIOSource::PlutoIIOSource(iio_context *ctx, const QString &id,
			       const QString &devName, QObject *parent)
	: SourceBlock(id, parent)
	, m_ctx(ctx)
	, m_devName(devName)
{}

PlutoIIOSource::~PlutoIIOSource()
{
	onStop();
}

void PlutoIIOSource::onStart()
{
	SourceBlock::onStart();

	m_dev = iio_context_find_device(m_ctx, m_devName.toLocal8Bit().constData());
	if(!m_dev)
		throw std::runtime_error(
			QString("Device '%1' not found").arg(m_devName).toStdString());

	for(auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
		if(!it.value())
			continue;
		iio_channel *ch = iio_device_find_channel(
			m_dev, it.key().toLocal8Bit().constData(), false);
		if(ch)
			iio_channel_enable(ch);
	}

	iio_buffer *buf = iio_device_create_buffer(m_dev, m_bufferSize, false);
	m_buf.store(buf);
	if(!buf)
		throw std::runtime_error(
			QString("Failed to create IIO buffer (size %1)").arg(m_bufferSize).toStdString());
}

void PlutoIIOSource::onStop()
{
	m_stopRequested = true;

	iio_buffer *buf = m_buf.exchange(nullptr);
	if(buf) {
		iio_buffer_cancel(buf);
		iio_buffer_destroy(buf);
	}

	if(m_dev) {
		for(auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
			iio_channel *ch = iio_device_find_channel(
				m_dev, it.key().toLocal8Bit().constData(), false);
			if(ch)
				iio_channel_disable(ch);
		}
		m_dev = nullptr;
	}
}

void PlutoIIOSource::acquire(DataStore *store)
{
	iio_buffer *buf = m_buf.load();
	if(!buf || m_stopRequested)
		return;

	const ssize_t ret = iio_buffer_refill(buf);
	if(ret < 0) {
		if(m_stopRequested)
			return;
		throw std::runtime_error(
			QString("iio_buffer_refill error %1").arg(ret).toStdString());
	}

	const ptrdiff_t step = iio_buffer_step(buf);

	for(auto it = m_channels.cbegin(); it != m_channels.cend(); ++it) {
		if(!it.value())
			continue;

		iio_channel *ch = iio_device_find_channel(
			m_dev, it.key().toLocal8Bit().constData(), false);
		if(!ch)
			continue;

		const char *p   = static_cast<const char *>(iio_buffer_first(buf, ch));
		const char *end = static_cast<const char *>(iio_buffer_end(buf));

		QVector<float> samples;
		samples.reserve(static_cast<int>(m_bufferSize));

		for(; p < end; p += step) {
			const int16_t raw = *reinterpret_cast<const int16_t *>(p);
			samples.append(static_cast<float>(raw) / FULL_SCALE);
		}

		store->write(DataKey::raw(m_id, it.key()), std::move(samples));
	}
}

} // namespace sim
} // namespace adc
} // namespace scopy
