#include "dataacquisition.h"

#include <qdebug.h>
#include <math.h>
#include <cstring>

using namespace scopy::qiqplugin;

DataAcquisition::DataAcquisition(QObject *parent)
	: QObject{parent}
	, m_ctx(nullptr)
	, m_mappedData(nullptr)
{
	m_ctx = m2kOpen();
	if(m_ctx) {
		setupAnalogIn();
		setupAnalogOut();
		createOutputBuffers();
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
	int samplesPerChannel = SAMPLES_PER_CHANNEL;
	int dataSize = samplesPerChannel * 2 * sizeof(double);
	auto data = m_ain->getSamplesInterleaved(samplesPerChannel);

	writeToMappedFile(data, dataSize, FILE_PATH);
	Q_EMIT dataAvailable(data, dataSize, FILE_PATH);
}

void DataAcquisition::setupAnalogIn()
{
	m_ain = m_ctx->getAnalogIn();
	m_ctx->calibrateADC();
	m_ain->reset();
	m_ain->enableChannel(0, true);
	m_ain->enableChannel(1, true);
	m_ain->setSampleRate(100000);
	m_ain->setRange((ANALOG_IN_CHANNEL)0, -10.0, 10.0);
	m_ain->setRange((ANALOG_IN_CHANNEL)1, PLUS_MINUS_25V);
}

void DataAcquisition::setupAnalogOut()
{
	m_aout = m_ctx->getAnalogOut();
	m_ctx->calibrateDAC();
	m_aout->reset();
	m_aout->setSampleRate(0, 750000);
	m_aout->setSampleRate(1, 750000);
	m_aout->enableChannel(0, true);
	m_aout->enableChannel(1, true);
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

void DataAcquisition::writeToMappedFile(const double *data, int dataSize, const QString &filePath)
{
	if(!data) {
		qWarning() << "Empty data";
		return;
	}

	m_file.setFileName(filePath);
	if(!m_file.open(QIODevice::ReadWrite)) {
		qCritical() << "Failed to open m_file:" << m_file.errorString();
		return;
	}

	if(!m_file.resize(dataSize)) {
		qCritical() << "Failed to resize m_file:" << m_file.errorString();
		m_file.close();
		return;
	}

	m_mappedData = m_file.map(0, dataSize);
	if(!m_mappedData) {
		qCritical() << "Failed to map m_file:" << m_file.errorString();
		m_file.close();
		return;
	}

	size_t samples = dataSize / sizeof(double);
	double *writePtr = reinterpret_cast<double *>(m_mappedData);
	for(size_t i = 0; i < samples; ++i) {
		*writePtr++ = *data++;
	}
	m_file.close();
}
