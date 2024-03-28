#ifndef SEVENSEGMENTMONITOR_HPP
#define SEVENSEGMENTMONITOR_HPP

#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
#include <lcdNumber.hpp>
#include "datamonitormodel.hpp"
#include "datamonitorutils.hpp"
#include "scopy-datamonitorplugin_export.h"

namespace scopy::datamonitor {
class SCOPY_DATAMONITORPLUGIN_EXPORT SevenSegmentMonitor : public QFrame
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit SevenSegmentMonitor(DataMonitorModel *model, QWidget *parent = nullptr);

	void togglePeakHolder(bool toggle);
	void updatePrecision(int precision);

signals:

private:
	DataMonitorModel *m_model;
	QHBoxLayout *layout;
	QLabel *name;
	QLabel *unitOfMeasurement;
	LcdNumber *lcdNumber;
	QVBoxLayout *minMaxLayout;
	LcdNumber *lcdNumberMin;
	QLabel *minLabel;
	LcdNumber *lcdNumberMax;
	QLabel *maxLabel;
};
} // namespace scopy::datamonitor
#endif // SEVENSEGMENTMONITOR_HPP
