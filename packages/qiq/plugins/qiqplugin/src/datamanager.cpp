#include "datamanager.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DATA_MANAGER, "DataManager");

using namespace scopy::qiqplugin;

DataManager::DataManager(QObject *parent)
	: QObject(parent)
{
	m_dataReader = new DataReader(this);
	m_dataReader->openFile(DEFAULT_FILE_PATH);
	setupConnections();
}

DataManager::~DataManager() {}

void DataManager::config(QMap<int, QList<QPair<int, int>>> chnlsMap, const QStringList &chnlsFormat,
			 const int channelCount)
{
	m_chnlsMap = chnlsMap;
	m_dataReader->setChannelFormat(chnlsFormat);
	m_dataReader->setChannelCount(channelCount);
}

void DataManager::onConfigAnalysis(const QString &type, const QVariantMap &config, const OutputInfo &info)
{
	// do someting with config;
	if(m_dataReader->channelCount() == 0) {
		m_dataReader->setChannelCount(info.channelCount());
	}
	if(m_dataReader->channelFormat().isEmpty()) {
		m_dataReader->setChannelFormat(info.channelFormat());
	}
	OutputConfig outConfig;
	outConfig.setOutputFile(DEFAULT_FILE_PATH);
	outConfig.setOutputFileFormat(FileFormatTypes::BINARY_INTERLEAVED);
	outConfig.setEnabledAnalysis({type});

	Q_EMIT configOutput(outConfig);
}

void DataManager::readData(int64_t startSample, int64_t sampleCount)
{
	m_dataReader->readData(startSample, sampleCount);
}

QList<CurveData> DataManager::dataForPlot(int id) { return m_plotsData.value(id, {}); }

void DataManager::onDataReady(QMap<int, QVector<double>> &data)
{
	m_plotsData.clear();
	for(auto it = m_chnlsMap.begin(); it != m_chnlsMap.end(); ++it) {
		QList<CurveData> plotData;
		const QList<QPair<int, int>> chPairs = it.value();
		for(const QPair<int, int> &ch : chPairs) {
			CurveData curve;
			if(ch.first >= 0) {
				curve.x = data.value(ch.first, {});
			}
			curve.y = data.value(ch.second, {});
			plotData.push_back(curve);
		}
		m_plotsData.insert(it.key(), plotData);
	}
	Q_EMIT dataIsReady();
}

QString DataManager::getDefaultFilePath() const { return DEFAULT_FILE_PATH; }

void DataManager::setupConnections() { connect(m_dataReader, &DataReader::dataReady, this, &DataManager::onDataReady); }
