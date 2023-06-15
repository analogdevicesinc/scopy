#include "scopyhomeaddpage.h"

#include "devicefactory.h"
#include "deviceloader.h"
#include "qmovie.h"
#include "ui_scopyhomeaddpage.h"
#include "iioutil/contextprovider.h"
#include <QtConcurrent>
#include <QFuture>

using namespace scopy;
ScopyHomeAddPage::ScopyHomeAddPage(QWidget *parent, PluginManager *pm) :
	QWidget(parent),
	ui(new Ui::ScopyHomeAddPage),
	pluginManager(pm), deviceImpl(nullptr),
	libSerialSupport(new LibSerialPortSupport())
{
	ui->setupUi(this);
	this->setProperty("device_page", true);
	initAddPage();
	initSubSections();
	verifyIioBackend();
	pendingUri = "";

	//loading icon
	loadingIcon = new QMovie(this);
	loadingIcon->setFileName(":/gui/loading.gif");
	connect(loadingIcon, &QMovie::frameChanged, [=]{
	    ui->btnScan->setIcon(loadingIcon->currentPixmap());
	});

	//verify
	fw = new QFutureWatcher<bool>(this);
	connect(fw, &QFutureWatcher<bool>::finished, this, [=](){
		bool result = fw->result();
		if (result == true) {
			ui->uriMessageLabel->clear();
			createDevice();
		} else {
			ui->uriMessageLabel->clear();
			ui->uriMessageLabel->setText("\""+ui->editUri->text() + "\" not a valid context!");
		}
	});

	//scan
	scanTask = new IIOScanTask(this);
	connect(scanTask, SIGNAL(scanFinished(QStringList)), this, SLOT(scanFinished(QStringList)));
	fwScan = new QFutureWatcher<void>(this);
	connect(fwScan, &QFutureWatcher<void>::finished, this, [=](){
		loadingIcon->stop();
		ui->btnScan->setIcon(QIcon());
		ui->btnScan->setText("Scan");
		ui->btnScan->setEnabled(true);
	});

	//btns connections
	connect(ui->btnBack, &QPushButton::clicked, this, [=](){
		ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);
	});
	connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(futureScan()));
	connect(ui->btnVerify, SIGNAL(clicked()), this, SLOT(futureVerify()));
	connect(ui->btnAdd, &QPushButton::clicked, this, [=](){
		pendingUri = ui->editUri->text();
		Q_EMIT newDeviceAvailable(deviceImpl);
	});
	connect(ui->comboBoxContexts,&QComboBox::textActivated, [=](){
		Q_EMIT uriChanged(ui->comboBoxContexts->currentText());
	});

	//serial widget connections
	connect(ui->comboBoxSerialPort, &QComboBox::textActivated, [=](){
		Q_EMIT uriChanged(getSerialPath());
	});
	connect(ui->comboBoxBaudRate, &QComboBox::textActivated, [=](){
		Q_EMIT uriChanged(getSerialPath());
	});
	connect(ui->editSerialFrameConfig, &QLineEdit::textChanged, [=](){
		Q_EMIT uriChanged(getSerialPath());
	});
	connect(this, &ScopyHomeAddPage::uriChanged, this, &ScopyHomeAddPage::updateUri);
	connect(ui->editUri, &QLineEdit::textChanged, [=](QString uri){
		if (uri.isEmpty()) {
			ui->btnVerify->setEnabled(false);
		} else {
			ui->btnVerify->setEnabled(true);
		}
	});

	connect(this, &ScopyHomeAddPage::deviceInfoAvailable, this, [=](QMap<QString, QString> ctxInfo){
		for (QString key : ctxInfo.keys()) {
			deviceInfoPage->update(key, ctxInfo[key]);
		}
	},Qt::QueuedConnection);
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
	if (libSerialSupport) {
		delete libSerialSupport;
	}
}

void ScopyHomeAddPage::initAddPage()
{
	bool hasLibSerialPort = libSerialSupport->hasLibSerialSupport();
	bool serialBackEnd = iio_has_backend("serial");

	QRegExp re("[5-9]{1}(n|o|e|m|s){1}[1-2]{1}(x|r|d){1}$");
	QRegExpValidator *validator = new QRegExpValidator(re, this);

	ui->editSerialFrameConfig->setValidator(validator);
	ui->serialSettingsWidget->setEnabled(hasLibSerialPort && serialBackEnd);
	ui->btnAdd->setProperty("blue_button", QVariant(true));
	ui->btnVerify->setProperty("blue_button", QVariant(true));
	ui->btnScan->setProperty("blue_button", QVariant(true));
	ui->btnBack->setProperty("blue_button", QVariant(true));
	ui->btnVerify->setEnabled(false);
	ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);

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
		QVector<QString> portsName = libSerialSupport->getSerialPortsName();
		ui->comboBoxSerialPort->clear();
		if (!portsName.empty()) {
			for (QString port : portsName) {
				ui->comboBoxSerialPort->addItem(port);
			}
		}
	}
}

void ScopyHomeAddPage::verifyIioBackend()
{
	bool scan = false;
	int backEndsCount = iio_get_backends_count();
	for (int i = 0; i < backEndsCount; i++) {
		QString backEnd(iio_get_backend(i));
		if (backEnd.compare("xml") == 0) {
			continue;
		}
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
		});
		cb->setChecked(true);
		ui->filterCheckBoxes->layout()->addWidget(cb);
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
	removePluginsCheckBoxes();
	deviceInfoPage->clear();
	QFuture<bool> f = QtConcurrent::run(std::bind(&ScopyHomeAddPage::verify,this));
	fw->setFuture(f);
}

void ScopyHomeAddPage::futureScan()
{
	QString scanParams = scanParamsList.join("");
	QFuture<void> f = QtConcurrent::run([&](){
		scanTask->setScanParams(scanParams);
		scanTask->run();
	});
	fwScan->setFuture(f);
	(f.isStarted()) ? loadingIcon->start() : loadingIcon->stop();
	ui->btnScan->setText("");
	ui->btnScan->setEnabled(false);
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

void ScopyHomeAddPage::scanFinished(QStringList scanCtxs)
{
	ui->comboBoxContexts->clear();
	ui->uriMessageLabel->clear();
	for (QString ctx: scanCtxs) {
		ui->comboBoxContexts->addItem(ctx);
	}
	findAvailableSerialPorts();
	updateUri(ui->comboBoxContexts->currentText());
}

void ScopyHomeAddPage::deviceLoaderInitialized()
{
	QList<Plugin *> plugins = deviceImpl->plugins();
	for (Plugin *p : qAsConst(plugins)) {
		PluginEntry *pluginDescription = new PluginEntry(ui->devicePluginBrowser->getContentWidget());
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
}

void ScopyHomeAddPage::updateUri(QString uri)
{
	ui->editUri->clear();
	ui->editUri->setText(uri);
}

void ScopyHomeAddPage::createDevice()
{
	QString uri = ui->editUri->text();
	deviceImpl = DeviceFactory::build(uri, pluginManager, "");
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

#include "moc_scopyhomeaddpage.cpp"
