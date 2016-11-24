#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "la_channel_manager.hpp"
#include "ui_la_channel_group.h"
#include "ui_la_channel.h"
#include "ui_pg_channel_manager.h"

namespace Ui {
	class PGChannelManager;
}

namespace pv {
	class MainWindow;
	namespace view {
		class View;
		class TraceTreeItem;
	}
}


namespace adiscope {


LogicAnalyzerChannel::LogicAnalyzerChannel(uint16_t id_,
                std::string label_) :
	Channel(id_,label_),
	channel_role("None"),
	trigger("rising")
{
}

std::string LogicAnalyzerChannel::getTrigger() const
{
	return trigger;
}

void LogicAnalyzerChannel::setTrigger(const std::string& value)
{
	trigger = value;
}

std::string LogicAnalyzerChannel::getChannel_role() const
{
	return channel_role;
}

void LogicAnalyzerChannel::setChannel_role(const std::string& value)
{
	channel_role = value;
}


LogicAnalyzerChannelUI::LogicAnalyzerChannelUI(LogicAnalyzerChannel *ch,
		QWidget *parent) :
	ChannelUI(ch,parent)
{
	this->lch = ch;
}

uint16_t LogicAnalyzerChannelUI::get_id_pvItem()
{
	return id_pvItem;
}

void LogicAnalyzerChannelUI::set_id_pvItem(uint16_t id)
{
	id_pvItem = id;
}


std::string LogicAnalyzerChannelGroup::getDecoder() const
{
	return decoder;
}

void LogicAnalyzerChannelGroup::setDecoder(const std::string& value)
{
	decoder = value;
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup(LogicAnalyzerChannel *ch):
	ChannelGroup(ch)
{
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup() : ChannelGroup()
{

}


LogicAnalyzerChannelGroupUI::LogicAnalyzerChannelGroupUI(
		LogicAnalyzerChannelGroup *chg, QWidget *parent):
	ChannelGroupUI(chg,parent)
{
	this->lchg = chg;
}

void LogicAnalyzerChannelGroupUI::set_decoder(std::string value)
{
	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(value);
	qDebug()<<QString().fromStdString(lchg->getDecoder());
}

uint16_t LogicAnalyzerChannelGroupUI::get_id_pvItem()
{
	return id_pvItem;
}

void LogicAnalyzerChannelGroupUI::set_id_pvItem(uint16_t id)
{
	id_pvItem = id;
}



LogicAnalyzerChannelManager::LogicAnalyzerChannelManager() :
	ChannelManager()
{
	for (auto i=0; i<16; i++) {
		std::string temp = "DIO" + std::to_string(i);
		channel.push_back(new LogicAnalyzerChannel(i,temp));
	}

	auto temp = static_cast<LogicAnalyzerChannel *>(channel.back());

	qDebug()<< QString().fromStdString(temp->getChannel_role());

	for (auto&& ch : channel) {
		channel_group.push_back(new LogicAnalyzerChannelGroup(
				static_cast<LogicAnalyzerChannel *>(ch)));
	}

}

LogicAnalyzerChannelManager::~LogicAnalyzerChannelManager()
{
	for (auto ch : channel_group) {
		delete ch;
	}

	for (auto ch : channel) {
		delete ch;
	}
}

void LogicAnalyzerChannelManager::join(std::vector<int> index)
{
	ChannelGroup *new_ch_group = new LogicAnalyzerChannelGroup();

	for (auto i=0; i<index.size(); i++) {
		for(auto j = 0; j < get_channel_group(index[i])->get_channels().size(); j++)
			new_ch_group->add_channel(
				get_channel_group(index[i])->get_channels()[j]);
	}

	new_ch_group->group(true);
	new_ch_group->set_label("GROUP");
	channel_group.push_back(new_ch_group);
}

void LogicAnalyzerChannelManager::split(int index)
{
	auto it = std::next(channel_group.begin(), index);
	it++;
	it = std::next(channel_group.begin(), index);
	channel_group.erase(it);
}


LogicAnalyzerChannelManagerUI::LogicAnalyzerChannelManagerUI(QWidget *parent,
		pv::MainWindow *main_win_,
		LogicAnalyzerChannelManager *chm) :
	QWidget(parent),
	ui(new Ui::PGChannelManager)
{
	ui->setupUi(this);
	main_win = main_win_;
	this->chm = chm;

	// update_ui();
}

void invalidateLayout(QLayout *layout);

void forceUpdate(QWidget *widget)
{
	// Update all child widgets.
	for (int i = 0; i < widget->children().size(); i++) {
		QObject *child = widget->children()[i];

		if (child->isWidgetType()) {
			forceUpdate((QWidget *)child);
		}
	}

	// Invalidate the layout of the widget.
	if (widget->layout()) {
		invalidateLayout(widget->layout());
	}
}

void invalidateLayout(QLayout *layout)
{
	// Recompute the given layout and all its child layouts.
	for (int i = 0; i < layout->count(); i++) {
		QLayoutItem *item = layout->itemAt(i);

		if (item->layout()) {
			invalidateLayout(item->layout());
		} else {
			item->invalidate();
		}
	}

	layout->invalidate();
	layout->activate();
}


void LogicAnalyzerChannelManagerUI::update_ui()
{
	ui->groupSplit->hide();

	for (auto ch : chg_ui) {
		delete ch;
	}

	main_win->view_->remove_trace_clones();
	chg_ui.erase(chg_ui.begin(),chg_ui.end());

	auto offset = 0;

	for (auto&& ch : *(chm->get_channel_groups())) {
		chg_ui.push_back(new LogicAnalyzerChannelGroupUI(
			static_cast<LogicAnalyzerChannelGroup *>(ch), 0));

		Ui::LA_channel_group *lachannelgroup =
				new Ui::LA_channel_group;
		lachannelgroup->setupUi(chg_ui.back());
		ui->verticalLayout->insertWidget(chg_ui.size(),chg_ui.back());
		lachannelgroup->groupName->setText(
				QString().fromStdString(ch->get_label()));

		int i = 0;
		connect(lachannelgroup->selectCheckBox, SIGNAL(toggled(bool)),
			chg_ui.back(), SLOT(select(bool)));

		offset+=(chg_ui.back()->geometry().bottomRight().y()-10);

		if (ch->is_grouped()) { // create subwidgets
			chg_ui.back()->set_id_pvItem(
				main_win->view_->add_decoder());
			auto trace = main_win->view_->get_trace_by_id(
				chg_ui.back()->get_id_pvItem());
			trace->force_to_v_offset(offset);


			for (auto i=0; i<ch->get_channel_count(); i++) {
				Ui::LA_channel *lachannelui =
					new Ui::LA_channel;
				LogicAnalyzerChannelUI *p =
					new LogicAnalyzerChannelUI(
						static_cast<LogicAnalyzerChannel*>(
							ch->get_channel(i)));
				lachannelui->setupUi(p);
				lachannelgroup->layoutChildren->insertWidget(i,p);

				auto str = QString().fromStdString(
					ch->get_channel(i)->get_label());
				lachannelui->channelName->setText(str);
				str = QString().number(ch->get_channel(i)->get_id());
				lachannelui->channelIndex->setText(str);
				auto index = ch->get_channel(i)->get_id();
				auto trace = main_win->view_->get_clone_of(index);
				p->set_id_pvItem(trace->getIdentifier());

				forceUpdate(p);
				offset+=p->geometry().bottomRight().y();
				trace->force_to_v_offset(offset);
			}
		}
		else {
			auto index = ch->get_channel(0)->get_id();
			auto trace = main_win->view_->get_clone_of(index);

			if (trace) {
				trace->force_to_v_offset(offset);
				chg_ui.back()->set_id_pvItem(
					trace->getIdentifier());
			}
		}
	}
}

void LogicAnalyzerChannelManagerUI::collapse()
{
}

void LogicAnalyzerChannelManagerUI::expand()
{
}


void LogicAnalyzerChannelManagerUI::on_groupSplit_clicked()
{
	std::vector<int> selection = chm->get_selected_indexes();

	if (selection.size() == 0) {
		update_ui();
		return;
	}

	if (selection.size() == 1) {
		if (chm->get_channel_group(selection.back())->is_grouped()) {
			chm->split(selection.back());
		} else {
			chm->join(selection);
		}

		chm->deselect_all();
	} else {
		chm->join(selection);
		chm->deselect_all();
	}

	update_ui();
}

void LogicAnalyzerChannelManagerUI::changeStuff(int value)
{
	update_ui();
}
}
