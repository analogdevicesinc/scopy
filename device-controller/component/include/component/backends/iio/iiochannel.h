#pragma once

#include "component/channel.h"
#include "iioutil/handles.h"

namespace scopy::component::iio {

// IIO channel identity. Carries the libiio channel handle on top of the shared
// Channel identity (id/name/label/direction).
class IIOChannel : public Channel
{
	Q_OBJECT
public:
	explicit IIOChannel(QObject *parent = nullptr)
		: Channel(parent)
	{
	}

	scopy::iio::ChannelHandle handle() const { return m_handle; }
	void setHandle(scopy::iio::ChannelHandle handle) { m_handle = handle; }

private:
	scopy::iio::ChannelHandle m_handle;
};

} // namespace scopy::component::iio
