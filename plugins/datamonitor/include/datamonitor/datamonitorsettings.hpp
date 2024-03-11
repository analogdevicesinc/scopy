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
#include "scopy-datamonitor_export.h"
#include "qloggingcategory.h"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_SETTINGS)

namespace scopy {

class MenuCollapseSection;

class MenuSectionWidget;

class CollapsableMenuControlButton;
namespace datamonitor {
class SCOPY_DATAMONITOR_EXPORT DataMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorSettings(MonitorPlot *m_plot, QWidget *parent = nullptr);
	~DataMonitorSettings();

	void init(QString title, QColor color);

public Q_SLOTS:
	void plotToggle(bool toggled);
	void addMonitorsList(QMap<QString, DataMonitorModel *> *monitoList);
	void updateTitle(QString title);
	void plotYAxisMinValueUpdate(double value);
	void plotYAxisMaxValueUpdate(double value);

Q_SIGNALS:
	void monitorColorChanged(QString color);
	void togglePlot(bool toggled);
	void curveStyleIndexChanged(int index);
	void changeCurveThickness(double thickness);
	void plotSizeIndexChanged(int index);
	void monitorToggled(bool toggled, QString monitorName);
	void removeMonitor();
	void plotYAxisAutoscale(bool toggled);
	void plotYAxisMinValueChange(double value);
	void plotYAxisMaxValueChange(double value);
	void plotXAxisMinValueChange(double value);
	void plotXAxisMaxValueChange(double value);
	void requestYMinMaxValues();
	void toggleRealTime(bool toggled);

private:
	Qt::PenStyle lineStyleFromIdx(int idx);
	MenuOnOffSwitch *peakHolderSwitch;
	MenuOnOffSwitch *plotSwitch;
	QPushButton *peakHolderReset;

	PositionSpinButton *m_ymin;
	PositionSpinButton *m_ymax;
	QWidget *generateYAxisSettings(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);

	// MenuComboWidget *mainMonitorCombo;
	QPushButton *deleteMonitor;

	MenuHeaderWidget *header;
	QVBoxLayout *layout;

	QWidget *settingsBody;
	QVBoxLayout *mainLayout;

	QButtonGroup *monitorsGroup;

	QMap<QString, MenuCollapseSection *> deviceMap;

	void addMonitor(QString monitor, QColor monitorColor);
	void generateDeviceSection(QString device);

	MonitorPlot *m_plot;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSETTINGS_HPP
