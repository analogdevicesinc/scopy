#pragma once

#include "core/result.h"

#include <QList>
#include <QObject>
#include <cstddef>

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
	Q_PROPERTY(QList<int> enabledChannels READ enabledChannels CONSTANT)
	Q_PROPERTY(quint64 samplesCount READ samplesCount CONSTANT)
public:
	explicit Stream(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~Stream() override = default;

	virtual Result<void> open(const StreamConfig &cfg) = 0;
	virtual void close() = 0;

	bool isOpen() const { return m_open; }
	const StreamConfig &config() const { return m_config; }
	QList<int> enabledChannels() const { return m_config.enabledChannels; }
	size_t samplesCount() const { return m_config.samplesCount; }

Q_SIGNALS:
	void streamError(const scopy::Error &error);
	void openedChanged(bool open);

protected:
	StreamConfig m_config;
	bool m_open = false;
};

} // namespace scopy::component
