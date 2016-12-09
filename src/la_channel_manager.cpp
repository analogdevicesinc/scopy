#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "la_channel_manager.hpp"
#include "logic_analyzer.hpp"
#include "ui_la_channel_group.h"
#include "ui_la_channel_manager.h"
#include "ui_la_manager_header.h"
#include <QScrollBar>

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
		LogicAnalyzerChannelGroup *chgroup,
		LogicAnalyzerChannelManagerUI *chm_ui,
		QWidget *parent) :
	ChannelUI(ch,parent),
	ui(new Ui::LA_channel_group)
{
	this->ui->setupUi(this);
	this->lch = ch;
	this->chgroup = chgroup;
	this->chm_ui = chm_ui;
}

uint16_t LogicAnalyzerChannelUI::get_id_pvItem()
{
	return id_pvItem;
}

void LogicAnalyzerChannelUI::set_id_pvItem(uint16_t id)
{
	id_pvItem = id;
}

void LogicAnalyzerChannelUI::remove()
{
	auto channelGroups = chm_ui->chm->get_channel_groups();
	auto chGroupIt = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
	if(chGroupIt == channelGroups->end())
		return;

	auto chGroupIndex = chGroupIt - channelGroups->begin();
	auto channels = chm_ui->chm->get_channel_group(chGroupIndex)->get_channels();
	auto chIt = std::find(channels.begin(), channels.end(), lch);
	if(chIt == channels.end())
		return;
	auto chIndex = chIt - channels.begin();
	chm_ui->chm->removeChannel(chGroupIndex, chIndex);
	chm_ui->update_ui();
}

std::string LogicAnalyzerChannelGroup::getDecoder() const
{
	return decoder;
}

void LogicAnalyzerChannelGroup::setDecoder(const std::string& value)
{
	decoder = value;
}

bool LogicAnalyzerChannelGroup::isCollapsed()
{
    return collapsed;
}

void LogicAnalyzerChannelGroup::collapse(bool val)
{
    collapsed = val;
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup(LogicAnalyzerChannel *ch):
		ChannelGroup(ch)
{
	collapsed = false;
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup():
		ChannelGroup()
{
	collapsed = false;
}


LogicAnalyzerChannelGroupUI::LogicAnalyzerChannelGroupUI(
		LogicAnalyzerChannelGroup *chg,
		LogicAnalyzerChannelManagerUI *chm_ui,
		QWidget *parent):
	ChannelGroupUI(chg,parent),
	ui(new Ui::LA_channel_group)
{
	this->ui->setupUi(this);
	this->lchg = chg;
	this->chm_ui = chm_ui;
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

void LogicAnalyzerChannelGroupUI::remove()
{
	lchg->select(true);
	chm_ui->remove();
}

void LogicAnalyzerChannelGroupUI::settingsHandler()
{
	chm_ui->set_current_channelGroup(this);
}

void LogicAnalyzerChannelGroupUI::mousePressEvent(QMouseEvent *event)
{
	chm_ui->set_current_channelGroup(this);
}


void LogicAnalyzerChannelGroupUI::collapse_group()
{
	bool value = !getChannelGroup()->isCollapsed();
	getChannelGroup()->collapse(value);
	findChild<QWidget*>("subChannelWidget")->setVisible(!value);
}

LogicAnalyzerChannelGroup* LogicAnalyzerChannelGroupUI::getChannelGroup()
{
    return static_cast<LogicAnalyzerChannelGroup*>(this->chg);
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

void LogicAnalyzerChannelManager::removeChannel(int grIndex, int chIndex)
{
	auto grIt = std::next(channel_group.begin(), grIndex);
	auto channels = (*grIt)->get_channels();
	(*grIt)->remove_channel(chIndex);
	if((*grIt)->get_channel_count() == 0)
		channel_group.erase(grIt);

}

void LogicAnalyzerChannelManager::remove(int index)
{
	auto it = std::next(channel_group.begin(), index);
	it = channel_group.erase(it);
}

LogicAnalyzerChannelManagerUI::LogicAnalyzerChannelManagerUI(QWidget *parent,
		pv::MainWindow *main_win_,
		LogicAnalyzerChannelManager *chm,
		LogicAnalyzer *la) :
	QWidget(parent),
	ui(new Ui::LAChannelManager),
	hidden(false),
	collapsed(false),
	settings_exclusive_group(new QButtonGroup(this)),
	current_channelGroup(NULL)
{
	ui->setupUi(this);
	main_win = main_win_;
	this->chm = chm;
	this->la = la;
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
		this, SLOT(update_position(int)));
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

void LogicAnalyzerChannelManagerUI::retainWidgetSizeWhenHidden(QWidget *w)
{
    QSizePolicy sp_retain = w->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    w->setSizePolicy(sp_retain);
}

void LogicAnalyzerChannelManagerUI::setWidgetMinimumNrOfChars(QWidget *w, int nrOfChars)
{
    QFontMetrics labelm(w->font());
    auto label_min_width = labelm.width(QString(nrOfChars,'X'));
    w->setMinimumWidth(label_min_width);
}

void LogicAnalyzerChannelManagerUI::update_ui()
{
	for (auto ch : chg_ui) {
		delete ch;
	}
	delete ui->verticalLayout_1->takeAt(0);
	QWidget *headerWidget = new QWidget(this);
	Ui::LAManagerHeader *managerHeaderUI =
		new Ui::LAManagerHeader;
	managerHeaderUI->setupUi(headerWidget);
	ui->verticalLayout_1->insertWidget(0, headerWidget);

	if(collapsed)
	{
		managerHeaderUI->leftWidget->setVisible(false);
		managerHeaderUI->rightWidget->setVisible(false);
		managerHeaderUI->line->setVisible(false);
	}

	ensurePolished();
	main_win->view_->remove_trace_clones();
	chg_ui.erase(chg_ui.begin(),chg_ui.end());
	current_channelGroup = NULL; //change this
	auto offset = 0;
	for (auto&& ch : *(chm->get_channel_groups())) {
		if((ch->is_enabled() && hidden) || !hidden) {

			LogicAnalyzerChannelGroupUI *lachannelgroupUI =
				new LogicAnalyzerChannelGroupUI(
				static_cast<LogicAnalyzerChannelGroup *>(ch), this, 0);
			chg_ui.push_back(lachannelgroupUI);

			ui->verticalLayout->insertWidget(chg_ui.size() - 1,
				chg_ui.back());
			lachannelgroupUI->ui->groupName->setText(
				QString::fromStdString(ch->get_label()));
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->groupName, 8);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->indexLabel, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox_2, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 5);


			if (collapsed)
			{
				lachannelgroupUI->ui->leftWidget->setVisible(false);
				lachannelgroupUI->ui->rightWidget->setVisible(false);

			}
			else
			{
				retainWidgetSizeWhenHidden(lachannelgroupUI->ui->collapseGroupBtn);
				retainWidgetSizeWhenHidden(lachannelgroupUI->ui->comboBox_2);
			}

			lachannelgroupUI->ui->comboBox_2->setVisible(false);
			lachannelgroupUI->ui->btnEnableChannel->setChecked(ch->is_enabled());


			int i = 0;
			connect(lachannelgroupUI->ui->selectCheckBox, SIGNAL(toggled(bool)),
				chg_ui.back(), SLOT(select(bool)));
			connect(lachannelgroupUI->ui->btnEnableChannel, SIGNAL(toggled(bool)),
				chg_ui.back(), SLOT(enable(bool)));

			offset+=(chg_ui.back()->geometry().bottomRight().y()-10);

			if (ch->is_grouped()) {			// grouped widget
				chg_ui.back()->set_id_pvItem(
					main_win->view_->add_decoder());
				auto trace = main_win->view_->get_trace_by_id(
					chg_ui.back()->get_id_pvItem());
				trace->force_to_v_offset(offset);

				connect(lachannelgroupUI->ui->collapseGroupBtn, SIGNAL(clicked()),
					chg_ui.back(), SLOT(collapse_group()));

				setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 5);
				lachannelgroupUI->ui->decoderCombo->setVisible(false);
				lachannelgroupUI->ui->indexLabel->setVisible(false);

				for (auto i=0; i<ch->get_channel_count(); i++) {	// create subwidgets
					LogicAnalyzerChannelUI *lachannelUI =
						new LogicAnalyzerChannelUI(
						static_cast<LogicAnalyzerChannel*>(
						ch->get_channel(i)),
						lachannelgroupUI->getChannelGroup(),
						this);

					auto str = QString::fromStdString(
						ch->get_channel(i)->get_label());
					lachannelUI->ui->groupName->setText(str);
					setWidgetMinimumNrOfChars(lachannelUI->ui->groupName, 8);
					setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox_2, 5);

					if (collapsed)
					{
						lachannelUI->ui->leftWidget->setVisible(false);
						lachannelUI->ui->rightWidget->setVisible(false);
					}
					else
					{
						retainWidgetSizeWhenHidden(lachannelUI->ui->btnEnableChannel);
						retainWidgetSizeWhenHidden(lachannelUI->ui->selectCheckBox);
						retainWidgetSizeWhenHidden(lachannelUI->ui->collapseGroupBtn);
						retainWidgetSizeWhenHidden(lachannelUI->ui->line);
						retainWidgetSizeWhenHidden(lachannelUI->ui->line_2);
						retainWidgetSizeWhenHidden(lachannelUI->ui->indexLabel);
					}
					lachannelUI->ui->line->setVisible(false);
					lachannelUI->ui->line_2->setVisible(false);
					lachannelUI->ui->btnEnableChannel->setVisible(false);
					lachannelUI->ui->selectCheckBox->setVisible(false);
					lachannelUI->ui->collapseGroupBtn->setVisible(false);
					lachannelUI->ui->decoderCombo->setVisible(false);

					lachannelgroupUI->ui->layoutChildren->insertWidget(i,lachannelUI);


					connect(lachannelgroupUI->ui->btnRemGroup, SIGNAL(pressed()),
						lachannelgroupUI, SLOT(remove()));
					connect(lachannelUI->ui->btnRemGroup, SIGNAL(pressed()),
						lachannelUI, SLOT(remove()));


					str = QString().number(ch->get_channel(i)->get_id());
					lachannelUI->ui->indexLabel->setText(str);
					auto index = ch->get_channel(i)->get_id();
					auto trace = main_win->view_->get_clone_of(index);
					lachannelUI->set_id_pvItem(trace->getIdentifier());

					forceUpdate(lachannelUI);
					offset+=lachannelUI->geometry().bottomRight().y();
					trace->force_to_v_offset(offset);
				}

				if(static_cast<LogicAnalyzerChannelGroup*>(ch)->isCollapsed())
				{
					lachannelgroupUI->ui->collapseGroupBtn->setChecked(true);
					lachannelgroupUI->ui->subChannelWidget->setVisible(false);
				}
			}
			else {
				auto index = ch->get_channel(0)->get_id();

				if(!collapsed)
				{
					retainWidgetSizeWhenHidden(lachannelgroupUI->ui->btnRemGroup);
					retainWidgetSizeWhenHidden(lachannelgroupUI->ui->line);
					retainWidgetSizeWhenHidden(lachannelgroupUI->ui->line_2);
				}
				lachannelgroupUI->ui->btnRemGroup->setVisible(false);
				lachannelgroupUI->ui->collapseGroupBtn->setVisible(false);

				lachannelgroupUI->ui->line->setVisible(false);
				lachannelgroupUI->ui->line_2->setVisible(false);
				lachannelgroupUI->ui->decoderCombo->setVisible(false);
				lachannelgroupUI->ui->indexLabel->setText(QString::number(index));
				auto trace = main_win->view_->get_clone_of(index);
				if (trace) {
					trace->force_to_v_offset(offset);
					chg_ui.back()->set_id_pvItem(
						trace->getIdentifier());
				}
			}
		}
	}

	if(chg_ui.size() != 0)
	{
		ui->scrollArea->verticalScrollBar()->setPageStep(chg_ui.front()->sizeHint().height());
		ui->scrollArea->verticalScrollBar()->setSingleStep(chg_ui.front()->sizeHint().height());
		ui->scrollArea->verticalScrollBar()->setRange(0, ui->scrollAreaWidgetContents->height() - chg_ui.front()->sizeHint().height());
	}
}

void LogicAnalyzerChannelManagerUI::collapse(bool check)
{
	collapsed = check;
	update_ui();
}

void LogicAnalyzerChannelManagerUI::remove()
{
	std::vector<int> selection = chm->get_selected_indexes();
	if (chm->get_channel_group(selection.back())->is_grouped())
		chm->split(selection.back());
	chm->deselect_all();
	update_ui();
}

void LogicAnalyzerChannelManagerUI::on_groupSplit_clicked()
{
	std::vector<int> selection = chm->get_selected_indexes();

	if (selection.size() == 0) {
		update_ui();
		return;
	}

	if (selection.size() == 1) {
		chm->join(selection);
		chm->deselect_all();
	} else {
		chm->join(selection);
		chm->deselect_all();
	}

	update_ui();
}

void LogicAnalyzerChannelManagerUI::on_hideInactive_clicked(bool hide)
{
	hidden = hide;
	update_ui();
}

void LogicAnalyzerChannelManagerUI::update_position(int value)
{
	visibleItemsIndexes.clear();
	/* print visible indexes  */
	int viewportHeight = ui->scrollArea->viewport()->height();
	int itemHeight = chg_ui.front()->sizeHint().height();
	double noOfVisibleItems = floor(viewportHeight / itemHeight);
	double minRange, maxRange;
	minRange = ceil(value / itemHeight);
//	maxRange = minRange + noOfVisibleItems - 1;
//	for(auto it = chg_ui.begin()+minRange; it != chg_ui.begin() + minRange + noOfVisibleItems; it++)
//		visibleItemsIndexes.push_back(it->get_id_pvItem);
	for(int i = minRange; i < minRange + noOfVisibleItems; i++)
		visibleItemsIndexes.push_back(i);
//	update_trace_positions(minRange, minRange + noOfVisibleItems);
}


void LogicAnalyzerChannelManagerUI::set_current_channelGroup(
	LogicAnalyzerChannelGroupUI* channelGroup)
{
	if(current_channelGroup && current_channelGroup != channelGroup)
		current_channelGroup->findChild<QWidget*>("baseWidget")->setStyleSheet("");
	current_channelGroup = channelGroup;
	current_channelGroup->findChild<QWidget*>("baseWidget")->setStyleSheet("QWidget {background-color: rgb(0, 0, 0);}");
}

LogicAnalyzerChannelGroupUI*
	LogicAnalyzerChannelManagerUI::get_current_channelGroup()
{
	return current_channelGroup;
}

void LogicAnalyzerChannelManagerUI::changeChannelName(const QString& label)
{
	current_channelGroup->get_group()->set_label(label.toStdString());
	update_ui();
}

}
