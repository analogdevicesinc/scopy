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
	pluginManager(pm), deviceImpl(nullptr)
{
	ui->setupUi(this);
	initAddPage();
	initInfoWidget();
	verifyIioBackend();

	fw = new QFutureWatcher<bool>(this);
	connect(fw,&QFutureWatcher<bool>::finished,this,[=](){
		bool result = fw->result();
		ui->contextInfoLabel->setVisible(result);
		ui->editPlugins->setVisible(result);
		if(fw->result()==true) {
			createDevice();
		}
	});

	scanTask = new IIOScanTask(this);
	connect(scanTask, SIGNAL(scanFinished(QStringList)), this, SLOT(scanFinished(QStringList)));

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

	connect(ui->radioBtnDiscover, &QRadioButton::toggled, this, [=](bool checked){
		ui->radioBtnManual->setChecked(!checked);
		ui->filterWidget->setEnabled(checked);
		ui->discoveredContextsWidget->setEnabled(checked);
		ui->btnScan->setEnabled(checked);
	});
	connect(ui->radioBtnManual, &QRadioButton::toggled, this, [=](bool checked){
		ui->editUri->clear();
		ui->uriWidget->setEnabled(checked);
	});

	connect(this, &ScopyHomeAddPage::contextDataAvailable, this, [=](QMap<QString, QString> ctxInfo){
		for (QString key : ctxInfo.keys()) {
			deviceInfoPage->update(key, ctxInfo[key]);
		}
	},Qt::QueuedConnection);

	pendingUri = "";
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
	if (deviceImpl) {
		delete deviceImpl;
	}
}

void ScopyHomeAddPage::initAddPage()
{
	ui->btnAdd->setProperty("blue_button", QVariant(true));
	ui->btnVerify->setProperty("blue_button", QVariant(true));
	ui->btnScan->setProperty("blue_button", QVariant(true));
	ui->radioBtnManual->setChecked(true);
	ui->filterWidget->setEnabled(false);
	ui->discoveredContextsWidget->setEnabled(false);
	ui->btnAdd->setVisible(false);
	ui->btnScan->setEnabled(false);
	ui->contextInfoLabel->setVisible(false);
	ui->editPlugins->setVisible(false);
}

void ScopyHomeAddPage::initInfoWidget()
{
	QVBoxLayout *layout = new QVBoxLayout(ui->infoWidget);
	deviceInfoPage = new InfoPage(ui->infoWidget);
	deviceInfoPage->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	layout->addWidget(deviceInfoPage);
	QSpacerItem *vSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	layout->addItem(vSpacer);
}

void ScopyHomeAddPage::verifyIioBackend()
{
	bool scan = false;
	int backEndsCount = iio_get_backends_count();
	for (int i = 0; i < backEndsCount; i++) {
		QString backEnd(iio_get_backend(i));
		//there is another handle process for serial and xml backends
		if (backEnd.compare("serial") && backEnd.compare("xml")) {
			QCheckBox *cb = new QCheckBox();
			cb->setText(backEnd);
			connect(cb, &QCheckBox::toggled, this, [=](bool en) {
				if (en) {
					scanParamsList.push_back(backEnd + ":");
				} else {
					scanParamsList.removeOne(backEnd + ":");
				}
			});
			ui->filterWidget->layout()->addWidget(cb);
		}
		scan = true;
	}

	if (iio_has_backend("serial")) {
		scan = true;
	} else {
		ui->serialContextSection->setEnabled(false);
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
	if(!pendingUri.isEmpty()) {
		Q_EMIT requestDevice(id);
		pendingUri = "";
	}
}

void ScopyHomeAddPage::scanFinished(QStringList scanCtxs)
{
	ui->comboBoxContexts->clear();
	for (QString ctx: scanCtxs) {
		ui->comboBoxContexts->addItem(ctx);
	}
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
		ui->pluginsWidget->layout()->addWidget(cb);
		connect(cb, &QCheckBox::toggled, this, [=](bool en){
			p->setEnabled(en);
		});
	}
	QSpacerItem *vSpacer = new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	ui->pluginsWidget->layout()->addItem(vSpacer);
	ui->btnAdd->setVisible(true);
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
	if (ui->pluginsWidget->layout()) {
		QLayoutItem* item;
		while ((item = ui->pluginsWidget->layout()->takeAt(0)) != nullptr) {
			delete item->widget();
			delete item;
		}
	}
}

#include "moc_scopyhomeaddpage.cpp"
