#include "scopyhomeaddpage.h"
#include "devicefactory.h"
#include "deviceloader.h"

#include <QFuture>
#include <QLoggingCategory>
#include <QtConcurrent>
#include <menusectionwidget.h>

Q_LOGGING_CATEGORY(CAT_HOME_ADD_PAGE, "ScopyHomeAddPage")

using namespace scopy;

ScopyHomeAddPage::ScopyHomeAddPage(QWidget *parent, PluginManager *pm)
	: QWidget(parent)
	, m_pluginManager(pm)
	, m_deviceImpl(nullptr)
{
	StyleHelper::BackgroundPage(this, "add_page");
	setProperty("device_page", true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	m_stackedWidget = new QStackedWidget(this);
	m_tabWidget = createTabWidget(m_stackedWidget);
	m_addPage = createAddPage(m_stackedWidget);
	m_stackedWidget->addWidget(m_tabWidget);
	m_stackedWidget->addWidget(m_addPage);
	m_stackedWidget->setCurrentWidget(m_tabWidget);

	layout->addWidget(m_stackedWidget);
	m_pendingUri = "";

	connect(m_addBtn, &QPushButton::clicked, this, &ScopyHomeAddPage::addBtnClicked);
	connect(m_backBtn, &QPushButton::clicked, this, &ScopyHomeAddPage::backBtnClicked);

	// verify iio device
	m_fw = new QFutureWatcher<bool>(this);
	connect(m_fw, &QFutureWatcher<bool>::finished, this, &ScopyHomeAddPage::onVerifyFinished, Qt::QueuedConnection);
	connect(this, &ScopyHomeAddPage::verifyFinished, m_iioTabWidget, &IioTabWidget::onVerifyFinished);
	connect(m_iioTabWidget, &IioTabWidget::startVerify, this, &ScopyHomeAddPage::futureVerify);

	connect(m_emuWidget, &EmuWidget::emuDeviceAvailable, this, &ScopyHomeAddPage::onEmuDeviceAvailable);

	connect(m_stackedWidget, &QStackedWidget::currentChanged, this, [=]() {
		if(m_stackedWidget->currentWidget() == m_addPage) {
			m_addBtn->setFocus();
		}
	});
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	if(m_deviceImpl) {
		delete m_deviceImpl;
		m_deviceImpl = nullptr;
	}
}

void ScopyHomeAddPage::futureVerify(QString uri, QString cat)
{
	removePluginsCheckBoxes();
	m_deviceInfoPage->clear();
	m_deviceImpl = DeviceFactory::build(uri, m_pluginManager, cat);
	QFuture<bool> f = QtConcurrent::run(std::bind(&DeviceImpl::verify, m_deviceImpl));
	m_fw->setFuture(f);
}

void ScopyHomeAddPage::onVerifyFinished()
{
	bool result = m_fw->result();
	if(result) {
		loadDeviceInfoPage();
		initializeDevice();
	} else {
		if(m_deviceImpl) {
			delete m_deviceImpl;
			m_deviceImpl = nullptr;
		}
		Q_EMIT verifyFinished(result);
	}
}

void ScopyHomeAddPage::loadDeviceInfoPage()
{
	QMap<QString, QString> deviceInfoMap = m_deviceImpl->readDeviceInfo();
	foreach(const QString &key, deviceInfoMap.keys()) {
		m_deviceInfoPage->update(key, deviceInfoMap[key]);
	}
}

void ScopyHomeAddPage::initializeDevice()
{
	if(m_deviceImpl) {
		DeviceLoader *dl = new DeviceLoader(m_deviceImpl, this);
		dl->init();
		connect(dl, &DeviceLoader::initialized, this, &ScopyHomeAddPage::deviceLoaderInitialized);
		connect(dl, &DeviceLoader::initialized, dl,
			&QObject::deleteLater); // don't forget to delete loader once we're done
	}
}

void ScopyHomeAddPage::deviceLoaderInitialized()
{
	QList<Plugin *> plugins = m_deviceImpl->plugins();
	for(Plugin *p : qAsConst(plugins)) {
		PluginEnableWidget *pluginDescription = new PluginEnableWidget(m_pluginBrowserSection);
		pluginDescription->setDescription(p->description());
		pluginDescription->checkBox()->setText(p->name());
		pluginDescription->checkBox()->setChecked(p->enabled());
		m_pluginBrowserSection->contentLayout()->addWidget(pluginDescription);
		m_pluginDescriptionList.push_back(pluginDescription);
		connect(pluginDescription->checkBox(), &QCheckBox::toggled, this, [=](bool en) { p->setEnabled(en); });
	}
	m_stackedWidget->setCurrentWidget(m_addPage);
	Q_EMIT verifyFinished(true);
}

void ScopyHomeAddPage::addBtnClicked()
{
	bool connection = m_deviceImpl->verify();
	if(!connection) {
		m_connLostLabel->setText("Connection with " + m_deviceImpl->param() + " has been lost!");
		return;
	}
	m_pendingUri = m_deviceImpl->param();
	m_connLostLabel->clear();
	Q_EMIT newDeviceAvailable(m_deviceImpl);
}

void ScopyHomeAddPage::deviceAddedToUi(QString id)
{
	if(!m_pendingUri.isEmpty()) {
		m_deviceImpl = nullptr;
		m_iioTabWidget->updateUri("");
		m_stackedWidget->setCurrentWidget(m_tabWidget);
		Q_EMIT requestDevice(id);
		m_pendingUri = "";
	}
}

void ScopyHomeAddPage::backBtnClicked()
{
	if(m_deviceImpl) {
		delete m_deviceImpl;
		m_deviceImpl = nullptr;
	}
	m_connLostLabel->clear();
	m_stackedWidget->setCurrentWidget(m_tabWidget);
}

void ScopyHomeAddPage::onEmuDeviceAvailable(QString uri)
{
	m_tabWidget->setCurrentWidget(m_iioTabWidget);
	m_iioTabWidget->updateUri(uri);
}

void ScopyHomeAddPage::removePluginsCheckBoxes()
{
	qDeleteAll(m_pluginDescriptionList);
	m_pluginDescriptionList.clear();
}

QTabWidget *ScopyHomeAddPage::createTabWidget(QWidget *parent)
{
	QTabWidget *tabWidget = new QTabWidget(parent);
	m_iioTabWidget = new IioTabWidget(tabWidget);
	tabWidget->addTab(m_iioTabWidget, "IIO");
	m_emuWidget = new EmuWidget(tabWidget);
	tabWidget->addTab(m_emuWidget, "EMU");
	tabWidget->setCurrentWidget(m_iioTabWidget);
	return tabWidget;
}

QWidget *ScopyHomeAddPage::createInfoSection(QWidget *parent)
{
	QScrollArea *infoScrollArea = new QScrollArea(parent);
	QWidget *infoSection = new QWidget(infoScrollArea);
	infoSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *layInfoSection = new QVBoxLayout(infoSection);
	layInfoSection->setSpacing(10);
	layInfoSection->setMargin(0);
	infoSection->setLayout(layInfoSection);

	infoScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	infoScrollArea->setWidgetResizable(true);
	infoScrollArea->setWidget(infoSection);

	MenuCollapseSection *deviceInfoSection =
		new MenuCollapseSection("Device info", MenuCollapseSection::MHCW_ONOFF, infoSection);
	deviceInfoSection->contentLayout()->setSpacing(10);
	deviceInfoSection->contentLayout()->setMargin(0);

	m_deviceInfoPage = new InfoPage(deviceInfoSection);
	m_deviceInfoPage->setAdvancedMode(false);
	m_deviceInfoPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	deviceInfoSection->contentLayout()->addWidget(m_deviceInfoPage);

	m_pluginBrowserSection =
		new MenuCollapseSection("Compatible plugins", MenuCollapseSection::MHCW_ONOFF, infoSection);
	m_pluginBrowserSection->contentLayout()->setSpacing(10);
	m_pluginBrowserSection->contentLayout()->setMargin(0);

	layInfoSection->addWidget(deviceInfoSection);
	layInfoSection->addWidget(m_pluginBrowserSection);
	layInfoSection->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return infoScrollArea;
}

QWidget *ScopyHomeAddPage::createBtnsWidget(QWidget *parent)
{
	QWidget *btnsWidget = new QWidget(parent);
	QHBoxLayout *btnsLay = new QHBoxLayout(btnsWidget);
	btnsLay->setMargin(0);
	btnsLay->setAlignment(Qt::AlignRight);

	m_backBtn = new QPushButton(btnsWidget);
	m_backBtn->setText("BACK");
	StyleHelper::BlueButton(m_backBtn);
	m_backBtn->setFixedWidth(128);

	m_addBtn = new QPushButton(btnsWidget);
	m_addBtn->setText("ADD DEVICE");
	m_addBtn->setAutoDefault(true);
	StyleHelper::BlueButton(m_addBtn);
	m_addBtn->setFixedWidth(128);

	btnsLay->addWidget(m_backBtn);
	btnsLay->addWidget(m_addBtn);
	return btnsWidget;
}

QWidget *ScopyHomeAddPage::createAddPage(QWidget *parent)
{
	QWidget *addPage = new QWidget(parent);
	addPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *addPageLay = new QVBoxLayout(addPage);
	addPageLay->setSpacing(10);
	addPage->setLayout(addPageLay);

	QWidget *infoSection = createInfoSection(addPage);
	QWidget *buttons = createBtnsWidget(addPage);
	m_connLostLabel = new QLabel(parent);
	m_connLostLabel->setText("");

	addPageLay->addItem(new QSpacerItem(20, 40, QSizePolicy::Preferred, QSizePolicy::Preferred));
	addPageLay->addWidget(infoSection);
	addPageLay->addWidget(m_connLostLabel);
	addPageLay->addWidget(buttons);
	return addPage;
}

#include "moc_scopyhomeaddpage.cpp"
