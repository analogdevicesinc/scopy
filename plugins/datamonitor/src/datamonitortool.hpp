#ifndef DATAMONITORTOOL_HPP
#define DATAMONITORTOOL_HPP

#include <QPushButton>
#include <QWidget>
#include <tooltemplate.h>
#include <toolbuttons.h>
#include <QMap>
#include <QScrollArea>

namespace scopy {

class CollapsableMenuControlButton;
class FlexGridLayout;

namespace datamonitor {

class DataMonitorModel;
class DataMonitorController;

class DataMonitorTool : public QWidget
{
	Q_OBJECT
public:
	explicit DataMonitorTool(QWidget *parent = nullptr);

	RunBtn *getRunButton() const;

	void initDataMonitor();

private:
	ToolTemplate *tool;
	QPushButton *openLatMenuBtn;
	GearBtn *settingsButton;
	InfoBtn *infoBtn;
	RunBtn *runBtn;
	SingleShotBtn *singleBtn;

	FlexGridLayout *m_flexGridLayout;
	QScrollArea *m_scrollArea;
	QMap<int, DataMonitorController *> *m_monitors;
	void generateMonitor(DataMonitorModel *model, CollapsableMenuControlButton *channelManager);

	QButtonGroup *grp;
};
} // namespace datamonitor
} // namespace scopy
#endif // DATAMONITORTOOL_HPP
