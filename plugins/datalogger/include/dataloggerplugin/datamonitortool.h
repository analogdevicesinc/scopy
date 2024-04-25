#ifndef DATAMONITORTOOL_H
#define DATAMONITORTOOL_H

#include "dataacquisitionmanager.hpp"
#include "scopy-dataloggerplugin_export.h"
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <menucontrolbutton.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include "monitorplot.hpp"
#include "menus/monitorselectionmenu.hpp"
#include "qloggingcategory.h"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR);
Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL);

namespace scopy::datamonitor {

class SevenSegmentDisplay;

class DataMonitorSettings;
class SCOPY_DATALOGGERPLUGIN_EXPORT DatamonitorTool : public QWidget
{
	friend class DataLogger_API;
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	DatamonitorTool(DataAcquisitionManager *dataAcquisitionManager, bool isDeletable = false,
			QWidget *parent = nullptr);
	~DatamonitorTool();

	RunBtn *getRunButton() const;

Q_SIGNALS:
	void requestNewTool();
	void runToggled(bool toggled);
	void settingsTitleChanged(QString newTitle);
	void requestDeleteTool();

private:
	MenuControlButton *monitorsButton;
	DataAcquisitionManager *m_dataAcquisitionManager;

	ToolTemplate *tool;
	GearBtn *settingsButton;
	InfoBtn *infoBtn;
	PrintBtn *printBtn;
	RunBtn *runBtn;
	QPushButton *clearBtn;
	AddBtn *addMonitorButton;

	bool first = true;
	void resetStartTime();

	MonitorPlot *m_monitorPlot;
	QTextEdit *textMonitors;
	SevenSegmentDisplay *sevenSegmetMonitors;

	MenuControlButton *showPlot;
	MenuControlButton *showText;
	MenuControlButton *showSegments;

	DataMonitorSettings *m_dataMonitorSettings;
	MonitorSelectionMenu *m_monitorSelectionMenu;

	void initTutorialProperties();
	void startTutorial();
};
} // namespace scopy::datamonitor
#endif // DATAMONITORTOOL_H
