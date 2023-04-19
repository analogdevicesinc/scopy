#ifndef DATALOGGERREADERTHREAD_HPP
#define DATALOGGERREADERTHREAD_HPP


#include <QObject>
#include <dmm.hpp>
#include <qthread.h>

struct activeChannel{
	std::string dmmId;
	libm2k::analog::DMM* dmm;
};


class DataLoggerReaderThread :  public QThread
{
	Q_OBJECT
public:
	DataLoggerReaderThread();

public Q_SLOTS:
	void setDataLoggerStatus(bool status);
	void channelToggled(int id, bool toggled);
	void addChannel(int id, std::string dmmId, libm2k::analog::DMM* dmm);

Q_SIGNALS:
	void updateChannelData(int chId, double value,QString nameOfUnitOfMeasure,QString symbolOfUnitOfMeasure);
	void updateDataLoggerValue(QString name, QString value);

private:
	void run() override;

	bool dataLoggerStatus;
	QMap<int,QPair<bool,activeChannel>> m_activeChannels ;

};

#endif // DATALOGGERREADERTHREAD_HPP
