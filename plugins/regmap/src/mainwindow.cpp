#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include "src/xmlfilemanager.hpp"
#include "src/deviceregistermap.hpp"
#include "src/registermapvalues.hpp"
#include "src/registermaptemplate.hpp"
#include "src/registermapinstrument.hpp"
#include <qdebug.h>
#include <iio.h>
#include <qboxlayout.h>
#include "src/readwrite/iioregisterreadstrategy.hpp"
#include "src/readwrite/iioregisterwritestrategy.hpp"
#include "src/readwrite/fileregisterwritestrategy.hpp"
#include "src/readwrite/fileregisterreadstrategy.hpp"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ctx = iio_create_context_from_uri("ip:192.168.2.1");//("ip:127.0.0.1");

	if (!ctx) {
		qDebug() << "Connection Error: No device available/connected to your PC.";
	}

	struct iio_device *dev = getIioDevice("ad9361-phy");


	IIORegisterReadStrategy *iioReadStrategy = new IIORegisterReadStrategy(dev);
	IIORegisterWriteStrategy *iioWriteStrategy = new IIORegisterWriteStrategy(dev);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(iioReadStrategy);
	registerMapValues->setWriteStrategy(iioWriteStrategy);

	//	FileRegisterReadStrategy *fileRegisterReadStrategy = new FileRegisterReadStrategy("/home/ubuntu/Documents/test.csv");
	//	FileRegisterWriteStrategy *fileRegisterWriteStrategy = new FileRegisterWriteStrategy("/home/ubuntu/Documents/test.csv");
	//	RegisterMapValues *registerMapValues = new RegisterMapValues(fileRegisterReadStrategy,fileRegisterWriteStrategy);


	RegisterMapTemplate *registerMapTemplate = new RegisterMapTemplate();

	XmlFileManager xmlFileManager(dev, "/home/ubuntu/Documents/RegisterMapDemo/ad9361-phy.xml");
	registerMapTemplate->setRegisterList(xmlFileManager.getAllRegisters());

	// each device register map will represent a tab
	DeviceRegisterMap *devRegMap = new DeviceRegisterMap(registerMapTemplate,registerMapValues);
	//	DeviceRegisterMap *regMap = new DeviceRegisterMap(nullptr,registerMapValues);

	//	QTabWidget *tabWidget = new QTabWidget;
	//	tabWidget->addTab(devRegMap, "ad9361");
	//	tabWidget->addTab(regMap, "test");
	//	tabWidget->addTab(new DeviceRegisterMap(),"All null");
	//	tabWidget->addTab(new DeviceRegisterMap(registerMapTemplate,nullptr), "template only");

	RegisterMapInstrument *regMapInstrument = new RegisterMapInstrument();

	regMapInstrument->addTab(devRegMap, "ad9361");
	regMapInstrument->addTab(dev,"iio device");
	regMapInstrument->addTab("/home/ubuntu/Documents/test.csv", "File strategy");
	regMapInstrument->addTab(new DeviceRegisterMap(registerMapTemplate,nullptr), "template only");

	QVBoxLayout *layout  = new QVBoxLayout();
	layout->addWidget(regMapInstrument);

	this->ui->centralwidget->setLayout(layout);


	//	TST_RegisterMapTest *test = new TST_RegisterMapTest();
	//	test->readFileStrategy();
}

struct iio_device* MainWindow::getIioDevice(const char *dev_name){
	auto deviceCount = iio_context_get_devices_count(ctx);

	for (int i = 0; i < deviceCount; i++) {
		iio_device *dev = iio_context_get_device(ctx, i);
		if (strcasecmp(iio_device_get_name(dev), dev_name) == 0) {
			return dev;
		}
	}
	return nullptr;
}

MainWindow::~MainWindow()
{
	delete ui;
}

