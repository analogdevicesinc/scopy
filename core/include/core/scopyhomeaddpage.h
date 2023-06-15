#ifndef SCOPYHOMEADDPAGE_H
#define SCOPYHOMEADDPAGE_H

#include <QWidget>
#include "deviceimpl.h"
#include "iioutil/iioscantask.h"
#include "iioutil/libserialportsupport.h"
#include "infopage.h"
#include "pluginentry.h"
#include "pluginmanager.h"
#include "scopy-core_export.h"
#include <QFuture>
#include <iio.h>

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
	void newDeviceAvailable(DeviceImpl *d);
	void deviceInfoAvailable(QMap<QString, QString> ctxInfo);
	void uriChanged(QString uri);

private Q_SLOTS:
	void futureVerify();
	void futureScan();
	bool verify();
	void deviceAddedToUi(QString);
	void scanFinished(QStringList);
	void deviceLoaderInitialized();
	void updateUri(QString uri);

private:
	void initAddPage();
	void initSubSections();
	void findAvailableSerialPorts();
	QMap<QString, QString> readContextAttributes(struct iio_context *ctx);
	void verifyIioBackend();
	void createDevice();
	void removePluginsCheckBoxes();
	QString getSerialPath();

	Ui::ScopyHomeAddPage *ui;
	QString pendingUri;
	QFutureWatcher<bool> *fw;

	IIOScanTask *scanTask;
	QStringList scanParamsList;
	InfoPage *deviceInfoPage;
	PluginManager *pluginManager;
	DeviceImpl *deviceImpl;
	LibSerialPortSupport *libSerialSupport;
	QList<PluginEntry*> pluginDescriptionList;

	const QVector<unsigned int> availableBaudRates = {2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
};
}
#endif // SCOPYHOMEADDPAGE_H
