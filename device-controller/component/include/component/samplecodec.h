#pragma once

#include <QObject>

namespace scopy::component {

// Per-channel single-sample codec: pure computation, no executor. Converts one
// sample between hardware and host representation.
class SampleCodec : public QObject
{
	Q_OBJECT
public:
	explicit SampleCodec(QObject *parent = nullptr)
		: QObject(parent)
	{
	}
	~SampleCodec() override = default;

	virtual void convert(void *dst, const void *src) const = 0;
	virtual void convertInverse(void *dst, const void *src) const = 0;
};

} // namespace scopy::component
