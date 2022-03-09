#ifndef DataLoggerModel_H
#define DataLoggerModel_H

#include <QObject>
#include <qvector.h>
#include <QTimer>
#include <QMap>
#include <filemanager.h>
#include <QDateTime>
#include <QtConcurrent>

namespace adiscope {

enum class CHANNEL_DATA_TYPE{
	INT,
	DOUBLE,
	QSTRING
};

enum class CHANNEL_FILTER{
	LAST_VALUE,
	AVERAGE,
	ALL
};

struct Channel{
	CHANNEL_DATA_TYPE type;
	CHANNEL_FILTER filter;
	QVector<QString> values;
};

class DataLoggerModel: public QObject
{
	Q_OBJECT

public:
	DataLoggerModel();
	~DataLoggerModel();
	void createChannel(QString name, CHANNEL_DATA_TYPE type, CHANNEL_FILTER filter);
	void destroyChannel(QString name);
	void resetChannel(QString name);
	void logData();

public Q_SLOTS:
	void receiveValue(QString name, QString value);
	void setPath(QString path);
	void setTimerInterval(double interval);
	void startLogger(bool overwrite);
	void stopLogger();

private:
	QString m_path;
	QTimer  *m_timer;
	QMap<QString,Channel> m_channels;
	const QString SEPARATOR = ",";

	double computeAvg(QVector<QString> values);
	bool isNumber(const QString &str);
};
}
#endif // DataLoggerModel_H
