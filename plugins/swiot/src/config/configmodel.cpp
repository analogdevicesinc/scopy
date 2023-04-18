#include "configmodel.hpp"
#include "qdebug.h"
#include <iio.h>
#include <QMap>
#include "src/swiot_logging_categories.h"

using namespace adiscope::swiot;

ConfigModel::ConfigModel(struct iio_channel* adChannel, struct iio_channel* maxChannel)
{
	m_channels.push_back(adChannel);
	m_channels.push_back(maxChannel);

	initChnlAttrValues(adChannel);
	initChnlAttrValues(maxChannel);

}

ConfigModel::~ConfigModel()
{}

void ConfigModel::initChnlAttrValues(struct iio_channel* iioChnl)
{
	if (iioChnl) {
		QMap<QString, QStringList> chnlAttr;
		int chnlAttrNumber = iio_channel_get_attrs_count(iioChnl);
		QStringList attrValues;
		for (int i = 0; i < chnlAttrNumber; i++) {
			QString attrName(iio_channel_get_attr(iioChnl, i));
			attrValues = readChnlAttr(iioChnl, attrName);
			chnlAttr[attrName] = attrValues;
			attrValues.clear();
		}
		m_attrValues.push_back(chnlAttr);

	}
}

QStringList ConfigModel::readChnlAttr(struct iio_channel* iio_chnl, QString attrName)
{
	QStringList attrValues;
	char* buffer = new char[500];
	std::string s_attrName = attrName.toStdString();

	int returnCode = iio_channel_attr_read(iio_chnl, s_attrName.c_str(), buffer, 499);

	if (returnCode > 0) {
		QString bufferValues(buffer);
		attrValues = bufferValues.split(" ");
	}

	delete[] buffer;
	return attrValues;
}

void ConfigModel::updateChnlAttributes(QVector<QMap<QString,QStringList>> newValues, QString attrName,
				       int deviceIdx)
{
	QStringList value = newValues[deviceIdx].value(attrName);
	if (value.size() == 1) {
		QString attrVal = value.first();
		std::string s_attrValue = attrVal.toStdString();
		std::string s_attrName = attrName.toStdString();

		if (m_channels[deviceIdx]) {
			qDebug(CAT_SWIOT_CONFIG) <<QString::number(deviceIdx) + attrName + " before:" + readChnlAttr(m_channels[deviceIdx],attrName).front();
			ssize_t retCode = iio_channel_attr_write(m_channels[deviceIdx],s_attrName.c_str(),s_attrValue.c_str());
			if (retCode >= 0) {
				m_attrValues[deviceIdx] = newValues[deviceIdx];
			}
			qDebug(CAT_SWIOT_CONFIG) << QString::number(deviceIdx) + attrName + " after:" + readChnlAttr(m_channels[deviceIdx],attrName).front();
		}
	}
}

QVector<QMap<QString, QStringList>> ConfigModel::getChnlsAttrValues()
{
	return m_attrValues;
}

QStringList ConfigModel::getActiveFunctions()
{
	QStringList func;
	QStringList funcAttrNames = {"function_cfg", "function"};
	if (m_attrValues.size() == funcAttrNames.size()) {
		for (int i = 0; i < m_attrValues.size(); i++) {
			func.push_back(m_attrValues[i][funcAttrNames[i]].first());
		}
	}
	return func;
}
