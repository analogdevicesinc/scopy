#include <iio.h>
#include "qdebug.h"
#include "swiotmodel.hpp"
#include <core/logging_categories.h>

using namespace adiscope;

SwiotAdModel::SwiotAdModel(struct iio_channel* iioChnl):
	m_iioChnl(iioChnl)
{
	init();
}

SwiotAdModel::~SwiotAdModel()
{}

void SwiotAdModel::init()
{
	if (m_iioChnl) {
		int chnlAttrNumber = iio_channel_get_attrs_count(m_iioChnl);
		QStringList attrValues;
		for (int i = 0; i < chnlAttrNumber; i++) {
			QString attrName(iio_channel_get_attr(m_iioChnl, i));
			attrValues = readChnlAttr(m_iioChnl, attrName);
			m_chnlAttributes[attrName] = attrValues;
			attrValues.clear();
		}
	}
}

QMap<QString, QStringList> SwiotAdModel::getChnlAttrValues()
{
	return m_chnlAttributes;
}

QStringList SwiotAdModel::readChnlAttr(struct iio_channel* iio_chnl, QString attrName)
{
	QStringList attrValues;
	char* buffer = new char[200];
	std::string s_attrName = attrName.toStdString();

	int returnCode = iio_channel_attr_read(iio_chnl, s_attrName.c_str(), buffer, 199);

	if (returnCode > 0) {
		QString bufferValues(buffer);
		attrValues = bufferValues.split(" ");
	}

	delete[] buffer;
	return attrValues;
}

void SwiotAdModel::updateChnlAttributes(QMap<QString,QStringList> newValues, QString attrName)
{
	QStringList value = newValues.value(attrName);

	if (value.size() == 1) {
		QString attrVal = value.first();
		std::string s_attrValue = attrVal.toStdString();
		std::string s_attrName = attrName.toStdString();

		if (m_iioChnl != nullptr) {
			qDebug(CAT_SWIOT_RUNTIME) << attrName + " before:" + readChnlAttr(m_iioChnl,attrName).front();
			int retCode = iio_channel_attr_write(m_iioChnl,s_attrName.c_str(),s_attrValue.c_str());
			if (retCode > 0) {
				m_chnlAttributes = newValues;
			}
			qDebug(CAT_SWIOT_RUNTIME) << attrName + " after:" + readChnlAttr(m_iioChnl,attrName).front();
		}
	}

}

