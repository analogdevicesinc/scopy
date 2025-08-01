#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "qiqutils.h"
#include <QMap>
#include <QObject>
#include <datareader.h>
#include <outputconfig.h>
#include <common/scopyconfig.h>
#include <QDir>
#include <outputinfo.h>

namespace scopy::qiqplugin {

class DataManager : public QObject
{
	Q_OBJECT
public:
	DataManager(QObject *parent = nullptr);
	~DataManager();

	void config(const QStringList &chnlsName, const QStringList &chnlsFormat, const int channelCount);
	void onConfigAnalysis(const QString &type, const QVariantMap &config, const OutputInfo &info);
	void readData(int64_t startSample, int64_t sampleCount);

	void setSamplingFreq(int samplingFreq);
	QVector<double> dataForKey(const QString &key);
	QString getDefaultFilePath() const;

Q_SIGNALS:
	void configOutput(const OutputConfig &outConfig);
	void dataIsReady();

public Q_SLOTS:
	void onInputData(QVector<QVector<double>> bufferData);

private Q_SLOTS:
	void onDataReady(QMap<QString, QVector<double>> &data);

private:
	void setupConnections();
	void computeXTime(int samplingFreq, int samples);
	void computeXFreq(int samplingFreq, int samples);

	int m_samplingFreq;
	int m_sampleCount;
	DataReader *m_dataReader;
	QMap<QString, QVector<double>> m_plotsData;

	const QString DEFAULT_FILE_PATH = scopy::config::executableFolderPath() + QDir::separator() + "data.out";
};

} // namespace scopy::qiqplugin

#endif // DATAMANAGER_H
