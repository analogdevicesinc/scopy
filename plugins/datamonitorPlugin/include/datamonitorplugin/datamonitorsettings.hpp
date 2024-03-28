#ifndef DATAMONITORSETTINGS_HPP
#define DATAMONITORSETTINGS_HPP

#include <QTimer>
#include <QWidget>
#include <menucombo.h>
#include <menuheader.h>
#include <menuonoffswitch.h>
#include <monitorplot.hpp>
#include <monitorplotcurve.hpp>
#include <spinbox_a.hpp>
#include "scopy-datamonitorplugin_export.h"
#include "qloggingcategory.h"
#include "sevensegmentmonitorsettings.hpp"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS)

namespace scopy {

class MenuCollapseSection;

class MenuSectionWidget;

class CollapsableMenuControlButton;
namespace datamonitor {
class SCOPY_DATAMONITORPLUGIN_EXPORT DataMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorSettings(MonitorPlot *m_plot, QWidget *parent = nullptr);
	~DataMonitorSettings();

	void init(QString title, QColor color);

	SevenSegmentMonitorSettings *getSevenSegmentMonitorSettings() const;

public Q_SLOTS:
	void updateTitle(QString title);
	void plotYAxisMinValueUpdate(double value);
	void plotYAxisMaxValueUpdate(double value);

Q_SIGNALS:
	void curveStyleIndexChanged(int index);
	void changeCurveThickness(double thickness);

	void plotYAxisAutoscale(bool toggled);
	void plotYAxisMinValueChange(double value);
	void plotYAxisMaxValueChange(double value);
	void plotXAxisMinValueChange(double value);
	void plotXAxisMaxValueChange(double value);
	void requestYMinMaxValues();

	void pathChanged(QString path);
	void requestDataLogging(QString path);

private:
	PositionSpinButton *m_ymin;
	PositionSpinButton *m_ymax;
	QWidget *generateYAxisSettings(QWidget *parent);

	MenuHeaderWidget *header;
	QVBoxLayout *layout;

	QWidget *settingsBody;
	QVBoxLayout *mainLayout;

	MonitorPlot *m_plot;

	QString filename;
	QLineEdit *dataLoggingFilePath;
	QPushButton *dataLoggingBrowseBtn;
	QPushButton *dataLoggingBtn;
	void chooseFile();

	SevenSegmentMonitorSettings *sevenSegmentMonitorSettings;

	bool eventFilter(QObject *watched, QEvent *event) override;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSETTINGS_HPP
