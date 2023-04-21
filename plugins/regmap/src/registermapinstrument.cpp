#include "registermapinstrument.hpp"
#include "readwrite/iioregisterreadstrategy.hpp"
#include "readwrite/iioregisterwritestrategy.hpp"
#include "readwrite/fileregisterwritestrategy.hpp"
#include "readwrite/fileregisterreadstrategy.hpp"
#include "deviceregistermap.hpp"
#include "registermaptemplate.hpp"
#include "registermapvalues.hpp"
#include <QTabWidget>
#include <qboxlayout.h>
#include <iio.h>

RegisterMapInstrument::RegisterMapInstrument(QWidget *parent)
	: QWidget{parent}
{
	layout = new QVBoxLayout();
	this->setLayout(layout);
	tabWidget = new QTabWidget();

	layout->addWidget(tabWidget);
}

RegisterMapInstrument::~RegisterMapInstrument()
{
	delete tabWidget;
}

void RegisterMapInstrument::addTab(QWidget *widget, QString title)
{
	tabWidget->addTab(widget, title);
}

void RegisterMapInstrument::addTab(iio_device *dev, QString title)
{
	IIORegisterReadStrategy *iioReadStrategy = new IIORegisterReadStrategy(dev);
	IIORegisterWriteStrategy *iioWriteStrategy = new IIORegisterWriteStrategy(dev);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(iioReadStrategy);
	registerMapValues->setWriteStrategy(iioWriteStrategy);
	DeviceRegisterMap *regMap = new DeviceRegisterMap(nullptr,registerMapValues);

	tabWidget->addTab(regMap, title);
}

void RegisterMapInstrument::addTab(QString filePath, QString title)
{
	FileRegisterReadStrategy *fileRegisterReadStrategy = new FileRegisterReadStrategy(filePath);
	FileRegisterWriteStrategy *fileRegisterWriteStrategy = new FileRegisterWriteStrategy(filePath);
	RegisterMapValues *registerMapValues = new RegisterMapValues();
	registerMapValues->setReadStrategy(fileRegisterReadStrategy);
	registerMapValues->setWriteStrategy(fileRegisterWriteStrategy);
	DeviceRegisterMap *regMap = new DeviceRegisterMap(nullptr,registerMapValues);

	tabWidget->addTab(regMap, title);
}
