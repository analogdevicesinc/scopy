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
	bool serialBackEnd = iio_has_backend("serial");
	bool hasLibSerialPort = libSerialSupport->hasLibSerialSupport();

	ui->setupUi(this);
	initAddPage();
	initSubSections();
	verifyIioBackend();
	pendingUri = "";

	//verify
	fw = new QFutureWatcher<bool>(this);
	connect(fw,&QFutureWatcher<bool>::finished,this,[=](){
		bool result = fw->result();
		if (result == true) {
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
		ui->scanSection->setVisible(true);
		ui->scrollAreaDevInfo->setVisible(false);
		ui->btnVerify->setVisible(true);
		ui->btnAdd->setVisible(false);
		ui->btnBack->setVisible(false);
	});
	connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(futureScan()));
	connect(ui->btnVerify, SIGNAL(clicked()), this, SLOT(futureverify()));
	connect(ui->btnAdd, &QPushButton::clicked, this, [=](){
		ui->btnAdd->setVisible(false);
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

	//radio btns connections
	connect(ui->radioBtnDiscover, &QRadioButton::toggled, this, [=](bool checked){
		ui->radioBtnManual->setChecked(!checked);
		ui->autoDetectionSection->setEnabled(checked);
		ui->serialSettingsWidget->setEnabled(serialBackEnd && hasLibSerialPort);
		ui->btnScan->setEnabled(checked);
	});
	connect(ui->radioBtnManual, &QRadioButton::toggled, this, [=](bool checked){
		ui->radioBtnDiscover->setChecked(!checked);
		ui->editUri->clear();
		ui->uriMessageLabel->clear();
		ui->uriWidget->setEnabled(checked);
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
	QRegExp re("[5-9]{1}(n|o|e|m|s){1}[1-2]{1}(x|r|d){1}$");
	QRegExpValidator *validator = new QRegExpValidator(re, this);

	ui->editSerialFrameConfig->setValidator(validator);
	ui->radioBtnManual->setChecked(true);
	ui->autoDetectionSection->setEnabled(false);
	ui->btnAdd->setVisible(false);
	ui->btnScan->setEnabled(false);
	ui->scrollAreaDevInfo->setVisible(false);
	ui->btnBack->setVisible(false);
	ui->serialSettingsWidget->setEnabled(hasLibSerialPort);
	for (int baudRate : availableBaudRates) {
		ui->comboBoxBaudRate->addItem(QString::number(baudRate));
	}
	ui->btnAdd->setProperty("blue_button", QVariant(true));
	ui->btnVerify->setProperty("blue_button", QVariant(true));
	ui->btnScan->setProperty("blue_button", QVariant(true));
	ui->btnBack->setProperty("blue_button", QVariant(true));
}

void ScopyHomeAddPage::initSubSections()
{
	ui->subSeparatorPlugins->setLabel("Compatible plugins");
	ui->subSeparatorPlugins->getContentWidget()->layout()->setSpacing(10);

	ui->subSeparatorDevInfo->setLabel("Context info");
	deviceInfoPage = new InfoPage(ui->subSeparatorDevInfo->getContentWidget());
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
		//there is another handle process for serial and xml backends
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
			ui->filterWidget->layout()->addWidget(cb);
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
	PluginManager *p = new PluginManager(this);
	struct iio_context* ctx = ContextProvider::GetInstance()->open(uri);
	if(ctx) {
		QList<Plugin*> plugins = p->getCompatiblePlugins(uri);
		getContextData(ctx);
		ContextProvider::GetInstance()->close(uri);
		return true;
	}	
	return false;

}

void ScopyHomeAddPage::futureverify()
{
	ui->btnAdd->setVisible(false);
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
	ui->btnBack->clicked(true);
	if(!pendingUri.isEmpty()) {
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
		QCheckBox *cb = new QCheckBox();
		cb->setText(p->name());
		cb->setChecked(p->enabled());
		ui->subSeparatorPlugins->getContentWidget()->layout()->addWidget(cb);
		connect(cb, &QCheckBox::toggled, this, [=](bool en){
			p->setEnabled(en);
		});
	}
	ui->btnVerify->setVisible(false);
	ui->scanSection->setVisible((false));
	ui->btnAdd->setVisible(true);
	ui->btnBack->setVisible(true);
	ui->scrollAreaDevInfo->setVisible(true);

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
