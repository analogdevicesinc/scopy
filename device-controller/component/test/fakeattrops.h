#pragma once

#include "iioutil/iattrops.h"

#include <QHash>

namespace dctest {

// In-memory IAttrOps: each AttrHandle.ptr is a key into a value map. read()
// returns the stored bytes; write() overwrites them. Range/available optional.
class FakeAttrOps : public scopy::iio::IAttrOps
{
public:
	scopy::iio::AttrHandle make(const QString &name, const QByteArray &value)
	{
		auto *key = new int(++m_counter);
		m_names.insert(key, name);
		m_values.insert(key, value);
		return {key};
	}

	scopy::iio::AttrHandle contextAttr(scopy::iio::ContextHandle, const QString &) override { return {}; }
	scopy::iio::AttrHandle deviceAttr(scopy::iio::DeviceHandle, const QString &) override { return {}; }
	scopy::iio::AttrHandle debugAttr(scopy::iio::DeviceHandle, const QString &) override { return {}; }
	scopy::iio::AttrHandle bufferAttr(scopy::iio::DeviceHandle, unsigned int, const QString &) override
	{
		return {};
	}
	scopy::iio::AttrHandle channelAttr(scopy::iio::ChannelHandle, const QString &) override { return {}; }
	void releaseAttr(scopy::iio::AttrHandle) override {}

	scopy::Result<QByteArray> read(scopy::iio::AttrHandle attr) override
	{
		return m_values.value(static_cast<int *>(attr.ptr));
	}
	scopy::Result<void> write(scopy::iio::AttrHandle attr, const QString &value) override
	{
		m_values.insert(static_cast<int *>(attr.ptr), value.toUtf8());
		return {};
	}
	scopy::Result<void> getRange(scopy::iio::AttrHandle, double &, double &, double &) const override
	{
		return scopy::Unexpected(scopy::Error{});
	}
	scopy::Result<void> getAvailable(scopy::iio::AttrHandle, QStringList &) const override
	{
		return scopy::Unexpected(scopy::Error{});
	}

private:
	int m_counter = 0;
	QHash<int *, QString> m_names;
	QHash<int *, QByteArray> m_values;
};

} // namespace dctest
