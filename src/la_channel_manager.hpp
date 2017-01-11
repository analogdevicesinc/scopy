#ifndef LA_CHANNEL_MANAGER_H
#define LA_CHANNEL_MANAGER_H

#include <QDebug>
#include "digitalchannel_manager.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

namespace Ui {
class LAChannelManager;
class LAChannelGroup;
class LASettingsWidget;
class LARequiredChannel;
}

namespace adiscope {
class LogicAnalyzer;
class LogicAnalyzerChannelGroup;
class LogicAnalyzerChannelManagerUI;
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
};


class LogicAnalyzerChannelUI : public ChannelUI
{
	Q_OBJECT
	LogicAnalyzerChannel *lch;

public:
	LogicAnalyzerChannelUI(LogicAnalyzerChannel *ch,
	                       LogicAnalyzerChannelGroup *chgroup,
	                       LogicAnalyzerChannelManagerUI *chm_ui,
	                       QWidget *parent = 0);
	Ui::LAChannelGroup *ui;
	void mousePressEvent(QMouseEvent *event);
	LogicAnalyzerChannel *getChannel();
	void setTrace(std::shared_ptr<pv::view::TraceTreeItem> item);
	std::shared_ptr<pv::view::TraceTreeItem> getTrace();
	void channelRoleChanged(const QString);
private Q_SLOTS:
	void remove();
public Q_SLOTS:
	//    void set_decoder(std::string value);
	void rolesChangedLHS(const QString text);
private:
	LogicAnalyzerChannelManagerUI *chm_ui;
	LogicAnalyzerChannelGroup *chgroup;
	std::shared_ptr<pv::view::TraceTreeItem> trace;
};


class LogicAnalyzerChannelGroup : public ChannelGroup
{
private:
	bool collapsed;
	const srd_decoder *decoder;
	QStringList decoderRolesNameList;
	std::vector<const srd_channel *> decoderRolesList;
	std::map<const srd_channel*,
		std::shared_ptr<pv::view::TraceTreeItem> > channels_;
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
	void setChannelForDecoder(const srd_channel*,
		std::shared_ptr<pv::view::TraceTreeItem>);
	std::map<const srd_channel*,
		std::shared_ptr<pv::view::TraceTreeItem> > get_decoder_channels();
};


class LogicAnalyzerChannelManagerUI;
class LogicAnalyzerChannelGroupUI : public ChannelGroupUI
{
	Q_OBJECT
	LogicAnalyzerChannelGroup *lchg;
	LogicAnalyzerChannelManagerUI *chm_ui;
public:
	LogicAnalyzerChannelGroupUI(LogicAnalyzerChannelGroup *chg,
	                            LogicAnalyzerChannelManagerUI *chm_ui,
	                            QWidget *parent = 0);
	LogicAnalyzerChannelGroup *getChannelGroup();
	std::vector<LogicAnalyzerChannelUI *> ch_ui;
	Ui::LAChannelGroup *ui;
	void enableControls(bool enabled);
	void setTrace(std::shared_ptr<pv::view::TraceTreeItem> item);
	std::shared_ptr<pv::view::TraceTreeItem> getTrace();
	LogicAnalyzerChannelUI* findChannelWithRole(const QString role);

private Q_SLOTS:
	void set_decoder(std::string value);
	void collapse_group();
	void decoderChanged(const QString);
private:
	std::shared_ptr<pv::view::TraceTreeItem> trace;
public Q_SLOTS:
	void remove();
	void enable(bool enabled);
Q_SIGNALS:
	void remove(int index);
protected:
	void mousePressEvent(QMouseEvent *event);
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
private:
	std::vector<const srd_decoder *> decoderList;
	QStringList nameDecoderList;
	static int decoder_name_cmp(const void *a, const void *b);
	LogicAnalyzerChannelGroup *highlightedChannelGroup;
	LogicAnalyzerChannel *highlightedChannel;
};



class LogicAnalyzerChannelManagerUI : public QWidget
{
	Q_OBJECT
public:
	pv::MainWindow *main_win;
	std::vector<LogicAnalyzerChannelGroupUI *> chg_ui;
	QWidget *managerHeaderWidget;
	QWidget *locationSettingsWidget;
	QWidget *currentSettingsWidget;
	Ui::LASettingsWidget *settingsUI;
	bool highlightShown;
	LogicAnalyzerChannelManagerUI(QWidget *parent,
	                              pv::MainWindow *main_win_,
	                              LogicAnalyzerChannelManager *chm,
	                              QWidget *locationSettingsWidget,
	                              LogicAnalyzer *la);
	LogicAnalyzerChannelManager *chm;
	LogicAnalyzer *la;
	Ui::LAChannelManager *ui;
	void update_ui();
	void collapse(bool);
	void showHighlight(bool check);
	LogicAnalyzerChannelGroupUI *getUiFromChGroup(
	        LogicAnalyzerChannelGroup *);
	LogicAnalyzerChannelUI *getUiFromCh(LogicAnalyzerChannel *);
	void createSettingsWidget();
	void deleteSettingsWidget();
	void set_pv_decoder(LogicAnalyzerChannelGroupUI *channelgroup);

public Q_SLOTS:
	void chmScrollChanged(int value);
	void remove();
	void set_label(QString);
	void rolesChangedRHS(const QString);

private Q_SLOTS:
	void on_groupSplit_clicked();
	void on_hideInactive_clicked(bool hide);

private:
	bool hidden;
	bool collapsed;
	std::vector<int> visibleItemsIndexes;
	QButtonGroup *settings_exclusive_group;
	void retainWidgetSizeWhenHidden(QWidget *w);
	void setWidgetMinimumNrOfChars(QWidget *w, int nrOfChars);
};
}
#endif // LA_CHANNEL_MANAGER_HPP
