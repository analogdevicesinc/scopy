#include "scopyhomeaddpage.h"
#include "iioutil/scopy-iioutil_config.h"
#include "devicefactory.h"
#include "deviceloader.h"
#include "qmovie.h"
#include "ui_scopyhomeaddpage.h"
#include "iioutil/contextprovider.h"
#include <QtConcurrent>
#include <QFuture>
#include <QLoggingCategory>
#include "iioutil/iioscantask.h"

Q_LOGGING_CATEGORY(CAT_HOME_ADD_PAGE,"ScopyHomeAddPage")

using namespace scopy;
ScopyHomeAddPage::ScopyHomeAddPage(QWidget *parent, PluginManager *pm) :
	QWidget(parent),
	ui(new Ui::ScopyHomeAddPage),
	emuWidget(new EmuWidget()),
	pluginManager(pm), deviceImpl(nullptr)
{
	ui->setupUi(this);
	this->setProperty("device_page", true);
	initAddPage();
	initSubSections();
	verifyIioBackend();
	pendingUri = "";

	//verify
	fw = new QFutureWatcher<bool>(this);
	connect(fw, &QFutureWatcher<bool>::started, ui->btnVerify, &AnimationPushButton::startAnimation, Qt::QueuedConnection);
	connect(fw, &QFutureWatcher<bool>::finished, this, &ScopyHomeAddPage::verifyFinished, Qt::QueuedConnection);
	connect(ui->btnVerify, SIGNAL(clicked()), this, SLOT(futureVerify()), Qt::QueuedConnection);

	//scan
	fwScan = new QFutureWatcher<int>(this);
	connect(fwScan, &QFutureWatcher<int>::started, ui->btnScan, &AnimationPushButton::startAnimation, Qt::QueuedConnection);
	connect(fwScan, &QFutureWatcher<int>::finished, this, &ScopyHomeAddPage::scanFinished, Qt::QueuedConnection);
	connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(futureScan()), Qt::QueuedConnection);

	//btns connections
	connect(ui->btnBack, &QPushButton::clicked, this, [=](){
		ui->labelConnectionLost->clear();
		ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);
	});
	connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addBtnClicked()));
	connect(ui->comboBoxContexts,&QComboBox::textActivated, this, [=](){
		Q_EMIT uriChanged(ui->comboBoxContexts->currentText());
	});

	//serial widget connections
	connect(ui->comboBoxSerialPort, &QComboBox::textActivated, this, [=](){
		Q_EMIT uriChanged(getSerialPath());
	});
	connect(ui->comboBoxBaudRate, &QComboBox::textActivated, this, [=](){
		Q_EMIT uriChanged(getSerialPath());
	});
	connect(ui->editSerialFrameConfig, &QLineEdit::returnPressed, this, [=](){
		Q_EMIT uriChanged(getSerialPath());
	});
	connect(this, &ScopyHomeAddPage::uriChanged, this, &ScopyHomeAddPage::updateUri);
	connect(ui->editUri, &QLineEdit::returnPressed, this, [=](){
		Q_EMIT ui->btnVerify->clicked();
	});
	connect(ui->editUri, &QLineEdit::textChanged, this, [=](QString uri){
		ui->btnVerify->setEnabled(!uri.isEmpty());
	});
	connect(this, &ScopyHomeAddPage::deviceInfoAvailable, this, [=](QMap<QString, QString> ctxInfo){
		foreach (const QString &key, ctxInfo.keys()){
			deviceInfoPage->update(key, ctxInfo[key]);
		}
	},Qt::QueuedConnection);
	connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](){
		if (ui->stackedWidget->currentWidget() == ui->deviceInfoPage) {
			ui->btnAdd->setFocus();
		} else {
			ui->btnScan->setFocus();
		}
	});
	connect(emuWidget, &EmuWidget::emuDeviceAvailable, this, &ScopyHomeAddPage::onEmuDeviceAvailable);
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
}

void ScopyHomeAddPage::initAddPage()
{
	bool hasLibSerialPort = false;
#ifdef WITH_LIBSERIALPORT
	hasLibSerialPort = true;
#endif
	bool serialBackEnd = iio_has_backend("serial");

	QMovie *veifyIcon(new QMovie(this));
	veifyIcon->setFileName(":/gui/loading.gif");
	ui->btnVerify->setAnimation(veifyIcon);

	QMovie *scanIcon(new QMovie(this));
	scanIcon->setFileName(":/gui/loading.gif");
	ui->btnScan->setAnimation(scanIcon);

	QRegExp re("[5-9]{1}(n|o|e|m|s){1}[1-2]{1}(x|r|d){0,1}$");
	QRegExpValidator *validator = new QRegExpValidator(re, this);

	ui->editSerialFrameConfig->setValidator(validator);
	ui->serialSettingsWidget->setEnabled(hasLibSerialPort && serialBackEnd);
	ui->btnAdd->setProperty("blue_button", QVariant(true));
	ui->btnVerify->setProperty("blue_button", QVariant(true));
	ui->btnVerify->setEnabled(false);
	ui->btnScan->setProperty("blue_button", QVariant(true));
	ui->btnScan->setIcon(QIcon(":/gui/icons/refresh.svg"));
	ui->btnScan->setIconSize(QSize(25,25));
	ui->btnBack->setProperty("blue_button", QVariant(true));
	ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);
	ui->labelConnectionLost->clear();
	addScanFeedbackMsg("No scanned contexts... Press the refresh button!");
	ui->tabEmu->layout()->addWidget(emuWidget);

	ui->btnScan->setAutoDefault(true);
	ui->btnVerify->setAutoDefault(true);
	ui->btnAdd->setAutoDefault(true);

	for (int baudRate : availableBaudRates) {
		ui->comboBoxBaudRate->addItem(QString::number(baudRate));
	}
}

void ScopyHomeAddPage::initSubSections()
{
	ui->devicePluginBrowser->setLabel("Compatible plugins");
	ui->devicePluginBrowser->getContentWidget()->layout()->setSpacing(10);

	ui->deviceInfo->setLabel("Device info");
	deviceInfoPage = new InfoPage(ui->deviceInfo->getContentWidget());
	deviceInfoPage->setAdvancedMode(false);
	deviceInfoPage->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	ui->deviceInfo->getContentWidget()->layout()->addWidget(deviceInfoPage);
	QSpacerItem *vSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	ui->deviceInfo->getContentWidget()->layout()->addItem(vSpacer);
}

void ScopyHomeAddPage::findAvailableSerialPorts()
{
	if (scanParamsList.contains("serial:")) {
		QVector<QString> portsName = IIOScanTask::getSerialPortsName();
		ui->comboBoxSerialPort->clear();
		if (!portsName.empty()) {
			for (const QString &port : portsName) {
				ui->comboBoxSerialPort->addItem(port);
			}
		}
	}
}

void ScopyHomeAddPage::verifyIioBackend()
{
	bool scan = false;
	int backEndsCount = iio_get_backends_count();
	bool hasLibSerialPort = false;
#ifdef WITH_LIBSERIALPORT
	hasLibSerialPort = true;
#endif
	for (int i = 0; i < backEndsCount; i++) {
		QString backEnd(iio_get_backend(i));
		if (backEnd.compare("xml") == 0 || (!hasLibSerialPort && backEnd.compare("serial") == 0)) {
			continue;
		}
		createBackEndCheckBox(backEnd);
		scan = true;
	}
	ui->btnScan->setVisible(scan);
}

QMap<QString, QString> ScopyHomeAddPage::readContextAttributes(struct iio_context *ctx)
{
	QMap <QString, QString> contextAttributes;
	for(int i=0;i<iio_context_get_attrs_count(ctx);i++) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(ctx,i,&name,&value);
		if(ret != 0)
			continue;
		contextAttributes[name] = value;
	}
	return contextAttributes;
}

bool ScopyHomeAddPage::verify() {
	QString uri = ui->editUri->text();
	struct iio_context* ctx = ContextProvider::GetInstance()->open(uri);
	if(ctx) {
		QMap<QString, QString> attrMap = readContextAttributes(ctx);
		ContextProvider::GetInstance()->close(uri);
		Q_EMIT deviceInfoAvailable(attrMap);
		return true;
	}	
	return false;
}

void ScopyHomeAddPage::futureVerify()
{
	QRegExp ipRegex("^(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$");
	QString uri(ui->editUri->text());
	bool isIp = uri.contains(ipRegex);
	if (isIp && !ui->editUri->text().contains("ip:")) {
		ui->editUri->blockSignals(true);
		ui->editUri->setText("ip:" + uri);
		ui->editUri->blockSignals(false);
	}
	removePluginsCheckBoxes();
	deviceInfoPage->clear();
	QFuture<bool> f = QtConcurrent::run(std::bind(&ScopyHomeAddPage::verify,this));
	fw->setFuture(f);
}

void ScopyHomeAddPage::futureScan()
{
	scanList.clear();
	QString scanParams = scanParamsList.join("").remove("serial:");
	QFuture<int> f = QtConcurrent::run(std::bind(&IIOScanTask::scan, &scanList, scanParams));
	fwScan->setFuture(f);
}

void ScopyHomeAddPage::deviceAddedToUi(QString id)
{
	if(!pendingUri.isEmpty()) {
		ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);
		ui->editUri->clear();
		Q_EMIT requestDevice(id);
		pendingUri = "";
	}
}

void ScopyHomeAddPage::scanFinished()
{
	int retCode = fwScan->result();
	ui->btnScan->stopAnimation();
	ui->comboBoxContexts->clear();
	ui->uriMessageLabel->clear();
	if (retCode < 0) {
		addScanFeedbackMsg("Scan command failed!");
		qWarning(CAT_HOME_ADD_PAGE) <<"iio_scan_context_get_info_list error " << retCode;
		return;
	}
	if (scanList.isEmpty()) {
		addScanFeedbackMsg("No scanned contexts available!");
		return;
	}
	if (!ui->comboBoxContexts->isEnabled()) {
		ui->comboBoxContexts->setEnabled(true);
	}
	for (const auto &ctx: qAsConst(scanList)) {
		ui->comboBoxContexts->addItem(ctx);
	}
	findAvailableSerialPorts();
	updateUri(ui->comboBoxContexts->currentText());
}

void ScopyHomeAddPage::verifyFinished()
{
	bool result = fw->result();
	if (result) {
		ui->uriMessageLabel->clear();
		createDevice("iio");
	} else {
		ui->uriMessageLabel->clear();
		ui->uriMessageLabel->setText("\""+ui->editUri->text() + "\" not a valid context!");
		ui->btnVerify->stopAnimation();
	}
}

void ScopyHomeAddPage::deviceLoaderInitialized()
{
	QList<Plugin *> plugins = deviceImpl->plugins();
	for (Plugin *p : qAsConst(plugins)) {
		PluginEnableWidget *pluginDescription = new PluginEnableWidget(ui->devicePluginBrowser->getContentWidget());
		pluginDescription->setDescription(p->description());
		pluginDescription->checkBox()->setText(p->name());
		pluginDescription->checkBox()->setChecked(p->enabled());
		ui->devicePluginBrowser->getContentWidget()->layout()->addWidget(pluginDescription);
		pluginDescriptionList.push_back(pluginDescription);
		connect(pluginDescription->checkBox(), &QCheckBox::toggled, this, [=](bool en){
			p->setEnabled(en);
		});
	}
	ui->stackedWidget->setCurrentWidget(ui->deviceInfoPage);
	ui->btnVerify->stopAnimation();
}

void ScopyHomeAddPage::updateUri(QString uri)
{
	ui->editUri->clear();
	ui->editUri->setText(uri);
	if (!uri.isEmpty()) {
		ui->btnVerify->setFocus();
	}
}

void ScopyHomeAddPage::addBtnClicked()
{
	//can be used only for iio devices
	pendingUri = ui->editUri->text();
	iio_context *ctx = ContextProvider::GetInstance()->open(deviceImpl->param());
	if (!ctx) {
		ui->labelConnectionLost->setText("Connection with " + deviceImpl->param() + " has been lost!");
		return;
	}
	ContextProvider::GetInstance()->close(deviceImpl->param());
	ui->labelConnectionLost->clear();
	Q_EMIT newDeviceAvailable(deviceImpl);
}

void ScopyHomeAddPage::onEmuDeviceAvailable(QString uri)
{
	updateUri(uri);
	ui->tabWidget->setCurrentWidget(ui->tabIio);
	ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);
}

void ScopyHomeAddPage::createDevice(QString cat)
{
	QString uri = ui->editUri->text();
	deviceImpl = DeviceFactory::build(uri, pluginManager, cat);
	DeviceLoader* dl = new DeviceLoader(deviceImpl, this);
	dl->init();
	connect(dl, &DeviceLoader::initialized, this, &ScopyHomeAddPage::deviceLoaderInitialized);
	connect(dl, &DeviceLoader::initialized, dl, &QObject::deleteLater); // don't forget to delete loader once we're done
}

void ScopyHomeAddPage::removePluginsCheckBoxes()
{
	qDeleteAll(pluginDescriptionList);
	pluginDescriptionList.clear();
}

QString ScopyHomeAddPage::getSerialPath()
{
	QString serialPath = "serial:";
	serialPath.append(ui->comboBoxSerialPort->currentText());
	serialPath.append("," + ui->comboBoxBaudRate->currentText());
	serialPath.append("," + ui->editSerialFrameConfig->text());
	return serialPath;
}

void ScopyHomeAddPage::createBackEndCheckBox(QString backEnd)
{
	QCheckBox *cb = new QCheckBox();
	cb->setText(backEnd);
	connect(cb, &QCheckBox::toggled, this, [=](bool en) {
		if (en) {
			scanParamsList.push_back(backEnd + ":");
		} else {
			scanParamsList.removeOne(backEnd + ":");
		}
		if (backEnd.compare("serial") == 0) {
			ui->serialSettingsWidget->setEnabled(en);
		}
		if (scanParamsList.empty()) {
			ui->btnScan->setEnabled(false);
		} else if (!ui->btnScan->isEnabled()) {
			ui->btnScan->setEnabled(true);
		}
	});
	cb->setChecked(true);
	ui->filterCheckBoxes->layout()->addWidget(cb);
}

void ScopyHomeAddPage::addScanFeedbackMsg(QString message)
{
	ui->comboBoxContexts->clear();
	ui->comboBoxContexts->addItem(message);
	ui->comboBoxContexts->setEnabled(false);
	updateUri("");
}

void ScopyHomeAddPage::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	ui->btnScan->setFocus();
}

#include "moc_scopyhomeaddpage.cpp"
