#ifndef PG_CHANNEL_MANAGER_H
#define PG_CHANNEL_MANAGER_H

#include <QDebug>
#include <QGroupBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QMimeData>
#include <QMimeType>
#include <QDrag>
#include <QBitmap>

#include "pg_patterns.hpp"
#include "digitalchannel_manager.hpp"
#include "ui_pg_channel_group.h"
#include "ui_pg_channel_manager.h"
#include "ui_pg_channel_header.h"
#include "ui_pg_channel.h"

namespace pv {
namespace view {
class View;
class LogicSignal;
class DecodeTrace;
}
}
namespace Ui {
class PGChannelGroup;
class PGChannel;
class PGChannelManager;
class PGChannelManagerHeader;
}


namespace adiscope {
class PatternGenerator;
class PatternGeneratorChannelGroup;
class PatternGeneratorChannelGroupUI;
class PatternGeneratorChannelManagerUI;

class PatternGeneratorChannel : public Channel
{
	std::string channel_role;
	std::string trigger;
public:
	PatternGeneratorChannel(uint16_t id_, std::string label_);

	std::string getChannel_role() const;
	void setChannel_role(const std::string& value);
	std::string getTrigger() const;
	void setTrigger(const std::string& value);
};

class PatternGeneratorChannelUI : public ChannelUI
{
	Q_OBJECT
	PatternGeneratorChannelManagerUI *managerUi;
	PatternGeneratorChannelGroupUI *chgui;
	PatternGeneratorChannelGroup *chg;
	PatternGeneratorChannel *ch;
	QPoint dragStartPosition;

	QRect topDragbox;
	QRect centerDragbox;
	QRect botDragbox;

	int traceOffset;
	int traceHeight;
	std::shared_ptr<pv::view::LogicSignal> trace;

public:
	Ui::PGChannelGroup *ui;
	PatternGeneratorChannelUI(PatternGeneratorChannel *ch,
	                          PatternGeneratorChannelGroup *chg, PatternGeneratorChannelGroupUI *chgui,
	                          PatternGeneratorChannelManagerUI *managerUi, QWidget *parent = 0);
	~PatternGeneratorChannelUI();
	PatternGeneratorChannelManagerUI *getManagerUi() const;
	PatternGeneratorChannel *getChannel();
	PatternGeneratorChannelGroup *getChannelGroup();
	void enableControls(bool val);

	void setTrace(std::shared_ptr<pv::view::LogicSignal> item);
	std::shared_ptr<pv::view::LogicSignal> getTrace();
	void highlight(bool val);

	QFrame *topSep,*botSep;

	void highlightTopSeparator();
	void highlightBotSeparator();
	void resetSeparatorHighlight(bool force = false);
Q_SIGNALS:
	void requestUpdateUi();
private Q_SLOTS:
	void split();
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void paintEvent(QPaintEvent *event);


//    void set_decoder(std::string value);
};

class PatternGeneratorChannelGroup : public ChannelGroup
{
	bool collapsed;
public:
	PatternGeneratorChannelGroup(PatternGeneratorChannel *ch, bool en);
	~PatternGeneratorChannelGroup();
	int created_index;
	Pattern *pattern;
	bool isCollapsed();
	void collapse(bool val);
	void append(PatternGeneratorChannelGroup *tojoin);
};

class PatternGeneratorChannelGroupUI : public ChannelGroupUI
{
	Q_OBJECT
	Q_PROPERTY(int checked READ isChecked WRITE check)
	int checked;
	PatternGeneratorChannelManagerUI *managerUi;
	int isChecked();
	void check(int val);
	QPoint dragStartPosition;
	QRect centerDragbox;
	QRect topDragbox;
	QRect botDragbox;

	std::shared_ptr<pv::view::TraceTreeItem> trace;
	std::shared_ptr<pv::view::LogicSignal> logicTrace;
	std::shared_ptr<pv::view::DecodeTrace> decodeTrace;

public:
	Ui::PGChannelGroup *ui;
	std::vector<PatternGeneratorChannelUI *> ch_ui;
	PatternGeneratorChannelGroupUI(PatternGeneratorChannelGroup *chg,
	                               PatternGeneratorChannelManagerUI *managerUi, QWidget *parent = 0);
	~PatternGeneratorChannelGroupUI();
	PatternGeneratorChannelGroup *getChannelGroup();
	PatternGeneratorChannelManagerUI *getManagerUi() const;
	void setTrace(std::shared_ptr<pv::view::LogicSignal> item);
	void setTrace(std::shared_ptr<pv::view::DecodeTrace> item);
	std::shared_ptr<pv::view::TraceTreeItem> getTrace();
	void enableControls(bool enabled);
	QFrame *topSep,*botSep, *chUiSep;
	int traceOffset;
	int traceHeight;

	void highlight(bool val);
	void highlightTopSeparator();
	void highlightBotSeparator();
	void resetSeparatorHighlight(bool force = false);


Q_SIGNALS:
	void channel_selected();
	void channel_enabled();
	void requestUpdateUi();

private Q_SLOTS:

	void patternChanged(int index);
	void select(bool selected);
	void enable(bool enabled);
	void split();
	void collapse();

private:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void paintEvent(QPaintEvent *event);

};


class PatternGeneratorChannelManager : public ChannelManager
{
	PatternGeneratorChannelGroup *highlightedChannelGroup;
	PatternGeneratorChannel *highlightedChannel;

public:
	void highlightChannel(PatternGeneratorChannelGroup *chg,
	                      PatternGeneratorChannel *ch = nullptr);
	PatternGeneratorChannelGroup *getHighlightedChannelGroup();
	PatternGeneratorChannel *getHighlightedChannel();
	PatternGeneratorChannelManager();
	~PatternGeneratorChannelManager();
	PatternGeneratorChannelGroup *get_channel_group(int index);

	void join(std::vector<int> index);
	void split(int index);
	void move(int from, int to, bool after=true);
	void moveChannel(int fromChgIndex, int from, int to, bool after=true);
	void splitChannel(int chgIndex, int chIndex);
	void preGenerate();
	void generatePatterns(short *mainbuffer, uint32_t sampleRate,
	                      uint32_t bufferSize);
	void commitBuffer(PatternGeneratorChannelGroup *chg, short *mainBuffer,
	                  uint32_t bufferSize);
	short remap_buffer(uint8_t *mapping, uint32_t val);

	uint32_t computeSuggestedSampleRate();
	uint32_t computeSuggestedBufferSize(uint32_t sample_rate);
};

class PatternGeneratorChannelManagerUI : public QWidget
{
	Q_OBJECT
	QWidget *settingsWidget; // pointer to settingspage in stacked widget in main pg ui
	QWidget *channelManagerHeaderWiget;

	PatternUI *currentUI; // pointer to currently drawn patternUI.
	bool disabledShown;
	bool detailsShown;
	bool highlightShown;

public:
	const bool pixmapEnable = true;
	const bool pixmapGrab = true;
	const bool pixmapRetainSize = true;
	const int  pixmapScale = 1;

public:
	pv::MainWindow *main_win;
	std::vector<PatternGeneratorChannelGroupUI *> chg_ui;
	std::vector<QFrame *> separators;
	PatternGeneratorChannelManagerUI(QWidget *parent, pv::MainWindow *main_win_,
	                                 PatternGeneratorChannelManager *chm, QWidget *settingsWidget,
	                                 PatternGenerator *pg);
	~PatternGeneratorChannelManagerUI();

	PatternGeneratorChannelGroupUI *findUiByChannelGroup(
	        PatternGeneratorChannelGroup *toFind);
	PatternGeneratorChannelUI *findUiByChannel(PatternGeneratorChannel *toFind);
	PatternGeneratorChannelManager *chm;
	PatternGenerator *pg;
	Ui::PGChannelManager *ui;

	QWidget *hoverWidget;

	void updateUi();
	void selectChannelGroup(PatternGeneratorChannelGroupUI *selected);
	void deleteSettingsWidget();
	void createSettingsWidget();
	QWidget *getSettingsWidget() const;

	bool isDisabledShown();
	void showDisabled();
	void hideDisabled();

	bool areDetailsShown();
	void showDetails();
	void hideDetails();

	void showHighlight(bool val);
	void setHoverWidget(QWidget *hover);
	void clearHoverWidget();

	void groupSplitSelected();
	PatternGeneratorChannelGroup *getSelectedChannelGroup() const;
	void setSelectedChannelGroup(PatternGeneratorChannelGroup *value);

	void retainWidgetSizeWhenHidden(QWidget *w);
	void setWidgetNrOfChars(QWidget *w, int minNrOfChars, int maxNrOfChars=0);

private:
	QFrame *addSeparator(QVBoxLayout *lay, int pos);

Q_SIGNALS:
	void channelsChanged();

private Q_SLOTS:
	void chmScrollChanged(int val);
	void chmRangeChanged(int min, int max);
	void triggerUpdateUi();

};


}
#endif // PG_CHANNEL_MANAGER_H
