#ifndef DataLoggerModel_H
#define DataLoggerModel_H

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QtConcurrent>
#include <qvector.h>

namespace scopy {
namespace datalogger {

enum class CHANNEL_DATA_TYPE
{
	INT,
	DOUBLE,
	QSTRING
};

enum class CHANNEL_FILTER
{
	LAST_VALUE,
	AVERAGE,
	ALL
};

struct Channel
{
	CHANNEL_DATA_TYPE type;
	CHANNEL_FILTER filter;
	QVector<QString> values;
};

class DataLoggerModel : public QObject
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
	QTimer *m_timer;
	QMap<QString, Channel> m_channels;
	const QString SEPARATOR = ",";

	double computeAvg(QVector<QString> values);
	bool isNumber(const QString &str);
};
} // namespace datalogger
} // namespace scopy

#endif // DataLoggerModel_H
