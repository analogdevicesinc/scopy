#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "toolmenuentry.h"

namespace adiscope {
class Device {
public:
	virtual ~Device(){};
	virtual QString name() = 0;
	virtual QString uri() = 0;;
	virtual QWidget *icon() = 0;;
	virtual QWidget *page() = 0;;
	virtual QList<ToolMenuEntry> toolList() = 0;;

public Q_SLOTS:
	virtual void connectDev() = 0;
	virtual void disconnectDev() = 0;

Q_SIGNALS:
	virtual void toolListChanged() = 0;
	virtual void connected() = 0;
	virtual void disconnected()  = 0;

};
}

#endif // DEVICE_H
