#ifndef DATAMONITORSETTINGS_HPP
#define DATAMONITORSETTINGS_HPP

#include <QTimer>
#include <QWidget>
#include <edittextmenuheader.h>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include <monitorplot.hpp>
#include <monitorplotcurve.hpp>
#include <progresslineedit.h>
#include <spinbox_a.hpp>
#include "scopy-dataloggerplugin_export.h"
#include "qloggingcategory.h"
#include "sevensegmentmonitorsettings.hpp"
#include "dataloggingmenu.hpp"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS)

namespace scopy {

class MenuCollapseSection;

class MenuSectionWidget;

class CollapsableMenuControlButton;
namespace datamonitor {

class SCOPY_DATALOGGERPLUGIN_EXPORT DataMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorSettings(MonitorPlot *m_plot, bool isDeletable = false, QWidget *parent = nullptr);
	~DataMonitorSettings();

	void init(QString title, QColor color);

	SevenSegmentMonitorSettings *getSevenSegmentMonitorSettings() const;

	DataLoggingMenu *getDataLoggingMenu() const;

public Q_SLOTS:
	void plotYAxisMinValueUpdate(double value);
	void plotYAxisMaxValueUpdate(double value);

Q_SIGNALS:
	void titleUpdated(QString title);
	void curveStyleIndexChanged(int index);
	void changeCurveThickness(double thickness);

	void plotYAxisAutoscale(bool toggled);
	void plotYAxisMinValueChange(double value);
	void plotYAxisMaxValueChange(double value);
	void plotXAxisMinValueChange(double value);
	void plotXAxisMaxValueChange(double value);
	void requestYMinMaxValues();
	void requestDeleteTool();

private:
	bool m_isDeletable;
	PositionSpinButton *m_ymin;
	PositionSpinButton *m_ymax;
	QPushButton *deleteMonitor = nullptr;
	QWidget *generateYAxisSettings(QWidget *parent);
	QWidget *generateCurveStyleSettings(QWidget *parent);
	QWidget *generatePlotUiSettings(QWidget *parent);

	EditTextMenuHeader *header;
	QVBoxLayout *layout;

	QWidget *settingsBody;
	QVBoxLayout *mainLayout;

	MonitorPlot *m_plot;

	SevenSegmentMonitorSettings *sevenSegmentMonitorSettings;
	DataLoggingMenu *dataLoggingMenu;

	bool eventFilter(QObject *watched, QEvent *event) override;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSETTINGS_HPP
