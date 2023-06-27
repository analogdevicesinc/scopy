#include "chnlinfo.h"
#include <math.h>

using namespace scopy::swiot;

ChnlInfo::ChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl) :
	m_iioChnl(iioChnl),
	m_plotUm(plotUm), m_hwUm(hwUm)
{
	if (m_iioChnl) {
		iio_channel_disable(iioChnl);
		m_chnlId = QString(iio_channel_get_id(m_iioChnl));
		double offset = 0.0;
		double scale = 0.0;
		int erno = 0;
		m_isOutput = iio_channel_is_output(iioChnl);
		erno = iio_channel_attr_read_double(iioChnl, "offset", &offset);
		iio_channel_attr_read_double(iioChnl, "scale", &scale);
		if (erno < 0) {
			scale = 1;
			offset = 0;
		}
		m_isScanElement = iio_channel_is_scan_element(iioChnl);
		m_isEnabled = false;
		m_offsetScalePair = {offset, scale};
		initUnitOfMeasureFactor();
	}
}

iio_channel *ChnlInfo::iioChnl() const
{
	return m_iioChnl;
}

bool ChnlInfo::isOutput() const
{
	return m_isOutput;
}

bool ChnlInfo::isScanElement() const
{
	return m_isScanElement;
}

QString ChnlInfo::chnlId() const
{
	return m_chnlId;
}

std::pair<int, int> ChnlInfo::rangeValues() const
{
	return m_rangeValues;
}

std::pair<double, double> ChnlInfo::offsetScalePair() const
{
	return m_offsetScalePair;
}

bool ChnlInfo::isEnabled() const
{
	return m_isEnabled;
}

void ChnlInfo::setIsEnabled(bool newIsEnabled)
{
	m_isEnabled = newIsEnabled;
}

QString ChnlInfo::unitOfMeasure() const
{
	return m_plotUm;
}

void ChnlInfo::initUnitOfMeasureFactor()
{
	m_unitOfMeasureFactor["G"] = pow(10, 9);
	m_unitOfMeasureFactor["M"] = pow(10, 6);
	m_unitOfMeasureFactor["K"] = pow(10, 3);
	m_unitOfMeasureFactor["m"] = pow(10, -3);
	m_unitOfMeasureFactor["u"] = pow(10, -6);
	m_unitOfMeasureFactor["µ"] = pow(10, -6);
	m_unitOfMeasureFactor["n"] = pow(10, -9);
	m_unitOfMeasureFactor["p"] = pow(10, -12);
}
