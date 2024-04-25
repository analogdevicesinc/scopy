#ifndef SEVENSEGMENTDISPLAY_H
#define SEVENSEGMENTDISPLAY_H

#include "datamonitormodel.hpp"
#include "scopy-dataloggerplugin_export.h"
#include "sevensegmentmonitor.hpp"

#include <QBoxLayout>
#include <QMap>
#include <QWidget>

namespace scopy::datamonitor {
class SCOPY_DATALOGGERPLUGIN_EXPORT SevenSegmentDisplay : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit SevenSegmentDisplay(QWidget *parent = nullptr);
	~SevenSegmentDisplay();

	void generateSegment(DataMonitorModel *model);
	void removeSegment(QString segment);

	void updatePrecision(int precision);
	void togglePeakHolder(bool toggle);
signals:

private:
	QVBoxLayout *layout;
	QMap<QString, SevenSegmentMonitor *> *monitorList;
};
} // namespace scopy::datamonitor
#endif // SEVENSEGMENTDISPLAY_H
