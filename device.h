#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

class Device : public QObject
{
	Q_OBJECT
public:
	explicit Device(QObject *parent = nullptr);
	~Device();

private:
	QWidget *icon;
	QWidget *page;

};

#endif // DEVICE_H
