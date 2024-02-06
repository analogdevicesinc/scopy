#ifndef DATAMONITORSETTINGS_HPP
#define DATAMONITORSETTINGS_HPP

#include <QWidget>
#include <menucombo.h>
#include <menuheader.h>
#include <menuonoffswitch.h>

namespace scopy {

class MenuSectionWidget;

class CollapsableMenuControlButton;
namespace datamonitor {
class DataMonitorSettings : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	explicit DataMonitorSettings(QWidget *parent = nullptr);
	~DataMonitorSettings();

	void init(QString title, QColor color);

public Q_SLOTS:
	void peakHolderToggle(bool toggled);
	void plotToggle(bool toggled);
	void peakHolderResetClicked();
	void plotSizeChanged(int size);
	void changeLineStyle(int index);
	void addMonitorsList(QList<QString> monitoList);
	void updateTitle(QString title);
	void updateMainMonitor(QString monitorTitle);
	QList<QString> *getActiveMonitors();

Q_SIGNALS:
	void monitorColorChanged(QString color);
	void togglePlot(bool toggled);
	void changeTimePeriod(int newValue);
	void lineStyleChanged(Qt::PenStyle lineStyle);
	void lineStyleIndexChanged(int index);
	void plotSizeIndexChanged(int index);
	void resetPeakHolder();
	void togglePeakHolder(bool toggled);
	void toggleAll(bool toggle);
	void monitorToggled(bool toggled, QString monitorTitle);
	void removeMonitor();
	void mainMonitorChanged(QString monitorTitle);

private:
	Qt::PenStyle lineStyleFromIdx(int idx);
	MenuOnOffSwitch *peakHolderSwitch;
	MenuOnOffSwitch *plotSwitch;
	QPushButton *peakHolderReset;
	MenuComboWidget *plotStyle;
	MenuComboWidget *plotSize;

	MenuComboWidget *mainMonitorCombo;
	QPushButton *deleteMonitor;

	MenuHeaderWidget *header;
	QVBoxLayout *layout;

	QWidget *settingsBody;
	QVBoxLayout *mainLayout;

	QList<QPair<QString, QCheckBox *>> *monitorsCheckboxList;

	// activeMonitors dropdown
	QList<QString> *activeMonitors;

	QMap<QString, MenuSectionWidget *> deviceMap;

	void addMonitor(QString monitor);
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORSETTINGS_HPP
