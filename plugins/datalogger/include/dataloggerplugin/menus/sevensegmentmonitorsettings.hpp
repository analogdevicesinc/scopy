#ifndef SEVENSEGMENTMONITORSETTINGS_HPP
#define SEVENSEGMENTMONITORSETTINGS_HPP

#include <QLineEdit>
#include <QWidget>
#include <menuonoffswitch.h>
#include "scopy-dataloggerplugin_export.h"

namespace scopy {
namespace datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT SevenSegmentMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
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
