#ifndef LA_CHANNEL_MANAGER_H
#define LA_CHANNEL_MANAGER_H

#include <QDebug>
#include "digitalchannel_manager.hpp"

namespace Ui {
	class LAChannelManager;
}

namespace adiscope {
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
					QWidget *parent = 0);
		uint16_t get_id_pvItem();
		void set_id_pvItem(uint16_t id);

	private Q_SLOTS:
		//    void set_decoder(std::string value);

	private:
		uint16_t id_pvItem;
	};


	class LogicAnalyzerChannelGroup : public ChannelGroup
	{

	public:
		std::string decoder;
		LogicAnalyzerChannelGroup(LogicAnalyzerChannel *ch);
		LogicAnalyzerChannelGroup();
		std::string getDecoder() const;
		void setDecoder(const std::string& value);
	};


	class LogicAnalyzerChannelGroupUI : public ChannelGroupUI
	{
		Q_OBJECT
		LogicAnalyzerChannelGroup *lchg;
	public:
		LogicAnalyzerChannelGroupUI(LogicAnalyzerChannelGroup *chg,
					    QWidget *parent = 0);
		uint16_t get_id_pvItem();
		void set_id_pvItem(uint16_t id);
	private Q_SLOTS:
		void set_decoder(std::string value);
	private:
		uint16_t id_pvItem;
	};


	class LogicAnalyzerChannelManager : public ChannelManager
	{
	public:
		LogicAnalyzerChannelManager();
		~LogicAnalyzerChannelManager();
		void join(std::vector<int> index);
		void split(int index);
	};


	class LogicAnalyzerChannelManagerUI : public QWidget
	{
		Q_OBJECT
	public:
		pv::MainWindow *main_win;
		std::vector<LogicAnalyzerChannelGroupUI *> chg_ui;
		LogicAnalyzerChannelManagerUI(QWidget *parent,
				pv::MainWindow *main_win_,
				LogicAnalyzerChannelManager *chm);
		LogicAnalyzerChannelManager *chm;
		Ui::LAChannelManager *ui;
		void update_ui();
		void collapse();
		void expand();

	public Q_SLOTS:
		void update_position(int value);

	private Q_SLOTS:
		void on_groupSplit_clicked();
		void on_hideInactive_clicked(bool hide);

	private:
		bool hidden;
	};
}
#endif // LA_CHANNEL_MANAGER_HPP
