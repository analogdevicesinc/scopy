#ifndef SWIOTRUNTIME_HPP
#define SWIOTRUNTIME_HPP

#include "src/runtime/ad74413r/ad74413r.hpp"

namespace adiscope {
namespace gui {
class GenericMenu;
class ChannelManager;
}

#define AD_NAME "ad74413r"
#define AD_TRIGGER_NAME "ad74413r-dev0"
#define TRIGGER_TIMER_MS 1000

class SwiotRuntime : public QObject
{
	Q_OBJECT
public:
	SwiotRuntime();
	~SwiotRuntime();

	bool isRuntimeCtx();
	void setContext(iio_context *ctx);
//	void triggerPing();
public Q_SLOTS:
//	void onTriggerTimeout();
	void onBackBtnPressed();
Q_SIGNALS:
	void backBtnPressed();
private:
	void createDevicesMap();
private:
	iio_context *m_iioCtx;
	QTimer *m_triggerTimer;
	QMap<QString, struct iio_device*> m_iioDevices;
};
}


#endif // SWIOTRUNTIME_HPP
