#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

class Device {
public:
	virtual ~Device(){};
	virtual QString name() = 0;
	virtual QString uri() = 0;;
	virtual QWidget *icon() = 0;;
	virtual QWidget *page() = 0;;
	virtual QStringList toolList() = 0;;

};

#endif // DEVICE_H
