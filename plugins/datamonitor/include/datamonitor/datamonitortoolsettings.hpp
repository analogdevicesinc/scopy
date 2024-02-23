#ifndef DATAMONITORTOOLSETTINGS_HPP
#define DATAMONITORTOOLSETTINGS_HPP

#include <QWidget>

namespace scopy {
namespace datamonitor {

class DataMonitorToolSettings : public QWidget
{
	Q_OBJECT
public:
	explicit DataMonitorToolSettings(QWidget *parent = nullptr);

Q_SIGNALS:
	void readIntervalChanged(double interval);
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORTOOLSETTINGS_HPP
