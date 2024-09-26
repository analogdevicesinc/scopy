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
#include <QStackedWidget>
#include <QWidget>
#include <menucollapsesection.h>

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
	void loadDeviceInfoPage();
	void initializeDevice();
	void removePluginsCheckBoxes();
	QTabWidget *createTabWidget(QWidget *parent);
	QWidget *createInfoSection(QWidget *parent);
	QWidget *createBtnsWidget(QWidget *parent);
	QWidget *createAddPage(QWidget *parent);

	EmuWidget *m_emuWidget;
	IioTabWidget *m_iioTabWidget;
	QString m_pendingUri;
	QFutureWatcher<bool> *m_fw;

	InfoPage *m_deviceInfoPage;
	PluginManager *m_pluginManager;
	DeviceImpl *m_deviceImpl;
	QList<PluginEnableWidget *> m_pluginDescriptionList;

	MenuCollapseSection *m_pluginBrowserSection;
	QPushButton *m_addBtn;
	QPushButton *m_backBtn;
	QLabel *m_connLostLabel;
	QWidget *m_addPage;
	QTabWidget *m_tabWidget;
	QStackedWidget *m_stackedWidget;
};
} // namespace scopy
#endif // SCOPYHOMEADDPAGE_H
