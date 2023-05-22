#include "bufferlogic.h"
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

BufferLogic::BufferLogic(struct iio_device* iioDev) :
	m_plotChnlsNo(0)
      ,m_iioDev(iioDev)
{
	if (m_iioDev) {
		initializeChnlsScaleInfo();
		createChannels();
		m_samplingFreqAvailable = readChnlsSamplingFreqAttr("sampling_frequency_available");
	}
}

BufferLogic::~BufferLogic()
{
	if (m_chnlsInfo.size() > 0) {
		qDeleteAll(m_chnlsInfo);
		m_chnlsInfo.clear();
	}
}

void BufferLogic::createChannels()
{
	if(m_iioDev) {
		int chnlsNumber = iio_device_get_channels_count(m_iioDev);
		int plotChnlsNo = 0;
		int chnlIdx = -1;
		bool isOutput = false;
		bool isScanElement = false;
		const QRegExp rx("[^0-9]+");
		for (int i = 0; i < chnlsNumber; i++) {
			struct iio_channel* iioChnl = iio_device_get_channel(m_iioDev, i);
			struct chnlInfo* chnlInfo = new struct chnlInfo;
			QString chnlId(iio_channel_get_id(iioChnl));
			double offset = 0.0;
			double scale = 0.0;
			int erno = 0;
			const auto&& parts = chnlId.split(rx);
			chnlIdx = -1;
			iio_channel_disable(iioChnl);
			isOutput = iio_channel_is_output(iioChnl);
			erno = iio_channel_attr_read_double(iioChnl, "offset", &offset);
			iio_channel_attr_read_double(iioChnl, "scale", &scale);
			if (erno < 0) {
				scale = -1;
				offset = -1;
			}
			isScanElement = iio_channel_is_scan_element(iioChnl);
			plotChnlsNo = (!isOutput && isScanElement) ? (plotChnlsNo + 1) : plotChnlsNo;
			chnlInfo->chnlId = chnlId;
			chnlInfo->iioChnl = iioChnl;
			chnlInfo->isEnabled = false;
			chnlInfo->isScanElement = isScanElement;
			chnlInfo->isOutput = isOutput;
			chnlInfo->unitOfMeasure = m_unitsOfMeasure[chnlId[0].toLower()];
			chnlInfo->rangeValues = m_valuesRange[chnlId[0].toLower()];
			chnlInfo->offsetScalePair = {offset, scale};
			if (parts.size() > 1) {
				if(parts[1].compare("")){
					chnlIdx = parts[1].toInt();
					chnlIdx = (isOutput) ? (chnlIdx + MAX_INPUT_CHNLS_NO) : chnlIdx;
				}
			}
			m_chnlsInfo[chnlIdx] = chnlInfo;
		}
		m_plotChnlsNo = plotChnlsNo;
	}

}

void BufferLogic::initializeChnlsScaleInfo()
{
	m_unitsOfMeasure['v'] = VOLTAGE_UM;
	m_unitsOfMeasure['c'] = CURRENT_UM;
	m_unitsOfMeasure['r'] = RESISTANCE_UM;

	m_valuesRange['v'] = {-VOLTAGE_LIMIT, VOLTAGE_LIMIT};
	m_valuesRange['c'] = {-CURRENT_LIMIT, CURRENT_LIMIT};
	m_valuesRange['r'] = {RESISTANCE_LOWER_LIMIT, RESISTANCE_UPPER_LIMIT};
}

struct iio_channel* BufferLogic::getIioChnl(int chnlIdx, bool outputPriority)
{
	struct iio_channel* iioChnl = nullptr;
	if (outputPriority) {
		iioChnl = (m_chnlsInfo.contains(chnlIdx + MAX_INPUT_CHNLS_NO)) ?
					m_chnlsInfo[chnlIdx + MAX_INPUT_CHNLS_NO]->iioChnl : m_chnlsInfo[chnlIdx]->iioChnl;
	} else {
		iioChnl = m_chnlsInfo[chnlIdx]->iioChnl;
	}

	return iioChnl;
}

bool BufferLogic::verifyEnableChanges(std::vector<bool> enabledChnls)
{
	bool changes = false;
	for (int i = 0; i < enabledChnls.size(); i++) {
		if (m_chnlsInfo.contains(i)) {
			if (enabledChnls[i] != m_chnlsInfo[i]->isEnabled) {
				m_chnlsInfo[i]->isEnabled = enabledChnls[i] ;
				changes = true;
			}
		}
	}
	if (changes) {
		Q_EMIT chnlsChanged(m_chnlsInfo);
	}
	return changes;
}

void BufferLogic::onSamplingFreqChanged(int idx)
{
	std::string newSamplingFreq = m_samplingFreqAvailable[idx].toStdString();

	for (int key : m_chnlsInfo.keys()) {
		if (key > 0 && !m_chnlsInfo[key]->isOutput) {
			int returnCode = iio_channel_attr_write(m_chnlsInfo[key]->iioChnl, SAMPLING_FREQ_ATTR_NAME, newSamplingFreq.c_str());
			if (returnCode < 0) {
				qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute write error " + QString::number(returnCode);
			} else {
				QStringList newSamplingFreq = readChnlsSamplingFreqAttr(SAMPLING_FREQ_ATTR_NAME);
				int samplingFreq = newSamplingFreq[0].toInt();
				Q_EMIT samplingFreqWritten(samplingFreq);
				break;
			}
		}
	}
}

QStringList BufferLogic::readChnlsSamplingFreqAttr(QString attrName)
{
	QStringList attrValues;
	char* buffer = new char[200];
	std::string s_attrName = attrName.toStdString();

	for (int key : m_chnlsInfo.keys()) {
		if (key > 0 && !m_chnlsInfo[key]->isOutput) {
			int returnCode = iio_channel_attr_read(m_chnlsInfo[key]->iioChnl, s_attrName.c_str(), buffer, 199);
			if (returnCode > 0) {
				QString bufferValues(buffer);
				attrValues = bufferValues.split(" ");
				break;
			} else {
				qDebug(CAT_SWIOT_AD74413R) << "Chnl attribute read error " + QString::number(returnCode);
			}
		}
	}
	attrValues.removeAll("");
	delete[] buffer;
	return attrValues;
}

int BufferLogic::getPlotChnlsNo()
{
	return m_plotChnlsNo;
}

QVector<QString> BufferLogic::getChnlsUnitOfMeasure()
{
	QVector<QString> chnlsUnitOfMeasure;

	for (struct chnlInfo* chnl : m_chnlsInfo) {
		if (chnl->isScanElement && !chnl->isOutput) {
			QString unitOfMeasure = chnl->unitOfMeasure;
			chnlsUnitOfMeasure.push_back(unitOfMeasure);
		}
	}
	return chnlsUnitOfMeasure;
}

QVector<std::pair<int, int>> BufferLogic::getChnlsRangeValues()
{
	QVector<std::pair<int, int>> chnlsRangeValues;
	for (struct chnlInfo* chnl : m_chnlsInfo) {
		if (chnl->isScanElement && !chnl->isOutput) {
			std::pair<int, int> rangeValues = chnl->rangeValues;
			chnlsRangeValues.push_back(rangeValues);
		}
	}
	return chnlsRangeValues;
}
