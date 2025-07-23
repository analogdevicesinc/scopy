#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "qiqutils.h"
#include <QMap>
#include <QObject>
#include <datareader.h>
#include <common/scopyconfig.h>
#include <QDir>

namespace scopy::qiqplugin {

class DataManager : public QObject
{
	Q_OBJECT
public:
	DataManager(QObject *parent = nullptr);
	~DataManager();

	void config(QMap<int, QList<QPair<int, int>>> chnlsMap, const QStringList &chnlsFormat, const int channelCount);
	void readData(int64_t startSample, int64_t sampleCount);
	QList<CurveData> dataForPlot(int id);

	QString getDefaultFilePath() const;

Q_SIGNALS:
	void dataIsReady();

private Q_SLOTS:
	void onDataReady(QMap<int, QVector<double>> &data);

private:
	void setupConnections();

	DataReader *m_dataReader;
	QMap<int, QList<QPair<int, int>>> m_chnlsMap;
	QMap<int, QList<CurveData>> m_plotsData;

	const QString DEFAULT_FILE_PATH = scopy::config::executableFolderPath() + QDir::separator() + "data.out";
};

} // namespace scopy::qiqplugin

#endif // DATAMANAGER_H
