#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <QDateTime>
#include <QObject>
#include <QTimer>

namespace scopy {
namespace datamonitor {

class TimeManager : public QObject
{
	Q_OBJECT

protected:
	TimeManager(QObject *parent = nullptr);
	~TimeManager();

public:
	TimeManager(TimeManager &other) = delete;
	static TimeManager *GetInstance();

	void setStartTime();
	double startTime() const;
	QDateTime startTime();

	void startTimer();
	void stopTimer();
	void setTimerInterval(double interval);

	bool isRunning() const;
	void setIsRunning(bool newIsRunning);

	QDateTime lastReadValue() const;

Q_SIGNALS:
	void timeout();

private:
	static TimeManager *pinstance_;

	QDateTime m_startTime;
	bool m_realTime = false;
	QTimer *m_timer;
	bool m_isRunning = false;

	QDateTime m_lastReadValue;
};
} // namespace datamonitor
} // namespace scopy
#endif // TIMEMANAGER_H
