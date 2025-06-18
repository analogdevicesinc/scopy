#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include "common/debugtimer.h"
#include <QObject>
#include <QFile>
#include <qprocess.h>
#include <qfuturewatcher.h>
#include <QDir>
#include <common/scopyconfig.h>

#define CHNLS 1

namespace scopy::qiqplugin {
class DataProvider : public QObject
{
	Q_OBJECT
public:
	DataProvider(QObject *parent);
	~DataProvider();

	void processData(const QString &inputFile);
	void setScriptPath(const QString &newScriptPath);
	void runProcess(int chnls);

	void setCliPath(const QString &newCliPath);

Q_SIGNALS:
	void dataReady(QVector<QVector<double>> processedData);
	void stopAcq();

private:
	QFutureWatcher<void> *m_processFw;
	QFile m_file;
	int m_chnls;
	int m_size;
	uchar *m_data;
	QString m_scriptPath;
	QString m_outputFile;
	static int m_uuid;
	DebugTimer m_debugTimer = DebugTimer(scopy::config::settingsFolderPath() + QDir::separator() + "benchmark.csv");
	QString m_cliPath = scopy::config::executableFolderPath() + QDir::separator() + "process_data";
	QProcess *m_cliProcess;

	void readProcessedData();
	void initOutputFile();
	void setupConnections();
	bool mapFile();
	void runPython(const QStringList args);
};
} // namespace scopy::qiqplugin

#endif // DATAPROVIDER_H
