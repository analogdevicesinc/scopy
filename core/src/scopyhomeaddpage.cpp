#include "scopyhomeaddpage.h"
#include "devicefactory.h"
#include "deviceloader.h"

#include "ui_scopyhomeaddpage.h"

#include <QFuture>
#include <QLoggingCategory>
#include <QtConcurrent>
#include <menusectionwidget.h>

Q_LOGGING_CATEGORY(CAT_HOME_ADD_PAGE, "ScopyHomeAddPage")

using namespace scopy;

ScopyHomeAddPage::ScopyHomeAddPage(QWidget *parent, PluginManager *pm)
	: QWidget(parent)
	, ui(new Ui::ScopyHomeAddPage)
	, pluginManager(pm)
	, deviceImpl(nullptr)
{
	ui->setupUi(this);
	this->setProperty("device_page", true);
	addTabs();
	initAddPage();
	setupInfoSection();
	pendingUri = "";

	connect(ui->btnAdd, &QPushButton::clicked, this, &ScopyHomeAddPage::addBtnClicked);
	connect(ui->btnBack, &QPushButton::clicked, this, &ScopyHomeAddPage::backBtnClicked);

	// verify iio device
	fw = new QFutureWatcher<bool>(this);
	connect(fw, &QFutureWatcher<bool>::finished, this, &ScopyHomeAddPage::onVerifyFinished, Qt::QueuedConnection);
	connect(this, &ScopyHomeAddPage::verifyFinished, iioTabWidget, &IioTabWidget::onVerifyFinished);
	connect(iioTabWidget, &IioTabWidget::startVerify, this, &ScopyHomeAddPage::futureVerify);

	connect(emuWidget, &EmuWidget::emuDeviceAvailable, this, &ScopyHomeAddPage::onEmuDeviceAvailable);

	connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=]() {
		if(ui->stackedWidget->currentWidget() == ui->addPage) {
			ui->btnAdd->setFocus();
		}
	});
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
	if(deviceImpl) {
		delete deviceImpl;
		deviceImpl = nullptr;
	}
}

void ScopyHomeAddPage::addTabs()
{
	iioTabWidget = new IioTabWidget();
	ui->tabWidget->addTab(iioTabWidget, "IIO");
	emuWidget = new EmuWidget();
	ui->tabWidget->addTab(emuWidget, "EMU");
}

void ScopyHomeAddPage::initAddPage()
{
	ui->stackedWidget->setCurrentWidget(ui->tabPage);
	ui->tabWidget->setCurrentWidget(iioTabWidget);

	ui->btnAdd->setProperty("blue_button", QVariant(true));
	ui->btnBack->setProperty("blue_button", QVariant(true));
	ui->btnAdd->setAutoDefault(true);
}

void ScopyHomeAddPage::futureVerify(QString uri, QString cat)
{
	removePluginsCheckBoxes();
	deviceInfoPage->clear();
	deviceImpl = DeviceFactory::build(uri, pluginManager, cat);
	QFuture<bool> f = QtConcurrent::run(std::bind(&DeviceImpl::verify, deviceImpl));
	fw->setFuture(f);
}

void ScopyHomeAddPage::onVerifyFinished()
{
	bool result = fw->result();
	if(result) {
		loadDeviceInfoPage();
		initializeDevice();
	} else {
		if(deviceImpl) {
			delete deviceImpl;
			deviceImpl = nullptr;
		}
		Q_EMIT verifyFinished(result);
	}
}

void ScopyHomeAddPage::loadDeviceInfoPage()
{
	QMap<QString, QString> deviceInfoMap = deviceImpl->readDeviceInfo();
	foreach(const QString &key, deviceInfoMap.keys()) {
		deviceInfoPage->update(key, deviceInfoMap[key]);
	}
}

void ScopyHomeAddPage::initializeDevice()
{
	if(deviceImpl) {
		DeviceLoader *dl = new DeviceLoader(deviceImpl, this);
		dl->init();
		connect(dl, &DeviceLoader::initialized, this, &ScopyHomeAddPage::deviceLoaderInitialized);
		connect(dl, &DeviceLoader::initialized, dl,
			&QObject::deleteLater); // don't forget to delete loader once we're done
	}
}

void ScopyHomeAddPage::deviceLoaderInitialized()
{
	QList<Plugin *> plugins = deviceImpl->plugins();
	for(Plugin *p : qAsConst(plugins)) {
		PluginEnableWidget *pluginDescription = new PluginEnableWidget(m_pluginBrowserSection);
		pluginDescription->setDescription(p->description());
		pluginDescription->checkBox()->setText(p->name());
		pluginDescription->checkBox()->setChecked(p->enabled());
		m_pluginBrowserSection->contentLayout()->addWidget(pluginDescription);
		pluginDescriptionList.push_back(pluginDescription);
		connect(pluginDescription->checkBox(), &QCheckBox::toggled, this, [=](bool en) { p->setEnabled(en); });
	}
	ui->stackedWidget->setCurrentWidget(ui->addPage);
	Q_EMIT verifyFinished(true);
}

void ScopyHomeAddPage::addBtnClicked()
{
	bool connection = deviceImpl->verify();
	if(!connection) {
		ui->labelConnectionLost->setText("Connection with " + deviceImpl->param() + " has been lost!");
		return;
	}
	pendingUri = deviceImpl->param();
	ui->labelConnectionLost->clear();
	Q_EMIT newDeviceAvailable(deviceImpl);
}

void ScopyHomeAddPage::deviceAddedToUi(QString id)
{
	if(!pendingUri.isEmpty()) {
		deviceImpl = nullptr;
		iioTabWidget->updateUri("");
		ui->stackedWidget->setCurrentWidget(ui->tabPage);
		Q_EMIT requestDevice(id);
		pendingUri = "";
	}
}

void ScopyHomeAddPage::backBtnClicked()
{
	if(deviceImpl) {
		delete deviceImpl;
		deviceImpl = nullptr;
	}
	ui->labelConnectionLost->clear();
	ui->stackedWidget->setCurrentWidget(ui->tabPage);
}

void ScopyHomeAddPage::onEmuDeviceAvailable(QString uri)
{
	ui->tabWidget->setCurrentWidget(iioTabWidget);
	iioTabWidget->updateUri(uri);
}

void ScopyHomeAddPage::removePluginsCheckBoxes()
{
	qDeleteAll(pluginDescriptionList);
	pluginDescriptionList.clear();
}

void ScopyHomeAddPage::setupInfoSection()
{
	MenuCollapseSection *deviceInfoSection =
		new MenuCollapseSection("Device info", MenuCollapseSection::MHCW_ONOFF, ui->infoSection);
	deviceInfoSection->contentLayout()->setSpacing(10);

	deviceInfoPage = new InfoPage(deviceInfoSection);
	deviceInfoPage->setAdvancedMode(false);
	deviceInfoPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	deviceInfoSection->contentLayout()->addWidget(deviceInfoPage);

	m_pluginBrowserSection =
		new MenuCollapseSection("Compatible plugins", MenuCollapseSection::MHCW_ONOFF, ui->infoSection);
	m_pluginBrowserSection->contentLayout()->setSpacing(10);

	ui->infoSection->layout()->addWidget(deviceInfoSection);
	ui->infoSection->layout()->addWidget(m_pluginBrowserSection);
	ui->infoSection->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

#include "moc_scopyhomeaddpage.cpp"
