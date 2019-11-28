/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
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
	LogicAnalyzerChannel(uint16_t id_, const std::string &label_);
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
public:
	LogicAnalyzerChannelUI(LogicAnalyzerChannel *ch,
	                       LogicAnalyzerChannelGroup *chgroup,
	                       LogicAnalyzerChannelGroupUI *chgroupui,
	                       LogicAnalyzerChannelManagerUI *chm_ui,
	                       QWidget *parent = 0);
	~LogicAnalyzerChannelUI();
	Ui::LAChannelGroup *ui;
	LogicAnalyzerChannel *getChannel();
	void setTrace(std::shared_ptr<pv::view::LogicSignal> &item);
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
	bool eventFilter(QObject *watched, QEvent *event);
private Q_SLOTS:
	void remove();
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
	void enterEvent(QEvent *event) override;
	void leaveEvent(QEvent *event) override;
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
	QString decoderSettingsString;
	std::map<const srd_channel*,uint16_t> channels_;
	const srd_channel* findByValue(uint16_t ch_id);
	qreal ch_thickness;
	std::vector<LogicAnalyzerChannel*> logicChannels;

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
	LogicAnalyzerChannel* get_channel_by_id(int id);
	LogicAnalyzerChannel* get_channel_at_index(unsigned int index);
	LogicAnalyzerChannel* get_logic_channel_by_id(int id);
	void add_logic_channel(LogicAnalyzerChannel *chn);
	void remove_logic_channel(int chnIndex);
	void setChannelForDecoder(const srd_channel*, uint16_t);
	std::map<const srd_channel*, uint16_t> get_decoder_channels();
	qreal getCh_thickness() const;
	void setCh_thickness(qreal value);
	void saveDecoderSettings();
	QString getDecoderSettings();
	void setDecoderSettings(QString val);
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
	void setTrace();

	std::shared_ptr<pv::view::TraceTreeItem> getTrace();
	LogicAnalyzerChannelUI* findChannelWithRole(const QString role);
	QPoint dragStartPosition;
	QFrame *topSep,*botSep, *chUiSep;

	void updateTrace();
	int getTraceOffset();
	void setupDecoderSettings();
	void setupDecoder();

	void highlightTopSeparator();
	void highlightBotSeparator();
	void resetSeparatorHighlight(bool force = false);
	std::shared_ptr<pv::view::DecodeTrace> getDecodeTrace();
	bool eventFilter(QObject *watched, QEvent *event);

	std::shared_ptr<pv::binding::Decoder> getBinding();
	void setBinding(std::shared_ptr<pv::binding::Decoder>&);

private Q_SLOTS:
	void set_decoder(const std::string &value);
	void collapse_group();
	void decoderChanged(const QString);
	void triggerChanged(int);
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;
	void enterEvent(QEvent *event) override;
	void leaveEvent(QEvent *event) override;
private:
	std::shared_ptr<pv::view::TraceTreeItem> trace;
	std::shared_ptr<pv::view::LogicSignal> logicTrace;
	std::shared_ptr<pv::view::DecodeTrace> decodeTrace;
	std::shared_ptr<pv::binding::Decoder> binding_;
public Q_SLOTS:
	void remove();
	void enable(bool enabled);
Q_SIGNALS:
	void remove(int index);
	void requestUpdateUI();
	void channel_enabled();
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

	LogicAnalyzerChannelGroup *get_channel_group(unsigned int index);
	void add_channel_group(LogicAnalyzerChannelGroup *chg);
	LogicAnalyzerChannel *get_channel(unsigned int);
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
	void set_streaming_mode(bool);
	bool is_streaming_mode();
	std::vector<std::string> getTriggerMapping();
	std::string getTriggerMapping(int);
	void remove_trace_clones();
	void setupChannel(LogicAnalyzerChannelGroup*, QFrame*);
	void setupGroupedChannel(LogicAnalyzerChannelGroupUI*, QFrame*);
	void deselect_all();
	std::vector<LogicAnalyzerChannelGroupUI*> getEnabledChannelGroups();
	bool checkChannelInGroup(int);

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
	bool streaming_mode;
	pv::widgets::ColourButton *colour_button_edge, *colour_button_BG,
		*colour_button_low, *colour_button_high;
	QFrame* addSeparator(QVBoxLayout *lay, int pos);
	void createColorButtons();
	void showColorSettings(bool);
	MouseWheelWidgetGuard *eventFilterGuard;
	void enableCgSettings(bool en);
	std::vector<std::string> trigger_mapping;
Q_SIGNALS:
	void widthChanged(int);
	void channels_changed();
};
}
#endif // LA_CHANNEL_MANAGER_HPP
