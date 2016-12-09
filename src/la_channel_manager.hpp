#ifndef LA_CHANNEL_MANAGER_H
#define LA_CHANNEL_MANAGER_H

#include <QDebug>
#include "digitalchannel_manager.hpp"

namespace Ui {
	class LAChannelManager;
	class LA_channel_group;
}

namespace adiscope {
	class LogicAnalyzer;
	class LogicAnalyzerChannelGroup;
	class LogicAnalyzerChannelManagerUI;
	class LogicAnalyzerChannel : public Channel
	{
	public:
		LogicAnalyzerChannel(uint16_t id_, std::string label_);
		std::string channel_role;
		std::string trigger;

		std::string getChannel_role() const;
		void setChannel_role(const std::string& value);
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
		uint16_t get_id_pvItem();
		void set_id_pvItem(uint16_t id);
		Ui::LA_channel_group *ui;

	private Q_SLOTS:
		void remove();
		//    void set_decoder(std::string value);

	private:
		LogicAnalyzerChannelManagerUI *chm_ui;
		LogicAnalyzerChannelGroup *chgroup;
		uint16_t id_pvItem;
	};


	class LogicAnalyzerChannelGroup : public ChannelGroup
	{
	private:
		bool collapsed;
		std::string decoder;
	public:
		LogicAnalyzerChannelGroup(LogicAnalyzerChannel *ch);
		LogicAnalyzerChannelGroup();
		std::string getDecoder() const;
		void setDecoder(const std::string& value);
		bool isCollapsed();
		void collapse(bool val);
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
		uint16_t get_id_pvItem();
		void set_id_pvItem(uint16_t id);
		LogicAnalyzerChannelGroup* getChannelGroup();
		Ui::LA_channel_group *ui;
	private Q_SLOTS:
		void set_decoder(std::string value);
		void collapse_group();
	protected:

	private:
		uint16_t id_pvItem;
	public Q_SLOTS:
		void remove();
		void settingsHandler();
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
	};


	class LogicAnalyzerChannelManagerUI : public QWidget
	{
		Q_OBJECT
	public:
		pv::MainWindow *main_win;
		std::vector<LogicAnalyzerChannelGroupUI *> chg_ui;
		LogicAnalyzerChannelManagerUI(QWidget *parent,
				pv::MainWindow *main_win_,
				LogicAnalyzerChannelManager *chm,
				LogicAnalyzer* la);
		LogicAnalyzerChannelManager *chm;
		LogicAnalyzer *la;
		Ui::LAChannelManager *ui;
		void update_ui();
		void collapse(bool);
		void set_current_channelGroup(LogicAnalyzerChannelGroupUI*);
		LogicAnalyzerChannelGroupUI* get_current_channelGroup();

	public Q_SLOTS:
		void update_position(int value);
		void remove();
		void changeChannelName(const QString&);

	private Q_SLOTS:
		void on_groupSplit_clicked();
		void on_hideInactive_clicked(bool hide);

	private:
		bool hidden;
		bool collapsed;
		std::vector<int> visibleItemsIndexes;
		QButtonGroup* settings_exclusive_group;
		LogicAnalyzerChannelGroupUI *current_channelGroup;
		void retainWidgetSizeWhenHidden(QWidget *w);
		void setWidgetMinimumNrOfChars(QWidget *w, int nrOfChars);
	};
}
#endif // LA_CHANNEL_MANAGER_HPP
