#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "la_channel_manager.hpp"
#include "logic_analyzer.hpp"
#include "dynamicWidget.hpp"
#include "ui_la_channel_group.h"
#include "ui_la_channel_manager.h"
#include "ui_la_manager_header.h"
#include "ui_la_settings.h"
#include "ui_la_decoder_reqChannel.h"
#include <QScrollBar>
#include <libsigrokcxx/libsigrokcxx.hpp>

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
	ui(new Ui::LAChannelGroup)
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

void LogicAnalyzerChannelUI::mousePressEvent(QMouseEvent *event)
{
	chm_ui->showHighlight(false);
	chm_ui->chm->highlightChannel(nullptr, getChannel());
	chm_ui->showHighlight(true);
}


void LogicAnalyzerChannelUI::setTrace(std::shared_ptr<pv::view::TraceTreeItem> item)
{
    trace = item;
}

std::shared_ptr<pv::view::TraceTreeItem> LogicAnalyzerChannelUI::getTrace()
{
    return trace;
}

LogicAnalyzerChannel* LogicAnalyzerChannelUI::getChannel()
{
    return static_cast<LogicAnalyzerChannel*>(this->lch);
}

void LogicAnalyzerChannelUI::remove()
{
	auto channelGroups = chm_ui->chm->get_channel_groups();
	auto chGroupIt = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
	if(chGroupIt == channelGroups->end())
		return;

	auto chGroupIndex = chGroupIt - channelGroups->begin();
	auto channels = chm_ui->chm->get_channel_group(chGroupIndex)->get_channels();
	auto chIt = std::find(channels->begin(), channels->end(), lch);
	if(chIt == channels->end())
		return;
	auto chIndex = chIt - channels->begin();
	chm_ui->chm->removeChannel(chGroupIndex, chIndex);
	chm_ui->update_ui();
}

const srd_decoder* LogicAnalyzerChannelGroup::getDecoder()
{
	return decoder;
}

void LogicAnalyzerChannelGroup::setDecoder(const srd_decoder *value)
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
	decoder = nullptr;
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup():
		ChannelGroup()
{
	collapsed = false;
	decoder = nullptr;
}

LogicAnalyzerChannelGroup::~LogicAnalyzerChannelGroup()
{
	if(decoder)
	{
		delete decoder;
		decoder = nullptr;
	}
}


LogicAnalyzerChannelGroupUI::LogicAnalyzerChannelGroupUI(
		LogicAnalyzerChannelGroup *chg,
		LogicAnalyzerChannelManagerUI *chm_ui,
		QWidget *parent):
	ChannelGroupUI(chg,parent),
	ui(new Ui::LAChannelGroup)
{
	this->ui->setupUi(this);
	this->lchg = chg;
	this->chm_ui = chm_ui;
}

void LogicAnalyzerChannelGroupUI::set_decoder(std::string value)
{
//	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(value);
//	qDebug()<<QString().fromUtf8(lchg->getDecoder()->name);
}

uint16_t LogicAnalyzerChannelGroupUI::get_id_pvItem()
{
	return id_pvItem;
}

void LogicAnalyzerChannelGroupUI::set_id_pvItem(uint16_t id)
{
	id_pvItem = id;
}


void LogicAnalyzerChannelGroupUI::setTrace(std::shared_ptr<pv::view::TraceTreeItem> item)
{
    trace = item;
}

std::shared_ptr<pv::view::TraceTreeItem> LogicAnalyzerChannelGroupUI::getTrace()
{
    return trace;
}

void LogicAnalyzerChannelGroupUI::remove()
{
	lchg->select(true);
	chm_ui->remove();
}

void LogicAnalyzerChannelGroupUI::settingsHandler()
{
//	chm_ui->set_current_channelGroup(this);

}

void LogicAnalyzerChannelGroupUI::mousePressEvent(QMouseEvent *event)
{
	chm_ui->showHighlight(false);
	chm_ui->chm->highlightChannel(getChannelGroup(), nullptr);
	chm_ui->showHighlight(true);
}


void LogicAnalyzerChannelGroupUI::collapse_group()
{
	bool value = !getChannelGroup()->isCollapsed();
	getChannelGroup()->collapse(value);
	findChild<QWidget*>("subChannelWidget")->setVisible(!value);
}

void LogicAnalyzerChannelGroupUI::enableControls(bool enabled)
{
	ui->btnRemGroup->setEnabled(enabled);
	ui->decoderCombo->setEnabled(enabled);
	ui->groupName->setEnabled(enabled);
	ui->comboBox_2->setEnabled(enabled);
	ui->comboBox->setEnabled(enabled);
	ui->indexLabel->setEnabled(enabled);
	ui->selectCheckBox->setEnabled(enabled);
}

void LogicAnalyzerChannelGroupUI::enable(bool enabled)
{
	ChannelGroupUI::enable(enabled);
	enableControls(enabled);
}

void LogicAnalyzerChannelGroupUI::decoderChanged(const QString text)
{
	const srd_decoder *decoder;
	if(text == "None")
	{
		decoder = nullptr;
	}
	else
	{
		decoder = chm_ui->chm->get_decoder_from_name(
			text.toStdString().c_str());
	}
	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(decoder);
	if(decoder)
	{
		chm_ui->set_pv_decoder(this, get_id_pvItem());
	}
	chm_ui->deleteSettingsWidget();
	chm_ui->chm->highlightChannel(getChannelGroup());
	chm_ui->createSettingsWidget();
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

	highlightedChannel = nullptr;
	highlightedChannelGroup = static_cast<LogicAnalyzerChannelGroup*>(channel_group.at(0));
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
		for(auto j = 0; j < get_channel_group(index[i])->get_channels()->size(); j++)
			new_ch_group->add_channel(
				get_channel_group(index[i])->get_channels()->at(j));
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

void LogicAnalyzerChannelManager::initDecoderList(bool first_level_decode)
{
	GSList *dL = g_slist_sort(g_slist_copy(
		(GSList*)srd_decoder_list()), decoder_name_cmp);
	for(; dL; dL = dL->next)
	{
		const srd_decoder *const d = (srd_decoder*)dL->data;
		decoderList.push_back(d);
		nameDecoderList << QString::fromUtf8(d->name);
	}
}

QStringList LogicAnalyzerChannelManager::get_name_decoder_list()
{
	return nameDecoderList;
}

const srd_decoder* LogicAnalyzerChannelManager::get_decoder_from_name(const char* name)
{
	for(auto var : decoderList)
	{
		if (strcmp(var->name, name) == 0)
			return var;
	}
	return nullptr;
}

void LogicAnalyzerChannelManager::highlightChannel(LogicAnalyzerChannelGroup *chg,
	LogicAnalyzerChannel *ch)
{
	highlightedChannel = ch;
	highlightedChannelGroup = chg;
}

LogicAnalyzerChannelGroup* LogicAnalyzerChannelManager::getHighlightedChannelGroup()
{
	return highlightedChannelGroup;
}

LogicAnalyzerChannel* LogicAnalyzerChannelManager::getHighlightedChannel()
{
	return highlightedChannel;
}

int LogicAnalyzerChannelManager::decoder_name_cmp(const void *a, const void *b)
{
	return strcmp(((const srd_decoder*)a)->name,
		((const srd_decoder*)b)->name);
}

LogicAnalyzerChannelManagerUI::LogicAnalyzerChannelManagerUI(QWidget *parent,
		pv::MainWindow *main_win_,
		LogicAnalyzerChannelManager *chm,
		QWidget *settingsWidget,
		LogicAnalyzer *la) :
	QWidget(parent),
	ui(new Ui::LAChannelManager),
	hidden(false),
	collapsed(false),
	settings_exclusive_group(new QButtonGroup(this)),
	managerHeaderWidget(nullptr),
	locationSettingsWidget(settingsWidget),
	settingsUI(nullptr),
	currentSettingsWidget(nullptr)
{
	ui->setupUi(this);
	main_win = main_win_;
	this->chm = chm;
	this->la = la;
	this->chm->initDecoderList();
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
    w->setMaximumWidth(label_min_width+4);
}

void LogicAnalyzerChannelManagerUI::update_ui()
{
	for (auto ch : chg_ui) {
		delete ch;
	}
	if(managerHeaderWidget)
	{
		delete managerHeaderWidget;
		managerHeaderWidget = nullptr;
	}

	managerHeaderWidget = new QWidget(ui->headerWidget);
	Ui::LAManagerHeader *managerHeaderUI =
		new Ui::LAManagerHeader();
	managerHeaderUI->setupUi(managerHeaderWidget);
	ui->headerWidgetLayout->addWidget(managerHeaderWidget);

	setWidgetMinimumNrOfChars(managerHeaderUI->labelName, 8);
	setWidgetMinimumNrOfChars(managerHeaderUI->label, 8);
	setWidgetMinimumNrOfChars(managerHeaderUI->labelView, 5);
	setWidgetMinimumNrOfChars(managerHeaderUI->labelOutput, 8);
	setWidgetMinimumNrOfChars(managerHeaderUI->indexLabel, 5);

	if(collapsed)
	{
		managerHeaderUI->rightWidget->setVisible(false);
		managerHeaderUI->leftWidget->setVisible(false);
		managerHeaderUI->line->setVisible(false);
	}

	main_win->view_->remove_trace_clones();
	chg_ui.erase(chg_ui.begin(),chg_ui.end());
	auto offset = 0;
	for (auto&& ch : *(chm->get_channel_groups())) {
		if((ch->is_enabled() && hidden) || !hidden) {

			LogicAnalyzerChannelGroupUI *lachannelgroupUI =
				new LogicAnalyzerChannelGroupUI(
				static_cast<LogicAnalyzerChannelGroup *>(ch), this, 0);

			chg_ui.push_back(lachannelgroupUI);

			ui->verticalLayout->insertWidget(chg_ui.size() - 1,
				chg_ui.back());
			lachannelgroupUI->ensurePolished();
			lachannelgroupUI->ui->groupName->setText(
				QString::fromStdString(ch->get_label()));
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->groupName, 8);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->indexLabel, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox_2, 5);

			retainWidgetSizeWhenHidden(lachannelgroupUI->ui->collapseGroupBtn);
			if (collapsed)
			{
				lachannelgroupUI->ui->leftWidget->setVisible(false);
				lachannelgroupUI->ui->rightWidget->setVisible(false);
				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(0);

			}

			lachannelgroupUI->ui->btnEnableChannel->setChecked(ch->is_enabled());
			lachannelgroupUI->enableControls(ch->is_enabled());


			int i = 0;
			connect(lachannelgroupUI->ui->selectCheckBox, SIGNAL(toggled(bool)),
				chg_ui.back(), SLOT(select(bool)));
			connect(lachannelgroupUI->ui->btnEnableChannel, SIGNAL(toggled(bool)),
				lachannelgroupUI, SLOT(enable(bool)));

			offset+=(chg_ui.back()->geometry().bottomRight().y()-10);

			if (ch->is_grouped()) {			// grouped widget
				chg_ui.back()->set_id_pvItem(
					main_win->view_->add_decoder());
				auto trace = main_win->view_->get_trace_by_id(
					chg_ui.back()->get_id_pvItem());
				trace->force_to_v_offset(offset);

				setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 15);
				lachannelgroupUI->ui->decoderCombo->addItem("None");
				for(auto var : chm->get_name_decoder_list())
				{
					lachannelgroupUI->ui->decoderCombo->addItem(var);
				}
				if(lachannelgroupUI->getChannelGroup()->getDecoder())
				{
					QString name = QString::fromUtf8(lachannelgroupUI->getChannelGroup()->getDecoder()->name);
					int decIndex =chm->get_name_decoder_list().indexOf(name)+1;
					lachannelgroupUI->ui->decoderCombo->setCurrentIndex(decIndex);
					set_pv_decoder(lachannelgroupUI, lachannelgroupUI->get_id_pvItem());
				}
				else
				{
					lachannelgroupUI->ui->decoderCombo->setCurrentIndex(0);
				}

				lachannelgroupUI->ui->comboBox->setVisible(false);


				connect(lachannelgroupUI->ui->collapseGroupBtn, SIGNAL(clicked()),
					lachannelgroupUI, SLOT(collapse_group()));
				connect(lachannelgroupUI->ui->decoderCombo, SIGNAL(currentIndexChanged(const QString&)),
					lachannelgroupUI, SLOT(decoderChanged(const QString&)));

				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(1);

				for (auto i=0; i<ch->get_channel_count(); i++) {	// create subwidgets
					LogicAnalyzerChannelUI *lachannelUI =
						new LogicAnalyzerChannelUI(
						static_cast<LogicAnalyzerChannel*>(
						ch->get_channel(i)),
						lachannelgroupUI->getChannelGroup(),
						this);

					lachannelUI->ensurePolished();
					auto str = QString::fromStdString(
						ch->get_channel(i)->get_label());
					lachannelUI->ui->groupName->setText(str);
					setWidgetMinimumNrOfChars(lachannelUI->ui->groupName, 8);
					setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel2, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox_2, 8);

					retainWidgetSizeWhenHidden(lachannelUI->ui->collapseGroupBtn);
					lachannelUI->ui->stackedWidget->setCurrentIndex(2);
					if (collapsed)
					{
						lachannelUI->ui->leftWidget->setVisible(false);
						lachannelUI->ui->rightWidget->setVisible(false);
						lachannelUI->ui->stackedWidget->setCurrentIndex(0);
					}
					else
					{
						retainWidgetSizeWhenHidden(lachannelUI->ui->btnEnableChannel);
						retainWidgetSizeWhenHidden(lachannelUI->ui->selectCheckBox);
						retainWidgetSizeWhenHidden(lachannelUI->ui->line);
						retainWidgetSizeWhenHidden(lachannelUI->ui->line_2);
					}
					lachannelUI->ui->line->setVisible(false);
					lachannelUI->ui->line_2->setVisible(false);
					lachannelUI->ui->btnEnableChannel->setVisible(false);
					lachannelUI->ui->selectCheckBox->setVisible(false);
					lachannelUI->ui->collapseGroupBtn->setVisible(false);

					lachannelgroupUI->ui->layoutChildren->insertWidget(i,lachannelUI);


					connect(lachannelgroupUI->ui->btnRemGroup, SIGNAL(pressed()),
						lachannelgroupUI, SLOT(remove()));
					connect(lachannelUI->ui->btnRemGroup, SIGNAL(pressed()),
						lachannelUI, SLOT(remove()));


					str = QString().number(ch->get_channel(i)->get_id());
					lachannelUI->ui->indexLabel2->setText(str);
					lachannelUI->ui->indexLabel->setText(str);
					auto index = ch->get_channel(i)->get_id();
					auto trace = main_win->view_->get_clone_of(index);                    
					lachannelUI->set_id_pvItem(trace->getIdentifier());
                    lachannelgroupUI->setTrace(trace);
//					main_win->view_->add_channel_to_group

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
				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(0);
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
                lachannelgroupUI->setTrace(trace);
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
	main_win->view_->verticalScrollBar()->valueChanged(value);

}

void LogicAnalyzerChannelManagerUI::showHighlight(bool check)
{
	LogicAnalyzerChannelGroupUI *chGroupUi = getUiFromChGroup(chm->getHighlightedChannelGroup());
	LogicAnalyzerChannelUI *chUi = getUiFromCh(chm->getHighlightedChannel());
	if(chGroupUi != nullptr)
	{
		setDynamicProperty(chGroupUi->ui->baseWidget, "highlight", check);
		chGroupUi->getTrace()->set_highlight(check);
	}
	if(chUi != nullptr)
	{
		setDynamicProperty(chUi->ui->baseWidget, "highlight", check);
		chUi->getTrace()->set_highlight(check);
	}
	deleteSettingsWidget();
	createSettingsWidget();
}

LogicAnalyzerChannelGroupUI* LogicAnalyzerChannelManagerUI::getUiFromChGroup(
		LogicAnalyzerChannelGroup* seek)
{
	for(auto groupUI : chg_ui)
		if(groupUI->getChannelGroup() == seek)
			return groupUI;
	return nullptr;
}

LogicAnalyzerChannelUI* LogicAnalyzerChannelManagerUI::getUiFromCh(LogicAnalyzerChannel* seek)
{
	for(auto groupUI : chg_ui)
		if(groupUI->getChannelGroup()->get_channel_count()!=0)
			for(auto channelUI : groupUI->ch_ui)
				if(channelUI->get_channel() == seek)
					return channelUI;
	return nullptr;
}

void LogicAnalyzerChannelManagerUI::createSettingsWidget()
{
	settingsUI = new Ui::LASettingsWidget;
	settingsUI->setupUi(locationSettingsWidget);
	currentSettingsWidget = new QWidget(locationSettingsWidget);
	settingsUI->setupUi(currentSettingsWidget);
	locationSettingsWidget->layout()->addWidget(currentSettingsWidget);
	ensurePolished();

	if(chm->getHighlightedChannelGroup())
	{
		LogicAnalyzerChannelGroup *chGroup = chm->getHighlightedChannelGroup();
		settingsUI->nameLineEdit->setText(QString::fromStdString(chGroup->get_label()));
		connect(settingsUI->nameLineEdit, SIGNAL(textChanged(QString)),
			this, SLOT(set_label(QString)));

		if(chGroup->is_grouped())
		{
			const srd_decoder* decoder = chGroup->getDecoder();
			if(!decoder)
			{
				locationSettingsWidget->setVisible(true);
				return;
			}
			GSList* reqCh = g_slist_copy(decoder->channels);
			for(; reqCh; reqCh = reqCh->next)
			{
				const srd_channel *const rqch = (const srd_channel*)reqCh->data;
				if(rqch == nullptr) {break;}
 				Ui::LARequiredChannel *reqChUI = new Ui::LARequiredChannel();
				QWidget *r = new QWidget(currentSettingsWidget);
				reqChUI->setupUi(r);
				reqChUI->labelRole->setText(QString::fromUtf8(rqch->desc));

				for(auto&& ch : *(chGroup->get_channels()))
				{
					reqChUI->roleCombo->addItem(QString::number(ch->get_id()));
				}
				currentSettingsWidget->layout()->addWidget(r);
			}
			free(reqCh);
		}

	}
	if(chm->getHighlightedChannel())
	{
		LogicAnalyzerChannel *ch = chm->getHighlightedChannel();
		settingsUI->nameLineEdit->setText(QString::fromStdString(ch->get_label()));
		connect(settingsUI->nameLineEdit, SIGNAL(textChanged(QString)),
			this, SLOT(set_label(QString)));

		QWidget *r = new QWidget(currentSettingsWidget);
		Ui::LARequiredChannel *roleChUI = new Ui::LARequiredChannel();
		roleChUI->setupUi(r);
		roleChUI->labelRole->setText(QString::fromStdString(ch->getChannel_role()));
		currentSettingsWidget->layout()->addWidget(r);
	}
	locationSettingsWidget->setVisible(true);
}

void LogicAnalyzerChannelManagerUI::deleteSettingsWidget()
{
	if(settingsUI)
	{
		locationSettingsWidget->setVisible(false);
		delete currentSettingsWidget;
		currentSettingsWidget = nullptr;
		delete settingsUI;
		settingsUI = nullptr;
	}
}

void LogicAnalyzerChannelManagerUI::set_label(QString text)
{
	if(chm->getHighlightedChannelGroup())
	{
		chm->getHighlightedChannelGroup()->set_label(text.toStdString());
	}
	if(chm->getHighlightedChannel())
	{
		chm->getHighlightedChannel()->set_label(text.toStdString());
	}
	update_ui();
}

void LogicAnalyzerChannelManagerUI::set_pv_decoder(LogicAnalyzerChannelGroupUI* chGroup, uint16_t id)
{
	main_win->view_->set_decoder_to_group(id, chGroup->getChannelGroup()->getDecoder());
}

}
