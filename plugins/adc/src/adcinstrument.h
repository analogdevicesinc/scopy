#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/verticalchannelmanager.h>
#include <gui/cursorcontroller.h>
#include <gui/widgets/measurementpanel.h>
#include <gui/widgets/measurementsettings.h>

#include "toolcomponent.h"
#include <adcplugin.h>


namespace scopy {
namespace adc {

class ADCInstrument : public QWidget
{
	friend class ADCFFTInstrumentController;
	friend class ADCTimeInstrumentController;
	friend class ADCInstrumentController;
	Q_OBJECT
public:
	ADCInstrument(ToolMenuEntry *tme, QWidget *parent = nullptr);
	~ADCInstrument();

	ToolTemplate *getToolTemplate();
	MapStackedWidget *getRightStack();

	int uuid = 0;
	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
	const QString settingsMenuId = "settings";

	VerticalChannelManager *vcm() const;

	QPushButton *sync() const;

public Q_SLOTS:
	void stopped();
	void started();
	void addDevice(CollapsableMenuControlButton *b, ToolComponent *dev);
	void addChannel(MenuControlButton *btn, ToolComponent *ch, CompositeWidget *c);

Q_SIGNALS:
	void requestStart();
	void requestStop();

	void requestNewInstrument(ADCInstrumentType t);
	void requestDeleteInstrument();

private:
	ToolTemplate *tool;
	ToolMenuEntry *m_tme;

	QPushButton *openLastMenuBtn;
	MapStackedWidget *rightStack;
	QButtonGroup *rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	AddBtn *addBtn;
	RemoveBtn *removeBtn;
	RunBtn *m_runBtn;
	GearBtn *m_settingsBtn;
	QPushButton *m_complex;
	SingleShotBtn *m_singleBtn;
	QPushButton *m_sync;
	MenuControlButton *m_cursor;
	MenuControlButton *channelsBtn;
	VerticalChannelManager *m_vcm;

	void setupToolLayout();
	void setupRunSingleButtonHelper();

	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);
	void setupCursorButtonHelper(MenuControlButton *cursor);
};
} // namespace adc
} // namespace scopy
static int instrumentIdx = 0;

#endif // ADCINSTRUMENT_H
