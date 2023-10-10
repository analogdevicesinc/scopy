#ifndef DEVICEBROWSER_H
#define DEVICEBROWSER_H

#include "scopy-core_export.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMap>
#include <QWidget>

#include <deviceicon.h>

namespace Ui {
class DeviceBrowser;
}

namespace scopy {
class SCOPY_CORE_EXPORT DeviceBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit DeviceBrowser(QWidget *parent = nullptr);
	~DeviceBrowser();
	QAbstractButton *getDeviceWidgetFor(QString id);
	void addDevice(QString id, Device *d, int position = -1);
	void removeDevice(QString id);
	void connectDevice(QString id);
	void disconnectDevice(QString id);

Q_SIGNALS:
	void requestDevice(QString id, int direction);
	void requestRemoveDevice(QString id);

public Q_SLOTS:

	void nextDevice();
	void prevDevice();

private Q_SLOTS:
	void updateSelectedDeviceIdx(QString);
	void forwardRequestDeviceWithDirection();

private:
	void initBtns();
	DeviceIcon *buildDeviceIcon(Device *d, QWidget *parent = nullptr);
	Ui::DeviceBrowser *ui;
	QButtonGroup *bg;
	QHBoxLayout *layout;
	QList<QAbstractButton *> list;
	int currentIdx;

	int getIndexOfId(QString k);
	QString getIdOfIndex(int idx);
	const char *devBrowserId = "DeviceBrowserId";
};
} // namespace scopy
#endif // DEVICEBROWSER_H
