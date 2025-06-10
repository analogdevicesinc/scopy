#include "dataacquisition.h"

#include <qdebug.h>
#include <math.h>
#include <cstring>

#include <common/debugtimer.h>

using namespace scopy::qiqplugin;

DataAcquisition::DataAcquisition(QObject *parent)
	: QObject{parent}
	, m_ctx(nullptr)
	, m_mappedData(nullptr)
{
	m_ctx = m2kOpen();
	m_dataSize = SAMPLES_PER_CHANNEL * 2 * sizeof(short);
	if(m_ctx) {
		setupAnalogIn();
		setupAnalogOut();
		createOutputBuffers();
		mapFile(FILE_PATH);
	} else {
		qWarning() << "Connection Error: No ADALM2000 device available/connected to your PC.";
	}
}

DataAcquisition::~DataAcquisition()
{
	m_aout->stop();
	contextClose(m_ctx);

	if(m_mappedData) {
		m_file.unmap(m_mappedData);
	}
	m_file.remove();
}

void DataAcquisition::readDeviceData()
{
	DebugTimer timer;
	auto data = m_ain->getSamplesRawInterleaved(SAMPLES_PER_CHANNEL);
	DEBUGTIMER_LOG(timer, "getSamplesRawInterleaved:");

	writeToMappedFile(data, m_dataSize, FILE_PATH);
	Q_EMIT dataAvailable(data, m_dataSize, FILE_PATH);
}

void DataAcquisition::setupAnalogIn()
{
	m_ain = m_ctx->getAnalogIn();
	m_ctx->calibrateADC();
	m_ain->reset();
	m_ain->enableChannel(0, true);
	m_ain->enableChannel(1, true);
	m_ain->setSampleRate(10e6);
	m_ain->setRange((ANALOG_IN_CHANNEL)0, -10.0, 10.0);
	m_ain->setRange((ANALOG_IN_CHANNEL)1, PLUS_MINUS_25V);
	m_ain->getTrigger()->reset();
	m_ain->getTrigger()->setAnalogMode(0, libm2k::ALWAYS);
	m_ain->getTrigger()->setAnalogMode(1, libm2k::ALWAYS);
	qInfo() << "[AIN] Sample rates:" << m_ain->getAvailableSampleRates();
}

void DataAcquisition::setupAnalogOut()
{
	m_aout = m_ctx->getAnalogOut();
	m_ctx->calibrateDAC();
	m_aout->reset();
	m_aout->setSampleRate(0, 50e6);
	m_aout->setSampleRate(1, 50e6);
	m_aout->enableChannel(0, true);
	m_aout->enableChannel(1, true);
	qInfo() << "[AOUT] Sample rates:" << m_aout->getAvailableSampleRates(0);
}

void DataAcquisition::createOutputBuffers()
{
	std::vector<double> sinv;
	std::vector<double> saw;
	double bufferSize = 1024;

	for(int i = 0; i < bufferSize; i++) {
		double rad = 2 * M_PI * (i / bufferSize);
		double val = sin(rad);
		sinv.push_back(val);
		saw.push_back((2 * i) / bufferSize);
	}

	m_aout->setCyclic(true);
	m_aout->push({sinv, saw});
}

void DataAcquisition::mapFile(const QString &filePath)
{
	m_file.setFileName(filePath);
	if(!m_file.open(QIODevice::ReadWrite)) {
		qCritical() << "Failed to open m_file:" << m_file.errorString();
		return;
	}

	if(!m_file.resize(m_dataSize)) {
		qCritical() << "Failed to resize m_file:" << m_file.errorString();
		m_file.close();
		return;
	}

	m_mappedData = m_file.map(0, m_dataSize);
	if(!m_mappedData) {
		qCritical() << "Failed to map m_file:" << m_file.errorString();
	}
	m_file.close();
}

void DataAcquisition::writeToMappedFile(const short *data, int dataSize, const QString &filePath)
{
	if(!data) {
		qWarning() << "Empty data";
		return;
	}
	if(!m_mappedData) {
		qWarning() << "Map failed!";
		return;
	}

	size_t samples = dataSize / sizeof(short);
	short *writePtr = reinterpret_cast<short *>(m_mappedData);
	for(size_t i = 0; i < samples; ++i) {
		*writePtr++ = *data++;
	}
	m_file.close();
}
