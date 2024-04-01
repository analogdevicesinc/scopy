#ifndef DATAMONITORTOOL_H
#define DATAMONITORTOOL_H

#include "dataacquisitionmanager.hpp"
#include "scopy-datamonitorplugin_export.h"
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <menucontrolbutton.h>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include "monitorplot.hpp"
#include "monitorselectionmenu.hpp"
#include "qloggingcategory.h"

Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR);
Q_DECLARE_LOGGING_CATEGORY(CAT_DATAMONITOR_TOOL);

namespace scopy::datamonitor {

class DataMonitorSettings;
class SCOPY_DATAMONITORPLUGIN_EXPORT DatamonitorTool : public QWidget
{
	friend class DataMonitorStyleHelper;
	Q_OBJECT
public:
	DatamonitorTool(DataAcquisitionManager *dataAcquisitionManager, QWidget *parent = nullptr);
	~DatamonitorTool();

	RunBtn *getRunButton() const;

Q_SIGNALS:
	void requestNewTool();
	void runToggled(bool toggled);
	void settingsTitleChanged(QString newTitle);

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

	QTextEdit *textMonitors;

	MonitorPlot *m_monitorPlot;
	DataMonitorSettings *m_dataMonitorSettings;
	MonitorSelectionMenu *m_monitorSelectionMenu;
};
} // namespace scopy::datamonitor
#endif // DATAMONITORTOOL_H
