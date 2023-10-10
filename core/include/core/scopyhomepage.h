#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include "device.h"
#include "pluginmanager.h"
#include "scopy-core_export.h"
#include "scopyhomeaddpage.h"

#include <QPushButton>
#include <QWidget>

namespace Ui {
class ScopyHomePage;
}

namespace scopy {
class SCOPY_CORE_EXPORT ScopyHomePage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomePage(QWidget *parent = nullptr, PluginManager *pm = nullptr);
	~ScopyHomePage();
	QPushButton *scanControlBtn();
Q_SIGNALS:

	void requestAddDevice(QString cat, QString id);
	void requestRemoveDevice(QString id);
	void requestDevice(QString id);
	void deviceAddedToUi(QString id);
	void newDeviceAvailable(DeviceImpl *d);

public Q_SLOTS:
	void addDevice(QString id, Device *);
	void removeDevice(QString id);
	void viewDevice(QString id);
	void connectDevice(QString);
	void disconnectDevice(QString);

private:
	Ui::ScopyHomePage *ui;
	ScopyHomeAddPage *add;
};
} // namespace scopy

#endif // SCOPYHOMEPAGE_H
