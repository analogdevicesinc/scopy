#ifndef DACINSTRUMENT_H
#define DACINSTRUMENT_H
#include "scopy-dac_export.h"

#include <QWidget>
#include <QPushButton>
#include <QList>

#include "verticalchannelmanager.h"
#include <gui/tooltemplate.h>
#include <gui/tutorialbuilder.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <tooltemplate.h>
#include <iioutil/connectionprovider.h>

namespace scopy {
namespace dac {
class DacDataManager;
class SCOPY_DAC_EXPORT DacInstrument : public QWidget
{
	Q_OBJECT
public:
	DacInstrument(const Connection *conn, QWidget *parent = nullptr);
	virtual ~DacInstrument();

public Q_SLOTS:
	void startTutorial();
	void runToggled(bool toggled);
	void dacRunning(bool toggled);
Q_SIGNALS:
	void running(bool toggled);

private:
	void setupDacDataManagers();
	MenuControlButton *addDevice(DacDataManager *dev, QWidget *parent);
	void addDeviceToStack(DacDataManager *dev, MenuControlButton *btn);
	void setupDacDataDeviceButtons();
	void startBufferTutorial();
	void startBufferNonCyclicTutorial();
	void startDdsTutorial();
	void abortTutorial();

	const Connection *m_conn;
	ToolTemplate *tool;
	InfoBtn *infoBtn;
	GearBtn *settingsBtn;
	QPushButton *openLastMenuBtn;
	QButtonGroup *devicesGroup;
	QButtonGroup *rightMenuBtnGrp;
	MenuControlButton *devicesBtn;
	MapStackedWidget *deviceStack;
	MapStackedWidget *dacManagerStack;
	QList<DacDataManager *> m_dacDataManagers;
	VerticalChannelManager *vcm;
	gui::TutorialBuilder *m_dacBufferTutorial;
	gui::TutorialBuilder *m_dacBufferNonCyclicTutorial;
	gui::TutorialBuilder *m_dacDdsTutorial;
	int uuid = 0;

	const QString settingsMenuId = "settings";
	const QString devicesMenuId = "devices";
	const QString verticalChannelManagerId = "vcm";
};
} // namespace dac
} // namespace scopy

#endif // DACINSTRUMENT_H_
