#include "ad74413r/chnlinfo.h"

#include <iioutil/iiocommand/iiochannelattributeread.h>
#include <math.h>

using namespace scopy::swiotrefactor;

ChnlInfo::ChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue)
	: m_iioChnl(iioChnl)
	, m_plotUm(plotUm)
	, m_hwUm(hwUm)
	, m_commandQueue(cmdQueue)
{
	m_offsetScalePair = {0, 1};
	if(m_iioChnl) {
		iio_channel_disable(iioChnl);
		m_chnlId = QString(iio_channel_get_id(m_iioChnl));
		double offset = 0.0;
		double scale = 0.0;
		int erno = 0;
		m_isOutput = iio_channel_is_output(iioChnl);

		addReadScaleCommand();
		addReadOffsetCommand();

		m_isScanElement = iio_channel_is_scan_element(iioChnl);
		m_isEnabled = false;

		initUnitOfMeasureFactor();
	}
}

ChnlInfo::~ChnlInfo()
{
	if(m_commandQueue) {
		m_commandQueue = nullptr;
	}
}

void ChnlInfo::addReadScaleCommand()
{
	Command *attrReadScale = new IioChannelAttributeRead(m_iioChnl, "scale", nullptr);
	connect(attrReadScale, &scopy::Command::finished, this, &ChnlInfo::readScaleCommandFinished,
		Qt::QueuedConnection);
	m_commandQueue->enqueue(attrReadScale);
}

void ChnlInfo::addReadOffsetCommand()
{
	Command *attrReadOffset = new IioChannelAttributeRead(m_iioChnl, "offset", nullptr);
	connect(attrReadOffset, &scopy::Command::finished, this, &ChnlInfo::readOffsetCommandFinished,
		Qt::QueuedConnection);
	m_commandQueue->enqueue(attrReadOffset);
}

void ChnlInfo::readScaleCommandFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *res = tcmd->getResult();
		bool ok = false;
		double scale = QString(res).toDouble(&ok);
		if(ok) {
			m_offsetScalePair.second = scale;
		}
	} else {
		//		qDebug(CAT_SWIOT) << "Error, could not read \"scale\" attribute for channel.";
	}
}

void ChnlInfo::readOffsetCommandFinished(Command *cmd)
{
	IioChannelAttributeRead *tcmd = dynamic_cast<IioChannelAttributeRead *>(cmd);
	if(!tcmd) {
		return;
	}
	if(tcmd->getReturnCode() >= 0) {
		char *res = tcmd->getResult();
		bool ok = false;
		double offset = QString(res).toDouble(&ok);
		if(ok) {
			m_offsetScalePair.first = offset;
		}
	} else {
		//		qDebug(CAT_SWIOT) << "Error, could not read \"offset\" attribute from channel";
	}
}

iio_channel *ChnlInfo::iioChnl() const { return m_iioChnl; }

bool ChnlInfo::isOutput() const { return m_isOutput; }

bool ChnlInfo::isScanElement() const { return m_isScanElement; }

QString ChnlInfo::chnlId() const { return m_chnlId; }

std::pair<int, int> ChnlInfo::rangeValues() const { return m_rangeValues; }

std::pair<double, double> ChnlInfo::offsetScalePair() const { return m_offsetScalePair; }

bool ChnlInfo::isEnabled() const { return m_isEnabled; }

void ChnlInfo::setIsEnabled(bool newIsEnabled) { m_isEnabled = newIsEnabled; }

QString ChnlInfo::unitOfMeasure() const { return m_plotUm; }

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
