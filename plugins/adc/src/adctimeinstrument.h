#ifndef ADCTIMEINSTRUMENT_H
#define ADCTIMEINSTRUMENT_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <gui/widgets/menucontrolbutton.h>

namespace scopy {
class ADCTimeInstrument : public QWidget
{
	Q_OBJECT
public:
	ADCTimeInstrument(QWidget *parent = nullptr);
	~ADCTimeInstrument();
private:


	ToolTemplate *tool;

	QPushButton *openLastMenuBtn;
	MapStackedWidget *channelStack;
	QButtonGroup *rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	MenuControlButton *channelsBtn;

	void setupToolLayout();

	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);
	void setupCursorButtonHelper(MenuControlButton *cursor);
	void setupMeasureButtonHelper(MenuControlButton *measure);



	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";

};
}

#endif // ADCTIMEINSTRUMENT_H
