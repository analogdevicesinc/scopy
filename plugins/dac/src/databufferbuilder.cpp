#include "databufferbuilder.h"
#include "filemanager.h"
#include "databuffer.h"
#include "dac_logging_categories.h"
#include "csvfilestrategy.h"
#include "databufferstrategyinterface.h"
#include "filedataguistrategy.h"
#include "dataguistrategyinterface.h"
#include <QFile>
#include <QString>

using namespace scopy;
using namespace scopy::dac;
DataBufferBuilder::DataBufferBuilder(QObject *parent)
	: QObject(parent)
	, m_dataStrategy(DS::NoDataStrategy)
	, m_guiStrategy(GuiDS::NoGuiStrategy)
	, m_widgetParent(nullptr)
	, m_filename("")
{}

DataBufferBuilder::~DataBufferBuilder() {}

DataBuffer *DataBufferBuilder::build()
{
	DataBufferStrategyInterface *ds = nullptr;
	DataGuiStrategyInterface *guids = nullptr;
	DataBuffer *widget = nullptr;
	if(m_dataStrategy == DS::NoDataStrategy) {
		qDebug(CAT_DAC_DATABUILDER) << "Not enough information provided";
		return nullptr;
	}
	ds = createDS();
	if(ds == nullptr) {
		return widget;
	}

	if(m_guiStrategy == GuiDS::NoGuiStrategy) {
		qDebug(CAT_DAC_DATABUILDER) << "Not building UI Strategy";
	} else {
		guids = createGuiGS();
	}

	widget = new DataBuffer(guids, ds, m_widgetParent);
	if(guids) {
		guids->init();
	}
	return widget;
}

DataBufferBuilder &DataBufferBuilder::dataStrategy(DataBufferBuilder::DS dataStrategy)
{
	m_dataStrategy = dataStrategy;
	return *this;
}

DataBufferBuilder &DataBufferBuilder::guiStrategy(GuiDS guiStrategy)
{
	m_guiStrategy = guiStrategy;
	return *this;
}

DataBufferBuilder &DataBufferBuilder::file(QString fullFilename)
{
	m_filename = fullFilename;
	return *this;
}

DataBufferBuilder &DataBufferBuilder::parent(QWidget *parent)
{
	m_widgetParent = parent;
	return *this;
}

DataBufferStrategyInterface *DataBufferBuilder::createDS()
{
	DataBufferStrategyInterface *ds = nullptr;
	DS tempStrategy = m_dataStrategy;
	bool fileOk = true;

	switch(tempStrategy) {
	case DS::NoDataStrategy:
		qDebug(CAT_DAC_DATABUILDER) << "Could not determine an appropriate strategy";
		break;
	case DS::FileStrategy:
		if(m_filename.endsWith(".csv")) {
			ds = new CSVFileStrategy(m_filename, m_widgetParent);
		} else {
			qDebug(CAT_DAC_DATABUILDER) << "No compatible strategy found";
		}
		break;
	case DS::CsvFileStrategy:
		if(m_filename == "") {
			qDebug(CAT_DAC_DATABUILDER) << "Provide a valid file path for CSV Strategy";
		}
		fileOk = checkFileValidity(m_filename, m_dataStrategy);
		if(fileOk) {
			ds = new CSVFileStrategy(m_filename, m_widgetParent);
		} else {
			qDebug(CAT_DAC_DATABUILDER) << "Provide a valid file path for CSV Strategy";
		}
		break;
	case DS::SinewaveData:
	default:
		qDebug(CAT_DAC_DATABUILDER) << "No valid arguments provided";
		break;
	}
	return ds;
}

DataGuiStrategyInterface *DataBufferBuilder::createGuiGS()
{
	DataGuiStrategyInterface *guids = nullptr;

	if(m_filename != "") {
		guids = new FileDataGuiStrategy(m_widgetParent);
	}
	return guids;
}

bool DataBufferBuilder::checkFileValidity(QString filepath, DataBufferBuilder::DS ds)
{
	bool valid = true;
	QFile file(filepath);
	file.open(QFile::ReadOnly);
	if(!file.isOpen()) {
		qDebug(CAT_DAC_DATABUILDER) << "Invalid file path";
		valid = false;
	} else {
		file.close();
		if(ds == DS::CsvFileStrategy) {
			valid = filepath.endsWith(".csv");
		} else if(ds == DS::MatlabFileStrategy) {
			valid = filepath.endsWith(".mat");
		}
	}
	return valid;
}
