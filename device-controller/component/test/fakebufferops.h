#pragma once

#include "iioutil/ibufferops.h"
#include "iioutil/ichannelops.h"

#include <QByteArray>
#include <QSet>

namespace dctest {

// In-memory IChannelOps: each ChannelHandle.ptr is a FakeChannel*. Enable state
// is tracked in the ChannelsMaskHandle (a QSet<long>* of enabled indices).
struct FakeChannel
{
	long index = 0;
	QString name;
	bool output = false;
	bool scan = true;
	scopy::iio::DataFormat format{};
};

class FakeChannelOps : public scopy::iio::IChannelOps
{
public:
	static FakeChannel *ch(scopy::iio::ChannelHandle h) { return static_cast<FakeChannel *>(h.ptr); }
	static QSet<long> *set(scopy::iio::ChannelsMaskHandle m) { return static_cast<QSet<long> *>(m.ptr); }

	QString id(scopy::iio::ChannelHandle) const override { return {}; }
	QString name(scopy::iio::ChannelHandle h) const override { return ch(h)->name; }
	QString label(scopy::iio::ChannelHandle) const override { return {}; }
	bool isOutput(scopy::iio::ChannelHandle h) const override { return ch(h)->output; }
	bool isScanElement(scopy::iio::ChannelHandle h) const override { return ch(h)->scan; }
	long index(scopy::iio::ChannelHandle h) const override { return ch(h)->index; }
	int channelType(scopy::iio::ChannelHandle) const override { return 0; }
	int modifier(scopy::iio::ChannelHandle) const override { return 0; }
	unsigned int attrsCount(scopy::iio::ChannelHandle) const override { return 0; }
	QString attrName(scopy::iio::ChannelHandle, unsigned int) const override { return {}; }
	scopy::iio::DataFormat dataFormat(scopy::iio::ChannelHandle h) const override { return ch(h)->format; }

	void enable(scopy::iio::ChannelHandle h, scopy::iio::ChannelsMaskHandle m) override { set(m)->insert(ch(h)->index); }
	void disable(scopy::iio::ChannelHandle h, scopy::iio::ChannelsMaskHandle m) override { set(m)->remove(ch(h)->index); }
	bool isEnabled(scopy::iio::ChannelHandle h, scopy::iio::ChannelsMaskHandle m) const override
	{
		return set(m)->contains(ch(h)->index);
	}

	size_t read(scopy::iio::ChannelHandle, scopy::iio::BlockHandle, void *, size_t, bool) const override { return 0; }
	size_t write(scopy::iio::ChannelHandle, scopy::iio::BlockHandle, const void *, size_t, bool) override { return 0; }
	void convert(scopy::iio::ChannelHandle, void *, const void *) const override {}
	void convertInverse(scopy::iio::ChannelHandle, void *, const void *) const override {}
};

// In-memory IBufferOps: openBuffer allocates a QByteArray sized samplesCount *
// step. bufferFirst offsets by a per-channel byte offset stored on FakeChannel
// (via a parallel map). refill/push succeed and bump counters.
class FakeBufferOps : public scopy::iio::IBufferOps
{
public:
	QByteArray storage;
	ptrdiff_t step = 0;
	int refills = 0, pushes = 0, opens = 0, closes = 0, cancels = 0;
	int masksAlive = 0;
	QHash<void *, ptrdiff_t> chOffset; // FakeChannel* -> byte offset within a sample

	scopy::iio::BufferHandle openBuffer(scopy::iio::DeviceHandle, unsigned int, scopy::iio::ChannelsMaskHandle,
					    size_t samplesCount, bool, unsigned int) override
	{
		++opens;
		storage.resize(static_cast<int>(samplesCount) * static_cast<int>(step));
		return {this};
	}
	void closeBuffer(scopy::iio::BufferHandle) override { ++closes; }
	void cancelBuffer(scopy::iio::BufferHandle) override { ++cancels; }
	scopy::Result<void> refill(scopy::iio::BufferHandle) override
	{
		++refills;
		return {};
	}
	scopy::Result<void> push(scopy::iio::BufferHandle) override
	{
		++pushes;
		return {};
	}

	void *bufferStart(scopy::iio::BufferHandle) const override { return const_cast<char *>(storage.constData()); }
	void *bufferEnd(scopy::iio::BufferHandle) const override
	{
		return const_cast<char *>(storage.constData()) + storage.size();
	}
	ptrdiff_t bufferStep(scopy::iio::BufferHandle) const override { return step; }
	void *bufferFirst(scopy::iio::BufferHandle, scopy::iio::ChannelHandle ch) const override
	{
		return const_cast<char *>(storage.constData()) + chOffset.value(ch.ptr, 0);
	}

	scopy::iio::ChannelsMaskHandle createChannelsMask(unsigned int) override
	{
		++masksAlive;
		return {new QSet<long>()};
	}
	void destroyChannelsMask(scopy::iio::ChannelsMaskHandle mask) override
	{
		--masksAlive;
		delete static_cast<QSet<long> *>(mask.ptr);
	}
};

} // namespace dctest
