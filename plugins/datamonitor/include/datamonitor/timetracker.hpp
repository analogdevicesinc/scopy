#ifndef TIMETRACKER_HPP
#define TIMETRACKER_HPP

#include <QObject>

namespace scopy {
namespace datamonitor {

class TimeTracker : public QObject
{
	Q_OBJECT

protected:
	TimeTracker(QObject *parent = nullptr);
	~TimeTracker();

public:
	TimeTracker(TimeTracker &other) = delete;
	static TimeTracker *GetInstance();

	void setStartTime();
	double startTime() const;

	double time();

private:
	static TimeTracker *pinstance_;

	double m_startTime = 0;
};
} // namespace datamonitor
} // namespace scopy
#endif // TIMETRACKER_HPP
