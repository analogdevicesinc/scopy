#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/viewport.hpp"
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
class Viewport;
class TraceTreeItem;
}
}


namespace adiscope {


LogicAnalyzerChannel::LogicAnalyzerChannel(uint16_t id_,
                std::string label_) :
	Channel(id_,label_),
	channel_role(nullptr)
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

const srd_channel *LogicAnalyzerChannel::getChannel_role()
{
	return channel_role;
}

void LogicAnalyzerChannel::setChannel_role(const srd_channel *value)
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
	setAcceptDrops(true);
}

LogicAnalyzerChannelUI::~LogicAnalyzerChannelUI()
{
	delete ui;
}

void LogicAnalyzerChannelUI::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		dragStartPosition = event->pos();

	chm_ui->showHighlight(false);
	chm_ui->chm->highlightChannel(nullptr, getChannel());
	chm_ui->showHighlight(true);
}

void LogicAnalyzerChannelUI::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if((event->pos() - dragStartPosition).manhattanLength()
		< QApplication::startDragDistance())
		{
			return;
		}

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	auto channelGroups =  chm_ui->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
	auto chgIndex = chgIter - channelGroups->begin();

	auto channels = (*chgIter)->get_channels();
	auto chIter = std::find(channels->begin(), channels->end(), lch);
	auto chIndex = chIter - channels->begin();

	dataStream << (short)chgIndex<<(short)chIndex;

	mimeData->setData("la/channel", itemData);

	if( chm_ui->pixmapEnable )
	{
		QPixmap pix;
		pix = this->grab().scaled(this->geometry().width() / chm_ui->pixmapScale,
			this->geometry().height() / chm_ui->pixmapScale);
		this->setVisible(!chm_ui->pixmapGrab);
		if( chm_ui->pixmapRetainSize )
		{
			chm_ui->retainWidgetSizeWhenHidden(this);
		}
		drag->setPixmap(pix);
	}
	drag->setMimeData(mimeData);
	Qt::DropAction dropAction = drag->exec( Qt::MoveAction );
	this->setVisible(true);
}

void LogicAnalyzerChannelUI::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();

	topDragbox.setRect(0, 0, this->geometry().width() + 1,
		this->geometry().height()/2 + 1);
	botDragbox.setRect(0, this->geometry().height()/2,
		this->geometry().width() + 1, this->geometry().height()/2 + 1);

	if( event->mimeData()->hasFormat("la/channelgroup"))
	{
		short from = (short)event->mimeData()->data("la/channelgroup")[1];
		auto channelGroups = chm_ui->chm->get_channel_groups();
		auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
		auto chgIndex = chgIter - channelGroups->begin();

		if( from == chgIndex )
		{
			event->ignore();
		}
	}
}

void LogicAnalyzerChannelUI::dragMoveEvent(QDragMoveEvent *event)
{
	if( event->answerRect().intersects(botDragbox) )
	{
		ui->line->setVisible(true);
		ui->line_2->setVisible(false);
	}
	else {
		ui->line->setVisible(false);
		ui->line_2->setVisible(true);
	}

	event->accept();
}

void LogicAnalyzerChannelUI::dragLeaveEvent(QDragLeaveEvent *event)
{
	ui->line->setVisible(false);
	ui->line_2->setVisible(false);
	event->accept();
}

void LogicAnalyzerChannelUI::dropEvent(QDropEvent *event)
{
	ui->line->setVisible(false);
	ui->line_2->setVisible(false);

	if( event->source() == this && event->possibleActions() & Qt::MoveAction)
		return;

	auto channelGroups = chm_ui->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
	auto chgIndex = chgIter - channelGroups->begin();

	auto toNrOfChannels = (*chgIter)->get_channel_count();

	auto channels = (*chgIter)->get_channels();
	auto chIter = std::find(channels->begin(), channels->end(), lch);
	auto chIndex = chIter - channels->begin();

	bool dropAfter = botDragbox.contains(event->pos());
	if( event->mimeData()->hasFormat("la/channelgroup") )
	{
		short from = (short)event->mimeData()->data("la/channelgroup")[1];
		auto fromNrOfChannels = chm_ui->chm->get_channel_group(from)->get_channel_count();
		chm_ui->chm->join({(int)chgIndex,from});
		chm_ui->chm->highlightChannel(chm_ui->chm->get_channel_group(chgIndex));
		chgIndex = chm_ui->chm->get_channel_groups()->size() - 1;
		auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
		auto j = 0;
		for( auto i = fromNrOfChannels; i > 0; i--)
		{
			chm_ui->chm->moveChannel(chgIndex, (*chgIter)->get_channel_count()-i, chIndex+j,dropAfter);
			j++;
		}
	}

	if( event->mimeData()->hasFormat("la/channel") )
	{
		short fromChg = (short)event->mimeData()->data("la/channel")[1];
		short fromCh = (short)event->mimeData()->data("la/channel")[3];

		auto fromNrOfChannels = chm_ui->chm->get_channel_group(fromChg)->get_channel_count();

		chm_ui->chm->splitChannel(fromChg, fromCh);
		if( fromNrOfChannels != 1 )
		{
			fromChg++;
			chgIndex = chgIndex + (chgIndex > fromChg ? 1 : 0);
		}

		chm_ui->chm->join({(int)chgIndex, fromChg});
		chgIndex = chgIndex + (chgIndex > fromChg ? -1 : 0);

		auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
		chm_ui->chm->moveChannel(chgIndex, (*chgIter)->get_channel_count() - 1, chIndex, dropAfter);
	}

	Q_EMIT requestUpdateUI();
}

void LogicAnalyzerChannelUI::channelRoleChanged(const QString text)
{
	const srd_channel *channel_role;
	if (text == "None") {
		channel_role = nullptr;
	} else {
		channel_role = chgroup->get_srd_channel_from_name(
		                  text.toStdString().c_str());
	}
	static_cast<LogicAnalyzerChannel *>(lch)->setChannel_role(channel_role);
	if (channel_role) {
		chgroup->setChannelForDecoder(channel_role, getTrace());
	}
}

LogicAnalyzerChannelGroup* LogicAnalyzerChannelUI::getChannelGroup()
{
	return chgroup;
}

void LogicAnalyzerChannelUI::rolesChangedLHS(const QString text)
{
	channelRoleChanged(text);
	chm_ui->deleteSettingsWidget();
	chm_ui->createSettingsWidget();
	chm_ui->main_win->view_->commit_decoder_channels(
		chm_ui->getUiFromChGroup(chgroup)->getTrace(),
		chgroup->get_decoder_channels());
}

void LogicAnalyzerChannelUI::setTrace(std::shared_ptr<pv::view::TraceTreeItem>
                                      item)
{
	trace = item;
}

std::shared_ptr<pv::view::TraceTreeItem> LogicAnalyzerChannelUI::getTrace()
{
	return trace;
}

LogicAnalyzerChannel *LogicAnalyzerChannelUI::getChannel()
{
	return static_cast<LogicAnalyzerChannel *>(this->lch);
}

void LogicAnalyzerChannelUI::remove()
{
	auto channelGroups = chm_ui->chm->get_channel_groups();
	auto highlightedItem = chm_ui->chm->getHighlightedChannel();
	bool changeHighlight = (highlightedItem == this->lch);

	auto chGroupIt = std::find(channelGroups->begin(), channelGroups->end(),
	                           chgroup);

	if (chGroupIt == channelGroups->end()) {
		return;
	}

	auto chGroupIndex = chGroupIt - channelGroups->begin();
	auto channels = chm_ui->chm->get_channel_group(chGroupIndex)->get_channels();
	auto chIt = std::find(channels->begin(), channels->end(), lch);

	if (chIt == channels->end()) {
		return;
	}

	auto chIndex = chIt - channels->begin();
	chm_ui->chm->removeChannel(chGroupIndex, chIndex);

	if( changeHighlight )
	{
		auto it = chm_ui->chg_ui.begin();
		chm_ui->chm->highlightChannel((*it)->getChannelGroup());
		chm_ui->deleteSettingsWidget();
		chm_ui->createSettingsWidget();
	}
	chm_ui->update_ui();
}

const srd_decoder *LogicAnalyzerChannelGroup::getDecoder()
{
	return decoder;
}

void LogicAnalyzerChannelGroup::setDecoder(const srd_decoder *value)
{
	decoderRolesNameList.clear();
	decoderRolesList.clear();
	channels_.clear();
	decoder = value;

	GSList *reqCh = g_slist_copy(decoder->channels);
	for (; reqCh; reqCh = reqCh->next) {
		const srd_channel *const rqch = (const srd_channel *)reqCh->data;

		if (rqch == nullptr) {
			break;
		}
		decoderRolesNameList << QString::fromUtf8(rqch->name);
		decoderRolesList.push_back(rqch);
	}
	free(reqCh);

	GSList *optChannels = g_slist_copy(decoder->opt_channels);
	for (; optChannels; optChannels = optChannels->next) {
		const srd_channel *const optch = (const srd_channel *)optChannels->data;

		if (optch == nullptr) {
			break;
		}
		decoderRolesNameList << QString::fromUtf8(optch->name);
		decoderRolesList.push_back(optch);
	}
	free(optChannels);
}

LogicAnalyzerChannel* LogicAnalyzerChannelGroup::getChannelById(int id)
{
	for(auto&& ch : *(get_channels()))
	{
		if (ch->get_id() == id)
		{
			return static_cast<LogicAnalyzerChannel *>(ch);
		}
	}
	return nullptr;
}

QStringList LogicAnalyzerChannelGroup::get_decoder_roles_list()
{
	return decoderRolesNameList;
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
	channels_ = std::map<const srd_channel*,
		std::shared_ptr<pv::view::TraceTreeItem> >();
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup():
	ChannelGroup()
{
	collapsed = false;
	decoder = nullptr;
	channels_ = std::map<const srd_channel*,
		std::shared_ptr<pv::view::TraceTreeItem> >();
}

void LogicAnalyzerChannelGroup::setChannelForDecoder(const srd_channel* ch,
		std::shared_ptr<pv::view::TraceTreeItem> trace)
{
	if(!channels_.empty()){
		auto it = channels_.find(ch);
		if( it != channels_.end() )
		{
			channels_.at(ch) = trace;
		}
	}
	else
	{
		channels_.insert(std::pair<const srd_channel*, std::shared_ptr<pv::view::TraceTreeItem> >(ch, trace));
	}
}

std::map<const srd_channel*, std::shared_ptr<pv::view::TraceTreeItem> >
	LogicAnalyzerChannelGroup::get_decoder_channels()
{
	return channels_;
}

LogicAnalyzerChannelGroup::~LogicAnalyzerChannelGroup()
{
	if (decoder) {
		delete decoder;
		decoder = nullptr;
	}
	for(auto var : decoderRolesList) {
		delete var;
	}
	channels_.clear();
}

const srd_channel* LogicAnalyzerChannelGroup::get_srd_channel_from_name(const char* name)
{
	for (auto var : decoderRolesList) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	return nullptr;
}

std::vector<std::string> LogicAnalyzerChannelGroupUI::trigger_mapping = {
		"none",
		"edge-any",
		"edge-rising",
		"edge-falling",
		"level-low",
		"level-high",
};

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
	setAcceptDrops(true);

	/* Set triggerCombo index according to the device */
	LogicAnalyzerChannel *ch;
	if( !lchg->is_grouped() )
	{
		ch = static_cast<LogicAnalyzerChannel*>(lchg->get_channel());
		std::string trigger_val = chm_ui->la->get_trigger_from_device(ch->get_id());
		for(int i = 0; i < trigger_mapping.size(); i++)
		{
			if( trigger_val == trigger_mapping[i] )
			{
				ch->setTrigger(trigger_val);
				ui->comboBox->setCurrentIndex(i);
			}
		}
	}
}

LogicAnalyzerChannelGroupUI::~LogicAnalyzerChannelGroupUI()
{
	delete ui;
}

void LogicAnalyzerChannelGroupUI::set_decoder(std::string value)
{
//	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(value);
//	qDebug()<<QString().fromUtf8(lchg->getDecoder()->name);
}

void LogicAnalyzerChannelGroupUI::setTrace(
        std::shared_ptr<pv::view::TraceTreeItem> item)
{
	trace = item;
}

std::shared_ptr<pv::view::TraceTreeItem> LogicAnalyzerChannelGroupUI::getTrace()
{
	return trace;
}

void LogicAnalyzerChannelGroupUI::remove()
{
	auto highlightedItem = chm_ui->chm->getHighlightedChannelGroup();

	if( highlightedItem == this->lchg )
	{
		auto it = chm_ui->chg_ui.begin();
		chm_ui->chm->highlightChannel((*it)->getChannelGroup());
		chm_ui->deleteSettingsWidget();
		chm_ui->createSettingsWidget();
	}

	auto it = std::find(chm_ui->chm->get_channel_groups()->begin(),
			chm_ui->chm->get_channel_groups()->end(),
			lchg);
	if( it != chm_ui->chm->get_channel_groups()->end() )
	{
		lchg->select(true);
		chm_ui->remove();
	}
}

void LogicAnalyzerChannelGroupUI::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
            dragStartPosition = event->pos();

	chm_ui->showHighlight(false);
	chm_ui->chm->highlightChannel(getChannelGroup(), nullptr);
	chm_ui->showHighlight(true);
}

void LogicAnalyzerChannelGroupUI::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if((event->pos() - dragStartPosition).manhattanLength()
		< QApplication::startDragDistance())
		{
			return;
		}

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;
	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	auto channelGroups =  chm_ui->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chg);
	auto chgIndex = chgIter - channelGroups->begin();

	dataStream << (short)chgIndex;

	mimeData->setData("la/channelgroup", itemData);
	drag->setMimeData(mimeData);

	if( chm_ui->pixmapEnable )
	{
		QPixmap pix;
		pix = this->grab().scaled(this->geometry().width() / chm_ui->pixmapScale,
			this->geometry().height() / chm_ui->pixmapScale);
		this->setVisible(!chm_ui->pixmapGrab);
		if( chm_ui->pixmapRetainSize )
		{
			chm_ui->retainWidgetSizeWhenHidden(this);
		}
		drag->setPixmap(pix);
	}

	Qt::DropAction dropAction = drag->exec( Qt::MoveAction );
	this->setVisible(true);
}

void LogicAnalyzerChannelGroupUI::dragEnterEvent(QDragEnterEvent *event)
{
	topDragbox.setRect(0, 0, ui->baseWidget->geometry().width() + 1,
		ui->baseWidget->geometry().height()/2 + 1);
	botDragbox.setRect(0, ui->baseWidget->geometry().height()/2,
		ui->baseWidget->geometry().width() + 1,
		ui->baseWidget->geometry().height()/2 + 1);
	event->accept();
}

void LogicAnalyzerChannelGroupUI::dragMoveEvent(QDragMoveEvent *event)
{
	if( event->answerRect().intersects(botDragbox) )
	{
		ui->line->setVisible(true);
		ui->line_2->setVisible(false);
		event->accept();
	}
	else if(event->answerRect().intersects(topDragbox))
	{
		ui->line->setVisible(false);
		ui->line_2->setVisible(true);
		event->accept();
	}
	else {
		event->ignore();
	}
}

void LogicAnalyzerChannelGroupUI::dragLeaveEvent(QDragLeaveEvent *event)
{
	ui->line->setVisible(false);
	ui->line_2->setVisible(false);
	event->accept();
}

void LogicAnalyzerChannelGroupUI::dropEvent(QDropEvent *event)
{
	ui->line->setVisible(false);
	ui->line_2->setVisible(false);

	if( event->source() == this && event->possibleActions() & Qt::MoveAction)
		return;

	auto channelGroups = chm_ui->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chg);
	auto chgIndex = chgIter - channelGroups->begin();

	bool dropAfter = botDragbox.contains(event->pos());
	if( event->mimeData()->hasFormat("la/channelgroup") )
	{
		short from = (short)event->mimeData()->data("la/channelgroup")[1];
		chm_ui->chm->move(from, chgIndex, dropAfter);
	}

	if( event->mimeData()->hasFormat("la/channel") )
	{
		short fromChg = (short)event->mimeData()->data("la/channel")[1];
		short fromCh = (short)event->mimeData()->data("la/channel")[3];

		auto fromNrOfChannels = chm_ui->chm->get_channel_group(fromChg)->get_channel_count();

		chm_ui->chm->splitChannel(fromChg, fromCh);
		if( fromNrOfChannels != 1 )
		{
			fromChg++;
			chgIndex = chgIndex + (fromChg > chgIndex ? 1 : 0);
		}
	}

	Q_EMIT requestUpdateUI();
}

void LogicAnalyzerChannelGroupUI::collapse_group()
{
	bool value = !getChannelGroup()->isCollapsed();
	getChannelGroup()->collapse(value);
	findChild<QWidget *>("subChannelWidget")->setVisible(!value);
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

LogicAnalyzerChannelUI*
	LogicAnalyzerChannelGroupUI::findChannelWithRole(const QString role)
{
	for(auto var : ch_ui)
	{
		if( var->getChannel()->getChannel_role() )
		{
			if( var->getChannel()->getChannel_role()->name == role )
			{
				return var;
			}
		}
	}
	return nullptr;
}

void LogicAnalyzerChannelGroupUI::decoderChanged(const QString text)
{
	const srd_decoder *decoder;

	if (text == "None") {
		decoder = nullptr;
	}
	else {
		decoder = chm_ui->chm->get_decoder_from_name(
			text.toStdString().c_str());
	}

	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(decoder);

	if (decoder) {
		chm_ui->set_pv_decoder(this);
	}

	if(getChannelGroup() == chm_ui->chm->getHighlightedChannelGroup())
	{
		chm_ui->deleteSettingsWidget();
		chm_ui->createSettingsWidget();
	}
	chm_ui->update_ui_children(this);
}

void LogicAnalyzerChannelGroupUI::triggerChanged(int index)
{
	LogicAnalyzerChannel *ch;
	if( !lchg->is_grouped() )
	{
		ch = static_cast<LogicAnalyzerChannel*>(lchg->get_channel());
		std::string trigger_val = trigger_mapping[index];
		if( trigger_val != ch->getTrigger() )
		{
			ch->setTrigger(trigger_val);
			chm_ui->la->set_trigger_to_device(ch->get_id(), trigger_val);
		}
	}
}

LogicAnalyzerChannelGroup *LogicAnalyzerChannelGroupUI::getChannelGroup()
{
	return static_cast<LogicAnalyzerChannelGroup *>(this->chg);
}


LogicAnalyzerChannelManager::LogicAnalyzerChannelManager() :
	ChannelManager()
{
	for (auto i=0; i<16; i++) {
		std::string temp = "DIO" + std::to_string(i);
		channel.push_back(new LogicAnalyzerChannel(i,temp));
	}

	auto temp = static_cast<LogicAnalyzerChannel *>(channel.back());

	for (auto&& ch : channel) {
		channel_group.push_back(new LogicAnalyzerChannelGroup(
		                                static_cast<LogicAnalyzerChannel *>(ch)));
	}

	highlightedChannel = nullptr;
	highlightedChannelGroup = static_cast<LogicAnalyzerChannelGroup *>
	                          (channel_group.at(0));
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

LogicAnalyzerChannelGroup *LogicAnalyzerChannelManager::get_channel_group(
        int index)
{
	return static_cast<LogicAnalyzerChannelGroup *>(channel_group[index]);
}

void LogicAnalyzerChannelManager::join(std::vector<int> index)
{
	ChannelGroup *new_ch_group = new LogicAnalyzerChannelGroup();

	for (auto i=0; i<index.size(); i++) {
		for (auto j = 0; j < get_channel_group(index[i])->get_channels()->size(); j++)
		{
			uint16_t someId = get_channel_group(index[i])->get_channels()->at(j)->get_id();
			std::vector<uint16_t> vec = new_ch_group->get_ids();
			auto it = find(vec.begin(),
					vec.end(), someId);
			if( it == vec.end())
			{
				Channel *ch = new LogicAnalyzerChannel(get_channel_group(index[i])->get_channels()->at(j)->get_id(),
					get_channel_group(index[i])->get_channels()->at(j)->get_label());
				new_ch_group->add_channel(ch);
			}
		}
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

	if ((*grIt)->get_channel_count() == 0) {
		channel_group.erase(grIt);
	}

}

void LogicAnalyzerChannelManager::remove(int index)
{
	auto it = std::next(channel_group.begin(), index);
	it = channel_group.erase(it);
}

void LogicAnalyzerChannelManager::initDecoderList(bool first_level_decode)
{
	GSList *dL = g_slist_sort(g_slist_copy(
	                                  (GSList *)srd_decoder_list()), decoder_name_cmp);

	for (; dL; dL = dL->next) {
		const srd_decoder *const d = (srd_decoder *)dL->data;
		decoderList.push_back(d);
		nameDecoderList << QString::fromUtf8(d->name);
	}
}

QStringList LogicAnalyzerChannelManager::get_name_decoder_list()
{
	return nameDecoderList;
}

const srd_decoder *LogicAnalyzerChannelManager::get_decoder_from_name(
        const char *name)
{
	for (auto var : decoderList) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}

	return nullptr;
}

void LogicAnalyzerChannelManager::splitChannel(int chgIndex, int chIndex)
{

	// Use this to insert split channel after channelgroup
	auto it = channel_group.begin() + chgIndex + 1;
	auto subch = channel_group[chgIndex]->get_channel(chIndex);
	auto chIt = channel_group[chgIndex]->get_channels()->begin() + chIndex;

	auto newChgIndex =
	        chgIndex; // Use this to insert split channel after channelgroup
	channel_group[newChgIndex]->get_channels()->erase(chIt);

	if (channel_group[newChgIndex]->get_channel_count()==0) {
		it = channel_group.begin()+newChgIndex;
		delete static_cast<LogicAnalyzerChannelGroup *>(*it);
		channel_group.erase(it);
	}

}

void LogicAnalyzerChannelManager::highlightChannel(LogicAnalyzerChannelGroup
                *chg,
                LogicAnalyzerChannel *ch)
{
	highlightedChannel = ch;
	highlightedChannelGroup = chg;
}

LogicAnalyzerChannelGroup
*LogicAnalyzerChannelManager::getHighlightedChannelGroup()
{
	return highlightedChannelGroup;
}

LogicAnalyzerChannel *LogicAnalyzerChannelManager::getHighlightedChannel()
{
	return highlightedChannel;
}

int LogicAnalyzerChannelManager::decoder_name_cmp(const void *a, const void *b)
{
	return strcmp(((const srd_decoder *)a)->name,
	              ((const srd_decoder *)b)->name);
}


void LogicAnalyzerChannelManager::move(int from, int to, bool after)
{
	if( from == to )
		return;
	try
	{
		auto fromElement = channel_group[from];
		auto it = channel_group.erase(channel_group.begin() + from);
		channel_group.insert(channel_group.begin() +
			to + ((after) ? 1 : 0) + ((from<to) ? -1 : 0), fromElement);
	}
	catch(const std::out_of_range& err) {
		return;
	}
}

void LogicAnalyzerChannelManager::moveChannel(
	int fromChgIndex, int from, int to, bool after)
{
	if( from ==to )
		return;
	auto fromChg = channel_group[fromChgIndex];
	auto fromElement = fromChg->get_channel(from);

	fromChg->get_channels()->erase(fromChg->get_channels()->begin() + from);
	fromChg->get_channels()->insert(fromChg->get_channels()->begin() +
		to + ((after) ? 1: 0) + ((from < to) ? -1 : 0), fromElement );
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
	currentSettingsWidget(nullptr),
	highlightShown(true)
{
	ui->setupUi(this);
	main_win = main_win_;
	this->chm = chm;
	this->la = la;
	this->chm->initDecoderList();
}

LogicAnalyzerChannelManagerUI::~LogicAnalyzerChannelManagerUI()
{
	delete ui;
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

void LogicAnalyzerChannelManagerUI::setWidgetMinimumNrOfChars(QWidget *w,
                int nrOfChars)
{
	QFontMetrics labelm(w->font());
	auto label_min_width = labelm.width(QString(nrOfChars,'X'));
	w->setMinimumWidth(label_min_width);
	w->setMaximumWidth(label_min_width+4);
}

void LogicAnalyzerChannelManagerUI::update_ui_children(LogicAnalyzerChannelGroupUI* chgroupUI)
{
	for(LogicAnalyzerChannelUI* lachannelUI : chgroupUI->ch_ui)
	{
		lachannelUI->ui->comboBox_2->clear();

		/* Populate role combo based on parent decoder */
		lachannelUI->ui->comboBox_2->addItem("None");
		for (auto var : chgroupUI->getChannelGroup()->get_decoder_roles_list()) {
			lachannelUI->ui->comboBox_2->addItem(var);
		}
		lachannelUI->getChannel()->setChannel_role(nullptr);
		lachannelUI->ui->comboBox_2->setCurrentIndex(0);
	}
}

void LogicAnalyzerChannelManagerUI::update_ui()
{
	for (auto ch : chg_ui) {
		ch->deleteLater();
	}

	if (managerHeaderWidget) {
		delete managerHeaderWidget;
		managerHeaderWidget = nullptr;
	}

	managerHeaderWidget = new QWidget(ui->headerWidget);
	Ui::LAManagerHeader *managerHeaderUI =
	        new Ui::LAManagerHeader();
	managerHeaderUI->setupUi(managerHeaderWidget);
	ui->headerWidgetLayout->addWidget(managerHeaderWidget);

	managerHeaderWidget->ensurePolished();
	ensurePolished();
	setWidgetMinimumNrOfChars(managerHeaderUI->labelName, 8);
	setWidgetMinimumNrOfChars(managerHeaderUI->labelOutput, 8);
	setWidgetMinimumNrOfChars(managerHeaderUI->indexLabel, 5);
	managerHeaderUI->label->setMinimumWidth(40);
	managerHeaderUI->labelView->setMinimumWidth(40);

	if (collapsed) {
		managerHeaderUI->rightWidget->setVisible(false);
		managerHeaderUI->leftWidget->setVisible(false);
	}
	ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);

	main_win->view_->remove_trace_clones();
	chg_ui.erase(chg_ui.begin(),chg_ui.end());
	auto offset = 0;

	for (auto&& ch : *(chm->get_channel_groups())) {
		if ((ch->is_enabled() && hidden) || !hidden) {

			LogicAnalyzerChannelGroupUI *lachannelgroupUI =
			        new LogicAnalyzerChannelGroupUI(
			        static_cast<LogicAnalyzerChannelGroup *>(ch), this, 0);

			chg_ui.push_back(lachannelgroupUI);

			ui->verticalLayout->insertWidget(chg_ui.size() - 1,
			                                 chg_ui.back());
			lachannelgroupUI->ensurePolished();
			lachannelgroupUI->ui->groupName->setText(
			        QString::fromStdString(ch->get_label()));

			/* Set no of characters to widgets */
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->groupName, 8);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->indexLabel, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox_2, 5);
			retainWidgetSizeWhenHidden(lachannelgroupUI->ui->collapseGroupBtn);

			/* Manage widget visibility  */
			if (collapsed) {
				lachannelgroupUI->ui->leftWidget->setVisible(false);
				lachannelgroupUI->ui->rightWidget->setVisible(false);
				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(0);
				setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 10);

			}

			lachannelgroupUI->ui->btnEnableChannel->setChecked(ch->is_enabled());
			lachannelgroupUI->enableControls(ch->is_enabled());

			connect(lachannelgroupUI->ui->selectCheckBox, SIGNAL(toggled(bool)),
			        chg_ui.back(), SLOT(select(bool)));
			connect(lachannelgroupUI->ui->btnEnableChannel, SIGNAL(toggled(bool)),
			        lachannelgroupUI, SLOT(enable(bool)));
			connect(lachannelgroupUI, SIGNAL(requestUpdateUI()),
				this, SLOT(triggerUpdateUi()));

			offset+=(lachannelgroupUI->geometry().bottomRight().y());

			/* Grouped widget */
			if (ch->is_grouped()) {
				/* Acquire the new decode trace */
				auto trace = main_win->view_->add_decoder();
				lachannelgroupUI->setTrace(trace);
				trace->force_to_v_offset(offset);

				if(!collapsed)
				{
					setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 15);
				}

				/* Populate role combo based on parent decoder */
				lachannelgroupUI->ui->decoderCombo->addItem("None");

				for (auto var : chm->get_name_decoder_list()) {
					lachannelgroupUI->ui->decoderCombo->addItem(var);
				}

				if (lachannelgroupUI->getChannelGroup()->getDecoder()) {
					QString name = QString::fromUtf8(
						lachannelgroupUI->getChannelGroup()->getDecoder()->name);
					int decIndex =chm->get_name_decoder_list().indexOf(name)+1;
					lachannelgroupUI->ui->decoderCombo->setCurrentIndex(decIndex);
					/* Update the pv decoder */
					set_pv_decoder(lachannelgroupUI);
				} else {
					lachannelgroupUI->ui->decoderCombo->setCurrentIndex(0);
				}

				lachannelgroupUI->ui->comboBox->setVisible(false);

				connect(lachannelgroupUI->ui->collapseGroupBtn, SIGNAL(clicked()),
				        lachannelgroupUI, SLOT(collapse_group()));
				connect(lachannelgroupUI->ui->decoderCombo,
				        SIGNAL(currentIndexChanged(const QString&)),
				        lachannelgroupUI, SLOT(decoderChanged(const QString&)));

				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(1);

				/* Create subwidgets */
				for (auto i=0; i<ch->get_channel_count(); i++) {
					LogicAnalyzerChannelUI *lachannelUI =
					        new LogicAnalyzerChannelUI(
					        static_cast<LogicAnalyzerChannel *>(
					                ch->get_channel(i)),
					        lachannelgroupUI->getChannelGroup(),
					        this);
					lachannelgroupUI->ch_ui.push_back(lachannelUI);
					lachannelUI->ensurePolished();

					/* Acquire the new signal trace */
					auto index = ch->get_channel(i)->get_id();
					auto trace = main_win->view_->get_clone_of(index);
					lachannelUI->setTrace(trace);
					trace->force_to_v_offset(offset);
					forceUpdate(lachannelUI);
					offset+=lachannelUI->geometry().bottomRight().y();

					auto str = QString::fromStdString(
					                   ch->get_channel(i)->get_label());
					lachannelUI->ui->groupName->setText(str);

					/* Set no of characters to widgets */
					setWidgetMinimumNrOfChars(lachannelUI->ui->groupName, 8);
					setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel2, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox, 5);
					setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox_2, 8);

					retainWidgetSizeWhenHidden(lachannelUI->ui->collapseGroupBtn);
					lachannelUI->ui->stackedWidget->setCurrentIndex(2);

					/* Manage widget visibility */
					if (collapsed) {
						lachannelUI->ui->leftWidget->setVisible(false);
						lachannelUI->ui->rightWidget->setVisible(false);
						lachannelUI->ui->stackedWidget->setCurrentIndex(0);
					} else {
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

					/* Populate role combo based on parent decoder */
					lachannelUI->ui->comboBox_2->addItem("None");
					for (auto var : lachannelgroupUI->getChannelGroup()->get_decoder_roles_list()) {
						lachannelUI->ui->comboBox_2->addItem(var);
					}
					if (lachannelUI->getChannel()->getChannel_role()) {
						QString name = QString::fromUtf8(
							lachannelUI->getChannel()->getChannel_role()->name);
						int roleIndex = lachannelgroupUI->getChannelGroup()->get_decoder_roles_list().indexOf(name)+1;
						lachannelUI->ui->comboBox_2->setCurrentIndex(roleIndex);
						/* Update in parent the (role, trace) map */
						lachannelgroupUI->getChannelGroup()->setChannelForDecoder(
							lachannelUI->getChannel()->getChannel_role(),
							lachannelUI->getTrace());
					} else {
						lachannelUI->ui->comboBox_2->setCurrentIndex(0);
					}

					lachannelgroupUI->ui->layoutChildren->insertWidget(i,lachannelUI);


					connect(lachannelgroupUI->ui->btnRemGroup, SIGNAL(pressed()),
					        lachannelgroupUI, SLOT(remove()));
					connect(lachannelUI->ui->btnRemGroup, SIGNAL(pressed()),
					        lachannelUI, SLOT(remove()));
					connect(lachannelUI->ui->comboBox_2,
						SIGNAL(currentIndexChanged(const QString&)),
						lachannelUI, SLOT(rolesChangedLHS(const QString&)));
					connect(lachannelUI, SIGNAL(requestUpdateUI()),
						this, SLOT(triggerUpdateUi()));



					str = QString().number(ch->get_channel(i)->get_id());
					lachannelUI->ui->indexLabel2->setText(str);
					lachannelUI->ui->indexLabel->setText(str);
				}

				if (static_cast<LogicAnalyzerChannelGroup *>(ch)->isCollapsed()) {
					lachannelgroupUI->ui->collapseGroupBtn->setChecked(true);
					lachannelgroupUI->ui->subChannelWidget->setVisible(false);
				}

				/* Update the channel roles in pv decoder */
				main_win->view_->commit_decoder_channels(trace,
					lachannelgroupUI->getChannelGroup()->get_decoder_channels());
			} else {
				auto index = ch->get_channel(0)->get_id();
				auto trace = main_win->view_->get_clone_of(index);
				lachannelgroupUI->setTrace(trace);
				trace->force_to_v_offset(offset);
				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(0);

				if (!collapsed) {
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

				connect(lachannelgroupUI->ui->comboBox,
				        SIGNAL(currentIndexChanged(int)),
				        lachannelgroupUI, SLOT(triggerChanged(int)));

			}
		}
	}

	if(highlightShown)
	{
		showHighlight(true);
	}

	if (chg_ui.size() != 0) {
		ui->scrollArea->verticalScrollBar()->setPageStep(
		        chg_ui.front()->sizeHint().height());
		ui->scrollArea->verticalScrollBar()->setSingleStep(
		        chg_ui.front()->sizeHint().height());
		ui->scrollArea->verticalScrollBar()->setRange(0,
		                ui->scrollAreaWidgetContents->height() - chg_ui.front()->sizeHint().height());
	}
	ui->scrollArea->setMaximumWidth(managerHeaderWidget->sizeHint().width());
	main_win->view_->viewport()->setDivisionHeight(44);
	main_win->view_->viewport()->setDivisionCount(10);
	main_win->view_->viewport()->setDivisionOffset(0);
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
	        this, SLOT(chmScrollChanged(int)));
}

void LogicAnalyzerChannelManagerUI::chmScrollChanged(int value)
{
	main_win->view_->set_v_offset(value);
}

void LogicAnalyzerChannelManagerUI::triggerUpdateUi()
{
	update_ui();
}

void LogicAnalyzerChannelManagerUI::collapse(bool check)
{
	collapsed = check;
	update_ui();
}

void LogicAnalyzerChannelManagerUI::remove()
{
	std::vector<int> selection = chm->get_selected_indexes();

	if (chm->get_channel_group(selection.back())->is_grouped()) {
		chm->split(selection.back());
	}

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

void LogicAnalyzerChannelManagerUI::showHighlight(bool check)
{
	LogicAnalyzerChannelGroupUI *chGroupUi = getUiFromChGroup(
				chm->getHighlightedChannelGroup());
	LogicAnalyzerChannelUI *chUi = getUiFromCh(chm->getHighlightedChannel());

	if (chGroupUi != nullptr) {
		setDynamicProperty(chGroupUi->ui->baseWidget, "highlight", check);
		if(chGroupUi->getTrace())
		{
			chGroupUi->getTrace()->set_highlight(check);
		}
	}

	if (chUi != nullptr) {
		setDynamicProperty(chUi->ui->baseWidget, "highlight", check);
		if(chUi->getTrace())
		{
			chUi->getTrace()->set_highlight(check);
		}
	}
	if (check)
	{
		deleteSettingsWidget();
		createSettingsWidget();
		highlightShown = true;
	}
	else
	{
		highlightShown = false;
	}

}

LogicAnalyzerChannelGroupUI *LogicAnalyzerChannelManagerUI::getUiFromChGroup(
        LogicAnalyzerChannelGroup *seek)
{
	for (auto groupUI : chg_ui)
		if (groupUI->getChannelGroup() == seek) {
			return groupUI;
		}

	return nullptr;
}

LogicAnalyzerChannelUI *LogicAnalyzerChannelManagerUI::getUiFromCh(
        LogicAnalyzerChannel *seek)
{
	for (auto groupUI : chg_ui)
		if (groupUI->getChannelGroup()->get_channel_count()!=0)
			for (auto channelUI : groupUI->ch_ui)
				if (channelUI->get_channel() == seek) {
					return channelUI;
				}

	return nullptr;
}

/**
 * @brief LogicAnalyzerChannelManagerUI::rolesChangedRHS
 * If the combo boxes from the right menu are connected to a ChannelGroupUI,
 * the connection is lost once update_ui() is called. So, they will be connected
 * to a ChannelManagerUI, which is not affected by the update.
 * @param text
 */
void LogicAnalyzerChannelManagerUI::rolesChangedRHS(const QString text)
{
	QComboBox *comboSender = (static_cast<QComboBox *>(QObject::sender()));
	const QString role_name = comboSender->property("name").toString();

	auto chGroup = chm->getHighlightedChannelGroup();
	if( chGroup )
	{
		auto chgroupUI = getUiFromChGroup(chGroup);
		if( text == "-" )
		{
			auto prevChannel = chgroupUI->findChannelWithRole(role_name);
			if( prevChannel )
			{
				prevChannel->channelRoleChanged("None");
			}
		}
		else
		{
			int channel_id = text.toInt();
			for(LogicAnalyzerChannelUI *var : chgroupUI->ch_ui)
			{
				if(var->get_channel()->get_id() == channel_id)
				{
					var->channelRoleChanged(role_name);
				}
			}
		}
		highlightShown = false;
		update_ui();
	}
}


void LogicAnalyzerChannelManagerUI::createSettingsWidget()
{
	settingsUI = new Ui::LASettingsWidget;
	settingsUI->setupUi(locationSettingsWidget);
	currentSettingsWidget = new QWidget(locationSettingsWidget);
	settingsUI->setupUi(currentSettingsWidget);
	locationSettingsWidget->layout()->addWidget(currentSettingsWidget);
	ensurePolished();
	connect(settingsUI->btnNext, SIGNAL(pressed()),
		this, SLOT(highlightNext()));
	connect(settingsUI->btnPrevious, SIGNAL(pressed()),
		this, SLOT(highlightPrevious()));

	if (chm->getHighlightedChannelGroup()) {
		LogicAnalyzerChannelGroup *chGroup = chm->getHighlightedChannelGroup();
		settingsUI->nameLineEdit->setText(QString::fromStdString(chGroup->get_label()));
		connect(settingsUI->nameLineEdit, &QLineEdit::editingFinished,
			[=]() {
				QString text = settingsUI->nameLineEdit->text();
				getUiFromChGroup(chm->getHighlightedChannelGroup())->
					ui->groupName->setText(text);
				set_label(text);
			});

		if (chGroup->is_grouped()) {
			const srd_decoder *decoder = chGroup->getDecoder();

			if (!decoder) {
				settingsUI->requiredChn->hide();
				settingsUI->optionalChn->hide();
				locationSettingsWidget->setVisible(true);
				return;
			}

			/* Create widgets for required channels */
			GSList *reqCh = g_slist_copy(decoder->channels);
			if( g_slist_length(reqCh) > 0 )
				settingsUI->requiredChn->show();
			else
				settingsUI->requiredChn->hide();

			for (; reqCh; reqCh = reqCh->next) {
				const srd_channel *const rqch = (const srd_channel *)reqCh->data;

				if (rqch == nullptr) {
					break;
				}

				Ui::LARequiredChannel *reqChUI = new Ui::LARequiredChannel();
				QWidget *r = new QWidget(currentSettingsWidget);
				reqChUI->setupUi(r);
				reqChUI->labelRole->setText(QString::fromUtf8(rqch->name));
				reqChUI->stackedWidget->setCurrentIndex(0);

				reqChUI->roleCombo->addItem("-");
				for (auto&& ch : *(chGroup->get_channels())) {
					reqChUI->roleCombo->addItem(QString::number(ch->get_id()));
					if(chGroup->getChannelById(ch->get_id())->getChannel_role() &&
						(QString::fromUtf8(chGroup->getChannelById(ch->get_id())->getChannel_role()->name)
							== rqch->name))
					{
						reqChUI->roleCombo->setCurrentText(QString::number(ch->get_id()));
					}
				}
				reqChUI->roleCombo->setProperty("id", QVariant(rqch->id));
				reqChUI->roleCombo->setProperty("name", QVariant(rqch->name));
				settingsUI->scrollAreaWidgetLayout->insertWidget(
					settingsUI->scrollAreaWidgetLayout->count() - 2, r);

				connect(reqChUI->roleCombo,
				        SIGNAL(currentIndexChanged(const QString&)),
				        this, SLOT(rolesChangedRHS(const QString&)));

			}
			free(reqCh);

			/* Create widgets for optional channels */
			GSList *optChannels = g_slist_copy(decoder->opt_channels);
			if( g_slist_length(optChannels) > 0)
				settingsUI->optionalChn->show();
			else
				settingsUI->optionalChn->hide();

			for (; optChannels; optChannels = optChannels->next) {
				const srd_channel *const optch = (const srd_channel *)optChannels->data;

				if (optch == nullptr) {
					break;
				}

				Ui::LARequiredChannel *optChUI = new Ui::LARequiredChannel();
				QWidget *r = new QWidget(currentSettingsWidget);
				optChUI->setupUi(r);
				optChUI->labelRole->setText(QString::fromUtf8(optch->name));
				optChUI->stackedWidget->setCurrentIndex(0);

				/* Add all the available channels + "none" to the list */
				optChUI->roleCombo->addItem("-");
				for (auto&& ch : *(chGroup->get_channels())) {
					optChUI->roleCombo->addItem(QString::number(ch->get_id()));
					if(chGroup->getChannelById(ch->get_id())->getChannel_role() &&
						(QString::fromUtf8(chGroup->getChannelById(ch->get_id())->getChannel_role()->name)
							== optch->name))
					{
						optChUI->roleCombo->setCurrentText(QString::number(ch->get_id()));
					}
				}
				optChUI->roleCombo->setProperty("id", QVariant(optch->id));
				optChUI->roleCombo->setProperty("name", QVariant(optch->name));
				settingsUI->scrollAreaWidgetLayout->insertWidget(
					settingsUI->scrollAreaWidgetLayout->count() - 1, r);

				connect(optChUI->roleCombo,
				        SIGNAL(currentIndexChanged(const QString&)),
				        this, SLOT(rolesChangedRHS(const QString&)));
			}
			free(optChannels);
		}
		else {
			settingsUI->requiredChn->hide();
			settingsUI->optionalChn->hide();
		}

	}

	if (chm->getHighlightedChannel()) {
		settingsUI->requiredChn->hide();
		settingsUI->optionalChn->hide();
		LogicAnalyzerChannel *ch = chm->getHighlightedChannel();
		settingsUI->nameLineEdit->setText(QString::fromStdString(ch->get_label()));
		connect(settingsUI->nameLineEdit, &QLineEdit::editingFinished,
			[=]() {
			QString text = settingsUI->nameLineEdit->text();
			getUiFromCh(chm->getHighlightedChannel())->
				ui->groupName->setText(text);
			set_label(text);
		});

		QWidget *r = new QWidget(currentSettingsWidget);
		Ui::LARequiredChannel *roleChUI = new Ui::LARequiredChannel();
		roleChUI->setupUi(r);
		if(ch->getChannel_role())
		{
			roleChUI->labelChRole->setText(QString::fromUtf8(ch->getChannel_role()->name));
		}
		else
		{
			roleChUI->labelChRole->setText("-");
		}
		roleChUI->stackedWidget->setCurrentIndex(1);

		settingsUI->scrollAreaWidgetLayout->insertWidget(
			settingsUI->scrollAreaWidgetLayout->count() - 1, r);
	}

	locationSettingsWidget->setVisible(true);
}

void LogicAnalyzerChannelManagerUI::deleteSettingsWidget()
{
	if (settingsUI) {
		locationSettingsWidget->setVisible(false);
		delete currentSettingsWidget;
		currentSettingsWidget = nullptr;
		delete settingsUI;
		settingsUI = nullptr;
	}
}

void LogicAnalyzerChannelManagerUI::highlightPrevious()
{
	bool update = false;
	if( chm->getHighlightedChannelGroup() ) {
		auto chgroup = chm->getHighlightedChannelGroup();
		auto chgIter = std::find(chm->get_channel_groups()->begin(),
			chm->get_channel_groups()->end(), chgroup);
		auto chgIndex = chgIter - chm->get_channel_groups()->begin();
		if( chgIter != chm->get_channel_groups()->end() ) {
			if( chgIndex - 1 >= 0 ) {
				auto prevChgroup = chm->get_channel_group(chgIndex - 1);
				chm->highlightChannel(prevChgroup);
				update = true;
			}
		}
	}
	if( chm->getHighlightedChannel() ) {
		auto ch = chm->getHighlightedChannel();
		auto chUI = getUiFromCh(ch);
		auto chgroup = chUI->getChannelGroup();
		auto chIter = std::find(chgroup->get_channels()->begin(),
			chgroup->get_channels()->end(), ch);
		auto chIndex = chIter - chgroup->get_channels()->begin();
		if( chIter != chgroup->get_channels()->end() ) {
			if( chIndex - 1 >= 0 ) {
				auto prevCh = chgroup->get_channel(chIndex - 1);
				chm->highlightChannel(nullptr,
					static_cast<LogicAnalyzerChannel*>(prevCh));
				update = true;
			}
		}
	}
	if(update)
		update_ui();
}

void LogicAnalyzerChannelManagerUI::highlightNext()
{
	bool update = false;
	if( chm->getHighlightedChannelGroup() ) {
		auto chgroup = chm->getHighlightedChannelGroup();
		auto chgIter = std::find(chm->get_channel_groups()->begin(),
			chm->get_channel_groups()->end(), chgroup);
		auto chgIndex = chgIter - chm->get_channel_groups()->begin();
		if( chgIter != chm->get_channel_groups()->end() ) {
			if( chgIndex + 1 < chm->get_channel_groups()->size() ) {
				auto nextChgroup = chm->get_channel_group(chgIndex + 1);
				chm->highlightChannel(nextChgroup);
				update = true;
			}
		}
	}
	if( chm->getHighlightedChannel() ) {
		auto ch = chm->getHighlightedChannel();
		auto chUI = getUiFromCh(ch);
		auto chgroup = chUI->getChannelGroup();
		auto chIter = std::find(chgroup->get_channels()->begin(),
			chgroup->get_channels()->end(), ch);
		auto chIndex = chIter - chgroup->get_channels()->begin();
		if( chIter != chgroup->get_channels()->end() ) {
			if( chIndex + 1 < chgroup->get_channel_count() ) {
				auto prevCh = chgroup->get_channel(chIndex + 1);
				chm->highlightChannel(nullptr,
					static_cast<LogicAnalyzerChannel*>(prevCh));
				update = true;
			}
		}
	}
	if(update)
		update_ui();
}

void LogicAnalyzerChannelManagerUI::set_label(QString text)
{
	if (chm->getHighlightedChannelGroup()) {
		chm->getHighlightedChannelGroup()->set_label(text.toStdString());
	}

	if (chm->getHighlightedChannel()) {
		chm->getHighlightedChannel()->set_label(text.toStdString());
	}
}

void LogicAnalyzerChannelManagerUI::set_pv_decoder(LogicAnalyzerChannelGroupUI
                *chGroup)
{
	main_win->view_->set_decoder_to_group(chGroup->getTrace(),
	                                      chGroup->getChannelGroup()->getDecoder());
}
}
