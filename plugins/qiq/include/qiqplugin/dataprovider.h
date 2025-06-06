#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QObject>
#include <QFile>
#include <qprocess.h>
#include <qfuturewatcher.h>

#define CHNL_NUMBER 2

namespace scopy::qiqplugin {
class DataProvider : public QObject
{
	Q_OBJECT
public:
	struct Sample
	{
		double ch1;
		double ch2;
	};

	DataProvider(QObject *parent);
	~DataProvider();

	void processData(const QString &inputFile);
	void setScriptPath(const QString &newScriptPath);

Q_SIGNALS:
	void dataReady(QVector<QVector<double>> processedData);

private:
	QFutureWatcher<void> *m_processFw;
	QFile m_file;
	int m_size;
	uchar *m_data;
	QString m_scriptPath;
	QString m_outputFile;
	static int m_uuid;

	void readProcessedData();
	void initOutputFile();
	void setupConnections();
	bool mapFile();
	void runPython(const QStringList args);
};
} // namespace scopy::qiqplugin

#endif // DATAPROVIDER_H
