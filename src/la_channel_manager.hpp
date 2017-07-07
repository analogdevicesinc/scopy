#ifndef LA_CHANNEL_MANAGER_H
#define LA_CHANNEL_MANAGER_H

#include <QDebug>
#include "digitalchannel_manager.hpp"
#include "libsigrokdecode/libsigrokdecode.h"
#include <QMimeData>
#include <QMimeType>
#include <QDrag>
#include <QFrame>
#include <QBitmap>
#include <QVBoxLayout>

namespace Ui {
class LAChannelManager;
class LAChannelGroup;
class LASettingsWidget;
class LARequiredChannel;
class LChannelSettings;
class LAManagerHeader;
}
namespace pv {
namespace view {
class View;
class LogicSignal;
class DecodeTrace;
}
namespace widgets{
class ColourButton;
}
namespace binding {
class Decoder;
}
namespace prop {
class Property;
}
}
namespace adiscope {
class LogicAnalyzer;
class LogicAnalyzerChannelGroup;
class LogicAnalyzerChannelGroupUI;
class LogicAnalyzerChannelManagerUI;
class MouseWheelWidgetGuard;

class LogicAnalyzerChannel : public Channel
{
public:
	LogicAnalyzerChannel(uint16_t id_, std::string label_);
	const srd_channel *channel_role;
	std::string trigger;

	const srd_channel *getChannel_role();
	void setChannel_role(const srd_channel *value);
	std::string getTrigger() const;
	void setTrigger(const std::string& value);
	qreal getCh_thickness() const;
	void setCh_thickness(qreal value);

private:
	qreal ch_thickness;
};


class LogicAnalyzerChannelUI : public ChannelUI
{
	Q_OBJECT
	LogicAnalyzerChannel *lch;
	LogicAnalyzerChannelManagerUI *chm_ui;
	LogicAnalyzerChannelGroup *chgroup;
	LogicAnalyzerChannelGroupUI *chgroupui;
	std::shared_ptr<pv::view::LogicSignal> trace;

	QRect topDragbox;
	QRect centerDragbox;
	QRect botDragbox;

	int traceOffset;
	int traceHeight;
private:
	static std::vector<std::string> trigger_mapping;
public:
	LogicAnalyzerChannelUI(LogicAnalyzerChannel *ch,
	                       LogicAnalyzerChannelGroup *chgroup,
	                       LogicAnalyzerChannelGroupUI *chgroupui,
	                       LogicAnalyzerChannelManagerUI *chm_ui,
	                       QWidget *parent = 0);
	~LogicAnalyzerChannelUI();
	Ui::LAChannelGroup *ui;
	LogicAnalyzerChannel *getChannel();
	void setTrace(std::shared_ptr<pv::view::LogicSignal> item);
	std::shared_ptr<pv::view::LogicSignal> getTrace();

	void channelRoleChanged(const QString);
	LogicAnalyzerChannelGroup* getChannelGroup();
	QPoint dragStartPosition;
	QFrame *topSep,*botSep;

	void updateTrace();

	void highlightTopSeparator();
	void highlightBotSeparator();
	void resetSeparatorHighlight(bool force = false);
	void enableControls(bool enabled);
private Q_SLOTS:
	void remove();
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void triggerChanged(int);
public Q_SLOTS:
	void rolesChangedLHS(const QString text);
Q_SIGNALS:
	void requestUpdateUI();
};


class LogicAnalyzerChannelGroup : public ChannelGroup
{
private:
	bool collapsed;
	const srd_decoder *decoder;
	QStringList decoderRolesNameList;
	std::map<const srd_channel*,uint16_t> channels_;
	const srd_channel* findByValue(uint16_t ch_id);
	qreal ch_thickness;
public:
	LogicAnalyzerChannelGroup(LogicAnalyzerChannel *ch);
	LogicAnalyzerChannelGroup();
	~LogicAnalyzerChannelGroup();
	const srd_decoder *getDecoder();
	void setDecoder(const srd_decoder *value);
	bool isCollapsed();
	void collapse(bool val);
	QStringList get_decoder_roles_list();
	const srd_channel *get_srd_channel_from_name(const char*);
	LogicAnalyzerChannel* getChannelById(int id);
	void setChannelForDecoder(const srd_channel*, uint16_t);
	std::map<const srd_channel*, uint16_t> get_decoder_channels();
	qreal getCh_thickness() const;
	void setCh_thickness(qreal value);
	std::vector<const srd_channel *> decoderOptChannels;
	std::vector<const srd_channel *> decoderReqChannels;
	std::vector< std::shared_ptr<pv::prop::Property> > properties_;
};


class LogicAnalyzerChannelGroupUI : public ChannelGroupUI
{
	Q_OBJECT
	LogicAnalyzerChannelGroup *lchg;
	LogicAnalyzerChannelManagerUI *chm_ui;
	QRect centerDragbox;
	QRect topDragbox;
	QRect botDragbox;
	int traceOffset;
	int traceHeight;
public:
	LogicAnalyzerChannelGroupUI(LogicAnalyzerChannelGroup *chg,
	                            LogicAnalyzerChannelManagerUI *chm_ui,
	                            QWidget *parent = 0);
	~LogicAnalyzerChannelGroupUI();
	LogicAnalyzerChannelGroup *getChannelGroup();
	std::vector<LogicAnalyzerChannelUI *> ch_ui;
	Ui::LAChannelGroup *ui;
	void enableControls(bool enabled);
	void setTrace(std::shared_ptr<pv::view::LogicSignal> item);
	void setTrace(std::shared_ptr<pv::view::DecodeTrace> item);

	std::shared_ptr<pv::view::TraceTreeItem> getTrace();
	LogicAnalyzerChannelUI* findChannelWithRole(const QString role);
	QPoint dragStartPosition;
	QFrame *topSep,*botSep, *chUiSep;

	void updateTrace();
	int getTraceOffset();
	void setupDecoder();

	void highlightTopSeparator();
	void highlightBotSeparator();
	void resetSeparatorHighlight(bool force = false);
	std::shared_ptr<pv::view::DecodeTrace> getDecodeTrace();

private Q_SLOTS:
	void set_decoder(std::string value);
	void collapse_group();
	void decoderChanged(const QString);
	void triggerChanged(int);
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
private:
	std::shared_ptr<pv::view::TraceTreeItem> trace;
	std::shared_ptr<pv::view::LogicSignal> logicTrace;
	std::shared_ptr<pv::view::DecodeTrace> decodeTrace;
	static std::vector<std::string> trigger_mapping;
public Q_SLOTS:
	void remove();
	void enable(bool enabled);
Q_SIGNALS:
	void remove(int index);
	void requestUpdateUI();
};


class LogicAnalyzerChannelManager : public ChannelManager
{
public:
	LogicAnalyzerChannelManager();
	~LogicAnalyzerChannelManager();
	void join(std::vector<int> index);
	void split(int index);
	void remove(int index);
	void removeChannel(int grIndex, int chIndex);
	void initDecoderList(bool first_level_decoder=true);
	QStringList get_name_decoder_list();
	const srd_decoder *get_decoder_from_name(const char *);
	void highlightChannel(LogicAnalyzerChannelGroup *chg,
	                      LogicAnalyzerChannel *ch=nullptr);
	LogicAnalyzerChannelGroup *getHighlightedChannelGroup();
	LogicAnalyzerChannel *getHighlightedChannel();
	void moveChannel(int fromChgIndex, int from, int to, bool after = true);
	void move(int from, int to, bool after = true);
	void splitChannel(int chgIndex, int chIndex);

	LogicAnalyzerChannelGroup *get_channel_group(int index);
	void add_channel_group(LogicAnalyzerChannelGroup *chg);
	LogicAnalyzerChannel *get_channel(int);
	void clearChannelGroups();
	void clearTrigger();
private:
	std::vector<const srd_decoder *> decoderList;
	QStringList nameDecoderList;
	static int decoder_name_cmp(const void *a, const void *b);
	LogicAnalyzerChannelGroup *highlightedChannelGroup;
	LogicAnalyzerChannel *highlightedChannel;
protected:
	std::vector<LogicAnalyzerChannel *> lchannels;
};


class LogicAnalyzerChannelManagerUI : public QWidget
{
	Q_OBJECT
public:
	pv::MainWindow *main_win;
	std::vector<LogicAnalyzerChannelGroupUI *> chg_ui;
	QWidget *managerHeaderWidget;
	QWidget *locationSettingsWidget;
	QVBoxLayout *locationSettingsLayout;
	QWidget *currentSettingsWidget;
	QWidget *generalSettings;
	Ui::LASettingsWidget *settingsUI;
	Ui::LChannelSettings *generalSettingsUi;
	Ui::LAManagerHeader *managerHeaderUI;
	std::vector<Ui::LARequiredChannel*> decChannelsUi;
	const bool pixmapEnable = true;
	const bool pixmapGrab = true;
	const bool pixmapRetainSize = true;
	const int  pixmapScale = 1;
	std::vector<QFrame *> separators;
	QWidget *hoverWidget;

	LogicAnalyzerChannelManagerUI(QWidget *parent,
	                              pv::MainWindow *main_win_,
	                              LogicAnalyzerChannelManager *chm,
	                              QWidget *locationSettingsWidget,
	                              QVBoxLayout *locationSettingsLayout,
	                              LogicAnalyzer *la);
	~LogicAnalyzerChannelManagerUI();
	LogicAnalyzerChannelManager *chm;
	LogicAnalyzer *la;
	Ui::LAChannelManager *ui;
	void update_ui();
	void update_ui_children(LogicAnalyzerChannelGroupUI*);
	void collapse(bool);
	void showHighlight(bool check);
	void setHoverWidget(QWidget *hover);
	void clearHoverWidget();

	LogicAnalyzerChannelGroupUI *getUiFromChGroup(
	        LogicAnalyzerChannelGroup *);
	LogicAnalyzerChannelUI *getUiFromCh(LogicAnalyzerChannel *);
	void createSettingsWidget();
	void deleteSettingsWidget();
	void set_pv_decoder(LogicAnalyzerChannelGroupUI *channelgroup);
	void retainWidgetSizeWhenHidden(QWidget *w);
	bool eventFilter(QObject *object, QEvent *event);
	void updatePlot();
	void setWidgetMinimumNrOfChars(QWidget *w, int nrOfChars);
	std::shared_ptr<pv::binding::Decoder> binding_;

public Q_SLOTS:
	void chmScrollChanged(int value);
	void remove();
	void set_label(QString);
	void rolesChangedRHS(const QString);
	void triggerUpdateUi();
	void highlightNext();
	void highlightPrevious();
	void resizeEvent(QResizeEvent *event);
	void chThicknessChanged(QString text);

private Q_SLOTS:
	void groupSplit_clicked();
	void hideInactive_clicked(bool hide);
	void chmRangeChanged(int min, int max);
	void colorChanged(QColor color);
private:
	bool hidden;
	bool collapsed;
	std::vector<int> visibleItemsIndexes;
	QButtonGroup *settings_exclusive_group;
	pv::widgets::ColourButton *colour_button_edge, *colour_button_BG,
		*colour_button_low, *colour_button_high;
	QFrame* addSeparator(QVBoxLayout *lay, int pos);
	void createColorButtons();
	void showColorSettings(bool);
	MouseWheelWidgetGuard *eventFilterGuard;
	void enableCgSettings(bool en);
Q_SIGNALS:
	void widthChanged(int);
};
}
#endif // LA_CHANNEL_MANAGER_HPP
