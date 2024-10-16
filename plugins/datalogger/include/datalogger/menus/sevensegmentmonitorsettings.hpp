#ifndef SEVENSEGMENTMONITORSETTINGS_HPP
#define SEVENSEGMENTMONITORSETTINGS_HPP

#include <QLineEdit>
#include <QWidget>
#include <menuonoffswitch.h>
#include "scopy-datalogger_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGER_EXPORT SevenSegmentMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	friend class DataLogger_API;
	Q_OBJECT
public:
	explicit SevenSegmentMonitorSettings(QWidget *parent = nullptr);

Q_SIGNALS:
	void peakHolderToggled(bool toggled);
	void precisionChanged(int precision);
signals:

private:
	MenuOnOffSwitch *peakHolderToggle;
	QLineEdit *precision;
	void changePrecision();
};
} // namespace datamonitor
} // namespace scopy
#endif // SEVENSEGMENTMONITORSETTINGS_HPP
