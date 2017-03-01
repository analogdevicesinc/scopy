#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "pulseview/pv/view/decodetrace.hpp"
#include "pulseview/pv/view/logicsignal.hpp"
#include "la_channel_manager.hpp"
#include "logic_analyzer.hpp"
#include "dynamicWidget.hpp"
#include "ui_la_channel_group.h"
#include "ui_la_channel_manager.h"
#include "ui_la_manager_header.h"
#include "ui_la_settings.h"
#include "ui_la_decoder_reqChannel.h"
#include "ui_logic_channel_settings.h"
#include <QScrollBar>
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <QPainter>

using std::dynamic_pointer_cast;

namespace pv {
class MainWindow;
namespace view {
class View;
class Viewport;
class TraceTreeItem;
class DecodeTrace;
class LogicSignal;
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
                LogicAnalyzerChannelGroupUI *chgroupui,
                LogicAnalyzerChannelManagerUI *chm_ui,
                QWidget *parent) :
	ChannelUI(ch,parent),
	ui(new Ui::LAChannelGroup)
{
	this->ui->setupUi(this);
	this->lch = ch;
	this->chgroup = chgroup;
	this->chgroupui = chgroupui;
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
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	} else {
		chm_ui->setHoverWidget(this);
	}
	if((event->pos() - dragStartPosition).manhattanLength()
		< QApplication::startDragDistance())
		{
			return;
		}

	chm_ui->clearHoverWidget();

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
	auto w = ui->baseWidget->geometry().width();
	auto h = ui->baseWidget->geometry().height();

	topDragbox.setRect(0, 0, w, h/2);
	botDragbox.setRect(0, h/2, w, h/2);

	if( event->mimeData()->hasFormat("la/channelgroup"))
	{
		short from = (short)event->mimeData()->data("la/channelgroup")[1];
		auto channelGroups = chm_ui->chm->get_channel_groups();
		auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
		auto chgIndex = chgIter - channelGroups->begin();

		if( from == chgIndex ){
			event->ignore();
			return;
		}
	}
	event->accept();
}

void LogicAnalyzerChannelUI::dragMoveEvent(QDragMoveEvent *event)
{
	if( event->answerRect().intersects(botDragbox) ) {
		chm_ui->clearHoverWidget();
		chgroupui->resetSeparatorHighlight(true);
		highlightBotSeparator();
		event->accept();
	}
	else if(event->answerRect().intersects(topDragbox)) {
		chm_ui->clearHoverWidget();
		chgroupui->resetSeparatorHighlight(true);
		highlightTopSeparator();
		event->accept();
	}
	else if(event->answerRect().intersects(centerDragbox)) {
		chm_ui->clearHoverWidget();
		chgroupui->resetSeparatorHighlight(true);
		event->accept();
	}
	else {
		chgroupui->resetSeparatorHighlight(true);
		event->ignore();
	}
}

void LogicAnalyzerChannelUI::dragLeaveEvent(QDragLeaveEvent *event)
{
	resetSeparatorHighlight();
	event->accept();
}

void LogicAnalyzerChannelUI::enterEvent(QEvent *event)
{
	chm_ui->setHoverWidget(this);
	QWidget::enterEvent(event);
}

void LogicAnalyzerChannelUI::leaveEvent(QEvent *event)
{
	chm_ui->clearHoverWidget();
	QWidget::leaveEvent(event);
}

void LogicAnalyzerChannelUI::dropEvent(QDropEvent *event)
{
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
		auto fromChannel = chm_ui->chm->get_channel_group(fromChg)->get_channel(fromCh);
		short fromChGlobal = fromChannel->get_id();

		if( fromChg != chgIndex ) {

			chm_ui->chm->splitChannel(fromChg, fromCh);
			if( fromNrOfChannels == 1 && chgIndex > fromChg)
				chgIndex--;
			chm_ui->chm->join({(int)chgIndex, fromChGlobal});
			chm_ui->chm->split(chgIndex);

			chm_ui->chm->move(chm_ui->chm->get_channel_groups()->size() - 1, chgIndex - 1);
		}
		else {
			chgIndex = chgIndex + (chgIndex > fromChg ? -1 : 0);
			auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chgroup);
			chm_ui->chm->moveChannel(chgIndex, fromCh, chIndex, dropAfter);
		}
	}

	Q_EMIT requestUpdateUI();
}

void LogicAnalyzerChannelUI::updateTrace()
{
	if (trace==nullptr) {
		auto index = lch->get_id();
		auto trace1 = chm_ui->main_win->view_->get_clone_of(index);
		setTrace(trace1);
	}

	auto height = geometry().height();
	auto v_offset = topSep->geometry().bottomRight().y() + 3 + height -
	                (trace->v_extents().second) + chgroupui->getTraceOffset()+3;//chgOffset.y();

	if (traceOffset!=v_offset || traceHeight!=height) {
		traceHeight = height;
		traceOffset = v_offset;
		trace->setSignal_height(traceHeight);
		trace->force_to_v_offset(v_offset);
	}

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
	if (channel_role)
		chgroup->setChannelForDecoder(channel_role, getChannel()->get_id());
	chgroupui->setupDecoder();
}

void LogicAnalyzerChannelUI::resetSeparatorHighlight(bool force)
{
	topSep->setVisible(false);
	botSep->setVisible(false);
}

void LogicAnalyzerChannelUI::highlightTopSeparator()
{
	resetSeparatorHighlight();
	topSep->setVisible(true);
}

void LogicAnalyzerChannelUI::highlightBotSeparator()
{
	resetSeparatorHighlight();
	botSep->setVisible(true);
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
}

void LogicAnalyzerChannelUI::setTrace(
        std::shared_ptr<pv::view::LogicSignal> item)
{
	trace = item;
}

std::shared_ptr<pv::view::LogicSignal> LogicAnalyzerChannelUI::getTrace()
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

	if( decoder == nullptr )
		return;

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
	channels_ = std::map<const srd_channel*, uint16_t>();
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup():
	ChannelGroup()
{
	collapsed = false;
	decoder = nullptr;
	channels_ = std::map<const srd_channel*, uint16_t>();
}

const srd_channel* LogicAnalyzerChannelGroup::findByValue(uint16_t ch_id)
{
	for( auto it : channels_ )
	{
		if( it.second == ch_id )
			return it.first;
	}
	return nullptr;
}

void LogicAnalyzerChannelGroup::setChannelForDecoder(const srd_channel* ch,
		uint16_t ch_id)
{
	if(!channels_.empty()){
		auto itByKey = channels_.find(ch);
		auto itByValue = (findByValue(ch_id) == nullptr) ? channels_.end() : channels_.find(findByValue(ch_id));
		if( itByKey != channels_.end() && itByValue != channels_.end() && itByKey != itByValue) {
			getChannelById(itByKey->second)->setChannel_role(nullptr);
			channels_.at(ch) = ch_id;
			itByValue->second = -1;
		}
		else if( itByKey != channels_.end() ) {
			getChannelById(itByKey->second)->setChannel_role(nullptr);
			channels_.at(ch) = ch_id;
		}
		else if( itByValue != channels_.end() ) {
			itByValue->second = -1;
			channels_.insert(std::pair<const srd_channel*, uint16_t>(ch, ch_id));
		}
		else {
			channels_.insert(std::pair<const srd_channel*, uint16_t>(ch, ch_id));
		}
	}
	else
	{
		channels_.insert(std::pair<const srd_channel*, uint16_t>(ch, ch_id));
	}
}

std::map<const srd_channel*, uint16_t>
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
	trace = nullptr;
	decodeTrace = nullptr;
}

void LogicAnalyzerChannelGroupUI::set_decoder(std::string value)
{
//	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(value);
//	qDebug()<<QString().fromUtf8(lchg->getDecoder()->name);
}

void LogicAnalyzerChannelGroupUI::setTrace(
        std::shared_ptr<pv::view::LogicSignal> item)
{
	trace = dynamic_pointer_cast<pv::view::TraceTreeItem>(item);
	logicTrace = item;
}

void LogicAnalyzerChannelGroupUI::setTrace(
        std::shared_ptr<pv::view::DecodeTrace> item)
{
	trace = dynamic_pointer_cast<pv::view::TraceTreeItem>(item);
	decodeTrace = item;
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

void LogicAnalyzerChannelGroupUI::enterEvent(QEvent *event)
{
	chm_ui->setHoverWidget(this);
	QWidget::enterEvent(event);
}

void LogicAnalyzerChannelGroupUI::leaveEvent(QEvent *event)
{
	chm_ui->clearHoverWidget();
	QWidget::leaveEvent(event);
}

void LogicAnalyzerChannelGroupUI::highlightTopSeparator()
{
	resetSeparatorHighlight(true);
	topSep->setVisible(true);
}

void LogicAnalyzerChannelGroupUI::highlightBotSeparator()
{
	resetSeparatorHighlight(true);
	botSep->setVisible(true);
}

void LogicAnalyzerChannelGroupUI::resetSeparatorHighlight(bool force)
{
	if (force || (!chg->is_grouped())) {
		topSep->setVisible(false);
		botSep->setVisible(false);
	} else {
		topSep->setVisible(true);
		botSep->setVisible(true);
	}
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

	chm_ui->clearHoverWidget();

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
	auto w = ui->baseWidget->geometry().width();
	auto h = ui->baseWidget->geometry().height();
	topDragbox.setRect(0, 0, w, h/3);
	botDragbox.setRect(0, 2*h/3, w, h/3);
	centerDragbox.setRect(0, h/3, w, h/3);
	event->accept();
}

void LogicAnalyzerChannelGroupUI::dragMoveEvent(QDragMoveEvent *event)
{
	if( event->answerRect().intersects(botDragbox) ) {
		chm_ui->clearHoverWidget();
		resetSeparatorHighlight(true);

		if( lchg->is_grouped() )
			ch_ui[0]->highlightTopSeparator();
		else
			highlightBotSeparator();
		event->accept();
	}
	else if(event->answerRect().intersects(centerDragbox)) {
		chm_ui->setHoverWidget(this);
		resetSeparatorHighlight();
		event->accept();
	}
	else if(event->answerRect().intersects(topDragbox)) {
		chm_ui->clearHoverWidget();
		resetSeparatorHighlight(true);
		highlightTopSeparator();
		event->accept();
	}
	else {
		resetSeparatorHighlight();
		event->ignore();
	}
}

void LogicAnalyzerChannelGroupUI::dragLeaveEvent(QDragLeaveEvent *event)
{
	resetSeparatorHighlight();
	chm_ui->clearHoverWidget();
	event->accept();
}

int LogicAnalyzerChannelGroupUI::getTraceOffset()
{
	return traceOffset;
}

void LogicAnalyzerChannelGroupUI::updateTrace()
{
	if (trace==nullptr && decodeTrace==nullptr) {

		if (!chg->is_grouped()) {
			auto index = chg->get_channel(0)->get_id();
			auto trace1 = chm_ui->main_win->view_->get_clone_of(index);
			setTrace(trace1);
		} else {
			auto trace1 = chm_ui->main_win->view_->add_decoder();
			setTrace(trace1);
			setupDecoder();
		}
	}

	auto chgOffset =  geometry().top()+ui->baseWidget->geometry().bottom() + 3;
	auto height = ui->baseWidget->geometry().height();
	auto v_offset = chgOffset - trace->v_extents().second;

	if (traceOffset!=v_offset || traceHeight!=height) {
		traceHeight = height;
		traceOffset = v_offset;
		trace->force_to_v_offset(v_offset);
		trace->setSignal_height(traceHeight);
	}

	if (chg->is_grouped() && !getChannelGroup()->isCollapsed())
		for (auto &&ch : ch_ui) {
			ch->updateTrace();
		}
}

void LogicAnalyzerChannelGroupUI::setupDecoder()
{
	auto decoder = lchg->getDecoder();
	if( decoder != nullptr ) {
		decodeTrace->set_decoder(decoder);
		std::map<const srd_channel *,
				std::shared_ptr<pv::view::TraceTreeItem> > channel_map;

		for(auto id : lchg->get_ids()) {
			LogicAnalyzerChannel* lch = lchg->getChannelById(id);
			std::pair<const srd_channel *,
				std::shared_ptr<pv::view::TraceTreeItem> > chtracepair;
			if( lch->getChannel_role() ) {
				chtracepair.first = lch->getChannel_role();
				chtracepair.second = chm_ui->main_win->view_->get_clone_of(id);
				chtracepair.second->force_to_v_offset(-100);
				channel_map.insert(chtracepair);
			}

		}
		decodeTrace->set_channel_map(channel_map);
	}
}

void LogicAnalyzerChannelGroupUI::dropEvent(QDropEvent *event)
{
	resetSeparatorHighlight();

	if( event->source() == this && event->possibleActions() & Qt::MoveAction)
		return;

	auto channelGroups = chm_ui->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(), channelGroups->end(), chg);
	auto chgIndex = chgIter - channelGroups->begin();

	bool dropAfter = false;
	bool formGroup = false;

	if( botDragbox.contains(event->pos())) {
		if( lchg->is_grouped() ) {
			formGroup = true;
		}
		else {
			dropAfter = true;
		}
	} else if( topDragbox.contains(event->pos())) {
		dropAfter = false;
	} else if(centerDragbox.contains(event->pos())) {
		formGroup = true;
	} else {
		Q_EMIT requestUpdateUI();
		return;
	}

	if( formGroup && event->mimeData()->hasFormat("la/channelgroup")) {
		short from = (short)event->mimeData()->data("la/channelgroup")[1];
		chm_ui->chm->join({(int)chgIndex, from});
		chgIndex = chgIndex + ((from < chgIndex) ? -1 : 0);
		auto chgNOfChannels = chm_ui->chm->get_channel_group(
			chgIndex)->get_channel_count();
		chm_ui->chm->moveChannel(chgIndex, chgNOfChannels-1, 0, false);
		chm_ui->chm->highlightChannel(chm_ui->chm->get_channel_group(chgIndex));
	}
	else {
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
	}

	Q_EMIT requestUpdateUI();
}

void LogicAnalyzerChannelGroupUI::collapse_group()
{
	bool value = !getChannelGroup()->isCollapsed();
	getChannelGroup()->collapse(value);
	ui->subChannelWidget->setVisible(!value);
	Q_EMIT requestUpdateUI();
}

void LogicAnalyzerChannelGroupUI::enableControls(bool enabled)
{
	ui->btnRemGroup->setEnabled(enabled);
	ui->decoderCombo->setEnabled(enabled);
	ui->groupName->setEnabled(enabled);
	ui->comboBox_2->setEnabled(enabled);
	ui->comboBox->setEnabled(enabled);
	ui->indexLabel->setEnabled(enabled);
	if( lchg->is_grouped() ) {
		ui->subChannelWidget->setEnabled(enabled);
	}
	chm_ui->updatePlot();
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
			chm_ui->la->setHWTrigger(ch->get_id(), trigger_val);
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
	highlightShown(true),
	hoverWidget(nullptr)
{
	ui->setupUi(this);
	main_win = main_win_;
	this->chm = chm;
	this->la = la;
	this->chm->initDecoderList();
	showHighlight(false);
	chm->highlightChannel(chm->get_channel_group(0));
	showHighlight(true);
	ui->scrollAreaWidgetContents->installEventFilter(this);
	Q_EMIT(widthChanged(geometry().width()));
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

QFrame *LogicAnalyzerChannelManagerUI::addSeparator(QVBoxLayout *lay,
                int pos)
{
	separators.push_back(new QFrame(this));
	QFrame *line = separators.back();
	line->setFrameShadow(QFrame::Plain);
	line->setLineWidth(1);
	line->setFrameShape(QFrame::HLine);
	line->setStyleSheet("color: rgba(255,255,255,50);");
	lay->insertWidget(pos,line);
	retainWidgetSizeWhenHidden(line);
	line->setVisible(false);
	return line;
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
		ch->hide();
		// prevent hovering after the channel is deleted
		ch->setMouseTracking(false);
		ch->deleteLater();
	}

	if (managerHeaderWidget) {
		delete managerHeaderWidget;
		managerHeaderWidget = nullptr;
	}

	for (auto sep : separators)    {
		sep->deleteLater();
	}
	separators.erase(separators.begin(),separators.end());

	hoverWidget = nullptr;
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

	QFrame *prevSep = addSeparator(ui->verticalLayout,
		ui->verticalLayout->count()-1);
	auto offset = 0;

	for (auto&& ch : *(chm->get_channel_groups())) {
		if ((ch->is_enabled() && hidden) || !hidden) {

			LogicAnalyzerChannelGroupUI *lachannelgroupUI =
			        new LogicAnalyzerChannelGroupUI(
			        static_cast<LogicAnalyzerChannelGroup *>(ch), this, 0);

			chg_ui.push_back(lachannelgroupUI);

			ui->verticalLayout->insertWidget(ui->verticalLayout->count()-1,
			                                 chg_ui.back());
			lachannelgroupUI->ensurePolished();
			lachannelgroupUI->topSep = prevSep;

			lachannelgroupUI->ui->groupName->setText(
			        QString::fromStdString(ch->get_label()));

			/* Set no of characters to widgets */
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->groupName, 8);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->indexLabel, 5);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->comboBox, 5);
			lachannelgroupUI->ui->comboBox->setIconSize(QSize(30,20));
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
				lachannelgroupUI->chUiSep = addSeparator(
							lachannelgroupUI->ui->layoutChildren,
							lachannelgroupUI->ui->layoutChildren->count() );
				prevSep = lachannelgroupUI->chUiSep;

				if(!collapsed)
				{
					setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 17);
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
					        lachannelgroupUI,
					        this);
					lachannelgroupUI->ch_ui.push_back(lachannelUI);
					lachannelUI->ensurePolished();

					auto index = ch->get_channel(i)->get_id();

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
					lachannelUI->ui->comboBox->setIconSize(QSize(30, 20));
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
					}

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
					} else {
						lachannelUI->ui->comboBox_2->setCurrentIndex(0);
					}

					lachannelgroupUI->ui->layoutChildren->insertWidget(
						lachannelgroupUI->ui->layoutChildren->count(), lachannelUI);
					lachannelgroupUI->ensurePolished();

					lachannelUI->botSep = addSeparator(
						lachannelgroupUI->ui->layoutChildren,
						lachannelgroupUI->ui->layoutChildren->count());
					lachannelUI->topSep = prevSep;
					prevSep =  lachannelUI->botSep;


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

				lachannelgroupUI->botSep = prevSep;
				lachannelgroupUI->resetSeparatorHighlight();
			} else {
				auto index = ch->get_channel(0)->get_id();
				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(0);

				if (!collapsed) {
					retainWidgetSizeWhenHidden(lachannelgroupUI->ui->btnRemGroup);
				}

				lachannelgroupUI->ui->btnRemGroup->setVisible(false);
				lachannelgroupUI->ui->collapseGroupBtn->setVisible(false);

				lachannelgroupUI->botSep = addSeparator(ui->verticalLayout,
					ui->verticalLayout->count()-1);
				prevSep = lachannelgroupUI->botSep;
				lachannelgroupUI->chUiSep = lachannelgroupUI->botSep;

				lachannelgroupUI->ui->decoderCombo->setVisible(false);
				lachannelgroupUI->ui->indexLabel->setText(QString::number(index));

				connect(lachannelgroupUI->ui->comboBox,
				        SIGNAL(currentIndexChanged(int)),
				        lachannelgroupUI, SLOT(triggerChanged(int)));
				connect(lachannelgroupUI->ui->comboBox,
				        SIGNAL(currentIndexChanged(int)),
				        la, SLOT(triggerChanged(int)));

			}
		}
	}

	if(highlightShown)
	{
		showHighlight(true);
	}

	ui->scrollArea->setMaximumWidth(managerHeaderWidget->sizeHint().width());
	main_win->view_->viewport()->setDivisionCount(10);
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
	        this, SLOT(chmScrollChanged(int)));
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
	        this, SLOT(chmRangeChanged(int,int)));
}

void LogicAnalyzerChannelManagerUI::chmScrollChanged(int value)
{
	main_win->view_->set_v_offset(value);
}

void LogicAnalyzerChannelManagerUI::chmRangeChanged(int min, int max)
{
	main_win->view_->verticalScrollBar()->setMinimum(min);
	main_win->view_->verticalScrollBar()->setMaximum(max);
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
	Ui::LChannelSettings *generalSettingsUi = new Ui::LChannelSettings;
	QWidget* generalSettings = new QWidget(locationSettingsWidget);
	generalSettingsUi->setupUi(generalSettings);
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
		generalSettingsUi->nameLineEdit->setText(QString::fromStdString(chGroup->get_label()));
		connect(generalSettingsUi->nameLineEdit, &QLineEdit::editingFinished,
			[=]() {
				QString text = generalSettingsUi->nameLineEdit->text();
				getUiFromChGroup(chm->getHighlightedChannelGroup())->
					ui->groupName->setText(text);
				set_label(text);
			});
		settingsUI->scrollAreaWidgetLayout->insertWidget(
			settingsUI->scrollAreaWidgetLayout->count()-1, generalSettings);

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
					settingsUI->scrollAreaWidgetLayout->count() - 3, r);

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
					settingsUI->scrollAreaWidgetLayout->count() - 2, r);

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
		generalSettingsUi->nameLineEdit->setText(QString::fromStdString(ch->get_label()));
		connect(generalSettingsUi->nameLineEdit, &QLineEdit::editingFinished,
			[=]() {
			QString text = generalSettingsUi->nameLineEdit->text();
			getUiFromCh(chm->getHighlightedChannel())->
				ui->groupName->setText(text);
			set_label(text);
		});

		settingsUI->scrollAreaWidgetLayout->insertWidget(
			settingsUI->scrollAreaWidgetLayout->count()-1, generalSettings);

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
			settingsUI->scrollAreaWidgetLayout->count() - 2, r);
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
				showHighlight(false);
				chm->highlightChannel(prevChgroup);
				showHighlight(true);
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
				showHighlight(false);
				chm->highlightChannel(nullptr,
					static_cast<LogicAnalyzerChannel*>(prevCh));
				showHighlight(true);
				update = true;
			}
		}
	}
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
				showHighlight(false);
				chm->highlightChannel(nextChgroup);
				showHighlight(true);
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
				showHighlight(false);
				chm->highlightChannel(nullptr,
					static_cast<LogicAnalyzerChannel*>(prevCh));
				showHighlight(true);
				update = true;
			}
		}
	}
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

void LogicAnalyzerChannelManagerUI::paintEvent(QPaintEvent *event)
{
	Q_EMIT(widthChanged(geometry().width()));
}

void LogicAnalyzerChannelManagerUI::setHoverWidget(QWidget *hover)
{
	if (hoverWidget!=nullptr) {
		setDynamicProperty(hoverWidget,"hover-property",false);
	}

	hoverWidget = hover->findChild<QWidget *>("baseWidget");

	if (hoverWidget) {
		setDynamicProperty(hoverWidget,"hover-property",true);
	}
}

void LogicAnalyzerChannelManagerUI::clearHoverWidget()
{
	if (hoverWidget!=nullptr) {
		setDynamicProperty(hoverWidget,"hover-property",false);
		hoverWidget = nullptr;
	}
}

void LogicAnalyzerChannelManagerUI::updatePlot()
{
	for (auto chgUi : chg_ui) {
		chgUi->updateTrace();
	}
}

bool LogicAnalyzerChannelManagerUI::eventFilter(QObject *object,
                QEvent *event)
{
	if (event->type()==QEvent::LayoutRequest) {
		updatePlot();
	}

	return false;
}

}
