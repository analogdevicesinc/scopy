#pragma once

#include "core/result.h"

#include <QList>
#include <QObject>
#include <cstddef>
#include <qcorotask.h>

namespace scopy::component {

// Direction-neutral open() configuration. Channel selection has one source of
// truth (the scan elements); cyclic/kernelBuffers are leaf setters, not here.
struct StreamConfig
{
	QList<int> enabledChannels;
	size_t samplesCount = 4096;
};

// Streaming session + lifecycle. Direction-specific I/O lives on InputStream /
// OutputStream. open() takes the same neutral config in both directions.
class Stream : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool isOpen READ isOpen NOTIFY openedChanged)
	Q_PROPERTY(unsigned bufferIndex READ bufferIndex CONSTANT)
	Q_PROPERTY(QList<int> enabledChannels READ enabledChannels CONSTANT)
	Q_PROPERTY(quint64 samplesCount READ samplesCount CONSTANT)
public:
	explicit Stream(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~Stream() override = default;

	virtual QCoro::Task<CommandResponse<void>> openAsync(const StreamConfig &cfg) = 0;
	virtual QCoro::Task<CommandResponse<void>> closeAsync() = 0;

	bool isOpen() const { return m_open; }
	// Which device buffer this stream drives. Distinguishes sibling streams of the
	// same direction on a multi-buffer device (libiio v1). 0 for single-buffer.
	unsigned bufferIndex() const { return m_bufferIndex; }
	const StreamConfig &config() const { return m_config; }
	QList<int> enabledChannels() const { return m_config.enabledChannels; }
	size_t samplesCount() const { return m_config.samplesCount; }

Q_SIGNALS:
	void streamError(const scopy::Error &error);
	void openedChanged(bool open);

protected:
	StreamConfig m_config;
	bool m_open = false;
	unsigned m_bufferIndex = 0;
};

// Discovery helpers. A device may hold several same-direction streams (one per
// device buffer on libiio v1), so callers must not assume uniqueness: enumerate
// with streamsOf<T>() and disambiguate by bufferIndex, or fetch one directly with
// streamAt<T>(). T is InputStream / OutputStream (direction) or a concrete leaf.
template <typename T> QList<T *> streamsOf(const QObject *device)
{
	return device ? device->findChildren<T *>(QString(), Qt::FindDirectChildrenOnly) : QList<T *>{};
}

template <typename T> T *streamAt(const QObject *device, unsigned bufferIndex = 0)
{
	for(T *s : streamsOf<T>(device)) {
		if(s->bufferIndex() == bufferIndex) {
			return s;
		}
	}
	return nullptr;
}

} // namespace scopy::component
