#ifndef SCOPYHOMEADDPAGE_H
#define SCOPYHOMEADDPAGE_H

#include "deviceimpl.h"
#include "emuwidget.h"
#include "iiotabwidget.h"
#include "infopage.h"
#include "pluginenablewidget.h"
#include "pluginmanager.h"
#include "scopy-core_export.h"

#include <iio.h>

#include <QFuture>
#include <QWidget>

namespace Ui {
class ScopyHomeAddPage;
}

namespace scopy {
class SCOPY_CORE_EXPORT ScopyHomeAddPage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomeAddPage(QWidget *parent = nullptr, PluginManager *pm = nullptr);
	~ScopyHomeAddPage();

Q_SIGNALS:
	void requestAddDevice(QString, QString);
	void requestDevice(QString);
	void newDeviceAvailable(scopy::DeviceImpl *d);
	void verifyFinished(bool valid);

private Q_SLOTS:
	void futureVerify(QString uri, QString cat);
	void deviceAddedToUi(QString);
	void onVerifyFinished();
	void deviceLoaderInitialized();
	void addBtnClicked();
	void backBtnClicked();
	void onEmuDeviceAvailable(QString uri);

private:
	void addTabs();
	void initAddPage();
	void initSubSections();
	void loadDeviceInfoPage();
	void initializeDevice();
	void removePluginsCheckBoxes();

	Ui::ScopyHomeAddPage *ui;
	EmuWidget *emuWidget;
	IioTabWidget *iioTabWidget;
	QString pendingUri;
	QFutureWatcher<bool> *fw;

	InfoPage *deviceInfoPage;
	PluginManager *pluginManager;
	DeviceImpl *deviceImpl;
	QList<PluginEnableWidget *> pluginDescriptionList;
};
} // namespace scopy
#endif // SCOPYHOMEADDPAGE_H
