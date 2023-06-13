#include "scopyhomeaddpage.h"

#include "devicefactory.h"
#include "deviceloader.h"
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

	//verify
	fw = new QFutureWatcher<bool>(this);
	connect(fw,&QFutureWatcher<bool>::finished,this,[=](){
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

	//btns connections
	connect(ui->btnBack, &QPushButton::clicked, this, [=](){
		ui->stackedWidget->setCurrentWidget(ui->deviceDetectionPage);
	});
	connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(futureScan()));
	connect(ui->btnVerify, SIGNAL(clicked()), this, SLOT(futureverify()));
	connect(ui->btnAdd, &QPushButton::clicked, this, [=](){
		pendingUri = ui->editUri->text();
		Q_EMIT newDeviceAvailable(deviceImpl);
	});


	connect(ui->comboBoxContexts,&QComboBox::textActivated, [=](){
		ui->editUri->clear();
		ui->editUri->setText(ui->comboBoxContexts->currentText());
	});

	//serial widget connections
	connect(ui->comboBoxSerialPort, &QComboBox::textActivated, [=](){
		ui->editUri->clear();
		ui->editUri->setText(getSerialPath());
	});
	connect(ui->comboBoxBaudRate, &QComboBox::textActivated, [=](QString br){
		ui->editUri->clear();
		ui->editUri->setText(getSerialPath());
	});
	connect(ui->editSerialFrameConfig, &QLineEdit::textChanged, [=](QString config){
		ui->editUri->clear();
		ui->editUri->setText(getSerialPath());
	});
	connect(ui->editUri, &QLineEdit::textChanged, [=](QString uri){
		if (uri.isEmpty()) {
			ui->btnVerify->setEnabled(false);
		} else {
			ui->btnVerify->setEnabled(true);
		}
	});

	connect(this, &ScopyHomeAddPage::contextDataAvailable, this, [=](QMap<QString, QString> ctxInfo){
		for (QString key : ctxInfo.keys()) {
			deviceInfoPage->update(key, ctxInfo[key]);
		}
	},Qt::QueuedConnection);
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
	if (deviceImpl) {
		delete deviceImpl;
	}
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
	ui->subSeparatorPlugins->setLabel("Compatible plugins");
	ui->subSeparatorPlugins->getContentWidget()->layout()->setSpacing(10);

	ui->subSeparatorDevInfo->setLabel("Device info");
	deviceInfoPage = new InfoPage(ui->subSeparatorDevInfo->getContentWidget());
	deviceInfoPage->setAdvancedMode(false);
	deviceInfoPage->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	ui->subSeparatorDevInfo->getContentWidget()->layout()->addWidget(deviceInfoPage);
	QSpacerItem *vSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	ui->subSeparatorDevInfo->getContentWidget()->layout()->addItem(vSpacer);
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
		if (backEnd.compare("xml")) {
			QCheckBox *cb = new QCheckBox();
			cb->setText(backEnd);
			if (backEnd.compare("serial") == 0) {
				connect(cb, &QCheckBox::toggled, this, [=](bool en) {
					if (en) {
						scanParamsList.push_back(backEnd + ":");
					} else {
						scanParamsList.removeOne(backEnd + ":");
					}
					ui->serialSettingsWidget->setEnabled(en);
				});
			} else {
				connect(cb, &QCheckBox::toggled, this, [=](bool en) {
					if (en) {
						scanParamsList.push_back(backEnd + ":");
					} else {
						scanParamsList.removeOne(backEnd + ":");
					}
				});
			}
			cb->setChecked(true);
			ui->filterCheckBoxes->layout()->addWidget(cb);
		}
		scan = true;
	}
	ui->btnScan->setVisible(scan);
}

void ScopyHomeAddPage::getContextData(struct iio_context *ctx)
{
	QMap <QString, QString> contextInfo;
	for(int i=0;i<iio_context_get_attrs_count(ctx);i++) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(ctx,i,&name,&value);
		if(ret != 0)
			continue;
		contextInfo[name] = value;
	}
	Q_EMIT contextDataAvailable(contextInfo);
}

bool ScopyHomeAddPage::verify() {
	QString uri = ui->editUri->text();
	struct iio_context* ctx = ContextProvider::GetInstance()->open(uri);
	if(ctx) {
		getContextData(ctx);
		ContextProvider::GetInstance()->close(uri);
		return true;
	}	
	return false;
}

void ScopyHomeAddPage::futureverify()
{
	removePluginsCheckBoxs();
	deviceInfoPage->clear();
	QFuture<bool> f = QtConcurrent::run(std::bind(&ScopyHomeAddPage::verify,this));
	fw->setFuture(f);
}

void ScopyHomeAddPage::futureScan()
{
	QString scanParams = scanParamsList.join("");
	QtConcurrent::run([=](){
		scanTask->setScanParams(scanParams);
		scanTask->run();
	});
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
	ui->editUri->clear();
	ui->editUri->setText(ui->comboBoxContexts->currentText());
}

void ScopyHomeAddPage::deviceLoaderInitialized()
{
	QList<Plugin *> plugins = deviceImpl->plugins();
	for (Plugin *p : qAsConst(plugins)) {
		QWidget *pluginInfo = new QWidget();
		QHBoxLayout *hLayout = new QHBoxLayout(pluginInfo);

		QCheckBox *cb = new QCheckBox();
		cb->setText(p->name());
		cb->setChecked(p->enabled());
		connect(cb, &QCheckBox::toggled, this, [=](bool en){
			p->setEnabled(en);
		});

		QLabel *descriptionLabel = new QLabel();
		descriptionLabel->setWordWrap(true);
		descriptionLabel->setText(p->description());

		pluginInfo->setStyleSheet("background-color: rgb(0, 0, 0);");
		hLayout->setContentsMargins(5, 5, 5, 5);
		hLayout->addWidget(cb);
		hLayout->addWidget(descriptionLabel);
		hLayout->setStretch(0,1);
		hLayout->setStretch(1,3);
		ui->subSeparatorPlugins->getContentWidget()->layout()->addWidget(pluginInfo);
	}
	ui->stackedWidget->setCurrentWidget(ui->deviceInfoPage);
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

void ScopyHomeAddPage::removePluginsCheckBoxs()
{
	if (ui->subSeparatorPlugins->getContentWidget()->layout()) {
		QLayoutItem* item;
		while ((item = ui->subSeparatorPlugins->getContentWidget()->layout()->takeAt(0)) != nullptr) {
			delete item->widget();
			delete item;
		}
	}
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
