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
#include <QListView>
#include <QFormLayout>
#include "pulseview/pv/widgets/colourbutton.hpp"
#include "pulseview/pv/view/tracepalette.hpp"
#include "pulseview/pv/binding/decoder.hpp"
#include "scroll_filter.hpp"

using std::dynamic_pointer_cast;

namespace pv {
class MainWindow;
namespace view {
class View;
class Viewport;
class TraceTreeItem;
class DecodeTrace;
class LogicSignal;
class TracePalette;
}
namespace binding {
class Decoder;
}
}


namespace adiscope {


LogicAnalyzerChannel::LogicAnalyzerChannel(uint16_t id_,
                std::string label_) :
	Channel(id_,label_),
	channel_role(nullptr),
	trigger("none"),
	ch_thickness(1.0)
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

qreal LogicAnalyzerChannel::getCh_thickness() const
{
	return ch_thickness;
}

void LogicAnalyzerChannel::setCh_thickness(qreal value)
{
	ch_thickness = value;
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
	this->installEventFilter(this);

	std::string trigger_val = chm_ui->chm->get_channel(get_channel()->get_id())->getTrigger();
	for(int i = 0; i < chm_ui->getTriggerMapping().size(); i++)
	{
		if( trigger_val == chm_ui->getTriggerMapping(i) )
		{
			getChannel()->setTrigger(trigger_val);
			ui->comboBox->setCurrentIndex(i);
			chm_ui->la->setHWTrigger(get_channel()->get_id(), trigger_val);
		}
	}

	QAbstractItemView *trigVw = ui->comboBox->view();
	QListView* listVw = qobject_cast<QListView*>(trigVw);
	listVw->setSpacing(2);
}

LogicAnalyzerChannelUI::~LogicAnalyzerChannelUI()
{
	if( ui->baseWidget == chm_ui->hoverWidget ){
		chm_ui->hoverWidget = nullptr;
	}
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

		for(int i = 0; i < fromNrOfChannels; i++) {
			/* Check for duplicates */
			auto chToBeAdded = chm_ui->chm->get_channel_group(from)->get_channel(i);
			auto ids = chm_ui->chm->get_channel_group(chgIndex)->get_ids();
			if( std::find(ids.begin(), ids.end(), chToBeAdded->get_id()) == ids.end()) {
				Channel *ch = new LogicAnalyzerChannel(
					chToBeAdded->get_id(), chToBeAdded->get_label());
				chm_ui->chm->get_channel_group(chgIndex)->add_channel(ch);
			}
		}
		chm_ui->chm->highlightChannel(chm_ui->chm->get_channel_group(chgIndex));

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
	bool highlighted = chm_ui->getUiFromCh(
		chm_ui->chm->getHighlightedChannel()) == this;
	if(highlighted != trace->get_highlight()) {
		trace->set_highlight(highlighted);
	}

	auto height = geometry().height();
	auto v_offset = topSep->geometry().bottomRight().y() + 3 + height -
	                (trace->v_extents().second) + chgroupui->getTraceOffset()+3;//chgOffset.y();

	if(trace) {
		trace->setBgcolour(get_channel()->getBgcolor());
		trace->setEdgecolour(get_channel()->getEdgecolor());
		trace->setHighcolour(get_channel()->getHighcolor());
		trace->setLowcolour(get_channel()->getLowcolor());
	}

	if (traceOffset!=v_offset || traceHeight!=height) {
		traceHeight = height;
		traceOffset = v_offset;
		trace->setSignal_height(traceHeight);
		trace->force_to_v_offset(v_offset);
	}
	chm_ui->main_win->view_->time_item_appearance_changed(true, true);
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

void LogicAnalyzerChannelUI::enableControls(bool enabled)
{
	ui->groupName->setEnabled(enabled);
	ui->comboBox->setEnabled(enabled);
	ui->indexLabel->setEnabled(enabled);
	ui->indexLabel2->setEnabled(enabled);
	trace->visible(enabled);
}

bool LogicAnalyzerChannelUI::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat("la/channelgroup")
				&& !enterEvent->mimeData()->hasFormat("la/channel"))
			return true;
		}
	if (event->type() == QEvent::Drop){
		QDropEvent *dropEvent = static_cast<QDropEvent *>(event);
		if (!dropEvent->mimeData()->hasFormat("la/channelgroup")
				&& !dropEvent->mimeData()->hasFormat("la/channel"))
			return true;
		}
	if (event->type() == QEvent::DragMove){
		QDragMoveEvent *dropEvent = static_cast<QDragMoveEvent *>(event);
		if (!dropEvent->mimeData()->hasFormat("la/channelgroup")
				&& !dropEvent->mimeData()->hasFormat("la/channel"))
			return true;
		}
	return QWidget::event(event);
}

LogicAnalyzerChannelGroup* LogicAnalyzerChannelUI::getChannelGroup()
{
	return chgroup;
}

void LogicAnalyzerChannelUI::triggerChanged(int index)
{
	std::string trigger_val = chm_ui->getTriggerMapping(index);
	if( trigger_val != getChannel()->getTrigger() )
	{
		chm_ui->chm->get_channel(get_channel()->get_id())->setTrigger(trigger_val);
		chm_ui->la->setHWTrigger(get_channel()->get_id(), trigger_val);
		chm_ui->la->setTriggerCache(get_channel()->get_id(), trigger_val);
	}
	Q_EMIT requestUpdateUI();
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
	if( value == nullptr )
		return;
	if( value ==  decoder )
		return;

	decoderRolesNameList.clear();
	decoderReqChannels.clear();
	decoderOptChannels.clear();
	properties_.clear();
	channels_.clear();
	decoder = value;

	GSList *reqCh = g_slist_copy(decoder->channels);
	for (; reqCh; reqCh = reqCh->next) {
		const srd_channel *const rqch = (const srd_channel *)reqCh->data;

		if (rqch == nullptr) {
			break;
		}
		decoderRolesNameList << QString::fromUtf8(rqch->name);
		decoderReqChannels.push_back(rqch);
	}
	g_slist_free(reqCh);

	GSList *optChannels = g_slist_copy(decoder->opt_channels);
	for (; optChannels; optChannels = optChannels->next) {
		const srd_channel *const optch = (const srd_channel *)optChannels->data;

		if (optch == nullptr) {
			break;
		}
		decoderRolesNameList << QString::fromUtf8(optch->name);
		decoderOptChannels.push_back(optch);
	}
	g_slist_free(optChannels);
}

LogicAnalyzerChannel* LogicAnalyzerChannelGroup::get_channel_by_id(int id)
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

LogicAnalyzerChannel* LogicAnalyzerChannelGroup::get_channel_at_index(int index)
{
	auto channels = get_channels();
	if(index < channels->size()) {
		return static_cast<LogicAnalyzerChannel *>(channels->at(index));
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
	ch_thickness = 1.0;
}

LogicAnalyzerChannelGroup::LogicAnalyzerChannelGroup():
	ChannelGroup()
{
	collapsed = false;
	decoder = nullptr;
	channels_ = std::map<const srd_channel*, uint16_t>();
	ch_thickness = 1.0;
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

qreal LogicAnalyzerChannelGroup::getCh_thickness() const
{
	return ch_thickness;
}

void LogicAnalyzerChannelGroup::setCh_thickness(qreal value)
{
	ch_thickness = value;
}

void LogicAnalyzerChannelGroup::setChannelForDecoder(const srd_channel* ch,
		uint16_t ch_id)
{
	if(!channels_.empty()){
		auto itByKey = channels_.find(ch);
		auto itByValue = (findByValue(ch_id) == nullptr) ? channels_.end() : channels_.find(findByValue(ch_id));
		if( itByKey != channels_.end() && itByValue != channels_.end() && itByKey != itByValue) {
			if( get_channel_by_id(itByKey->second))
				get_channel_by_id(itByKey->second)->setChannel_role(nullptr);
			channels_.at(ch) = ch_id;
			itByValue->second = -1;
		}
		else if( itByKey != channels_.end() ) {
			if( get_channel_by_id(itByKey->second))
				get_channel_by_id(itByKey->second)->setChannel_role(nullptr);
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
	channels_.clear();
	properties_.clear();
}

const srd_channel* LogicAnalyzerChannelGroup::get_srd_channel_from_name(const char* name)
{
	for (auto var : decoderOptChannels) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	for (auto var : decoderReqChannels) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	return nullptr;
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
	setAcceptDrops(true);
	this->installEventFilter(this);

	/* Set triggerCombo index according to the device */
	LogicAnalyzerChannel *ch;
	if( !lchg->is_grouped() )
	{
		ch = static_cast<LogicAnalyzerChannel*>(lchg->get_channel());
		std::string trigger_val = chm_ui->chm->get_channel(ch->get_id())->getTrigger();
		for(int i = 0; i < chm_ui->getTriggerMapping().size(); i++)
		{
			if( trigger_val == chm_ui->getTriggerMapping(i) )
			{
				ch->setTrigger(trigger_val);
				ui->comboBox->setCurrentIndex(i);
				chm_ui->la->setHWTrigger(ch->get_id(), trigger_val);
			}
		}
	}

	trace = nullptr;
	decodeTrace = nullptr;

	QAbstractItemView *trigVw = ui->comboBox->view();
	QListView* listVw = qobject_cast<QListView*>(trigVw);
	listVw->setSpacing(2);
}

LogicAnalyzerChannelGroupUI::~LogicAnalyzerChannelGroupUI()
{
	if( ui->baseWidget == chm_ui->hoverWidget ){
		chm_ui->hoverWidget = nullptr;
	}
	delete ui;
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

std::shared_ptr<pv::view::DecodeTrace> LogicAnalyzerChannelGroupUI::getDecodeTrace()
{
	return decodeTrace;
}

bool LogicAnalyzerChannelGroupUI::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat("la/channelgroup")
				&& !enterEvent->mimeData()->hasFormat("la/channel"))
			return true;
		}
	if (event->type() == QEvent::Drop){
		QDropEvent *dropEvent = static_cast<QDropEvent *>(event);
		if (!dropEvent->mimeData()->hasFormat("la/channelgroup")
				&& !dropEvent->mimeData()->hasFormat("la/channel"))
			return true;
		}
	if (event->type() == QEvent::DragMove){
		QDragMoveEvent *dropEvent = static_cast<QDragMoveEvent *>(event);
		if (!dropEvent->mimeData()->hasFormat("la/channelgroup")
				&& !dropEvent->mimeData()->hasFormat("la/channel"))
			return true;
		}
	return QWidget::event(event);
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

		if( lchg->is_grouped() ) {
			if(lchg->isCollapsed()) {
				highlightBotSeparator();
			}
			else {
				ch_ui[0]->highlightTopSeparator();
			}
		}
		else {
			highlightBotSeparator();
		}
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
	auto chgOffset =  geometry().top()+ui->baseWidget->geometry().bottom() + 3;
	auto height = ui->baseWidget->geometry().height();
	auto v_offset = chgOffset - trace->v_extents().second;

	bool highlighted = (chm_ui->getUiFromChGroup(
	                            chm_ui->chm->getHighlightedChannelGroup()) == this);

	highlighted &= chm_ui->chm->getHighlightedChannel() == nullptr;

	if (highlighted != trace->get_highlight()) {
		trace->set_highlight(highlighted);
	}

	if(logicTrace) {
		logicTrace->setEdgecolour(chg->getEdgecolor());
		logicTrace->setHighcolour(chg->getHighcolor());
		logicTrace->setLowcolour(chg->getLowcolor());
		logicTrace->setBgcolour(chg->getBgcolor());
	}
	if(decodeTrace) {
		decodeTrace->setBgcolour(chg->getBgcolor());
	}

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
	chm_ui->main_win->view_->time_item_appearance_changed(true, true);
}

void LogicAnalyzerChannelGroupUI::setupDecoder()
{
	auto decoder = lchg->getDecoder();
	if( decoder != nullptr ) {
		decodeTrace->set_decoder(decoder);
		std::map<const srd_channel *,
				std::shared_ptr<pv::view::TraceTreeItem> > channel_map;

		for(auto id : lchg->get_ids()) {
			LogicAnalyzerChannel* lch = lchg->get_channel_by_id(id);
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
		if( lchg->is_grouped() && !(lchg->isCollapsed())) {
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
	ui->decoderCombo->setEnabled(enabled);
	ui->groupName->setEnabled(enabled);
	ui->comboBox_2->setEnabled(enabled);
	ui->comboBox->setEnabled(enabled);
	ui->indexLabel->setEnabled(enabled);
	trace->visible(enabled);
	for (auto &&ch : ch_ui) {
		ch->enableControls(enabled);
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

	if(decoder)
	{
		QLayoutItem* item;
		while ( ( item = ui->ann_row_layout->takeAt( 0 ) ) != NULL )
		{
		     delete item->widget();
		     delete item;
		}
		for(auto l = decoder->annotation_rows; l ;l=l->next){
			auto label = new QLabel("",this);
			label->setStyleSheet("");
			ui->ann_row_layout->addWidget(label);
		}
	}
	static_cast<LogicAnalyzerChannelGroup *>(chg)->setDecoder(decoder);
	chm_ui->update_ui_children(this);
}

void LogicAnalyzerChannelGroupUI::triggerChanged(int index)
{
	LogicAnalyzerChannel *ch;
	if( !lchg->is_grouped() )
	{
		ch = static_cast<LogicAnalyzerChannel*>(lchg->get_channel());
		std::string trigger_val = chm_ui->getTriggerMapping(index);
		if( trigger_val != ch->getTrigger() )
		{
			chm_ui->chm->get_channel(ch->get_id())->setTrigger(trigger_val);
			chm_ui->la->setHWTrigger(ch->get_id(), trigger_val);
			chm_ui->la->setTriggerCache(ch->get_id(), trigger_val);
		}
	}
	Q_EMIT requestUpdateUI();
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
		lchannels.push_back(static_cast<LogicAnalyzerChannel*>(ch));
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
	if(index < channel_group.size())
		return static_cast<LogicAnalyzerChannelGroup *>(
			channel_group[index]);
	return nullptr;
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

LogicAnalyzerChannel *LogicAnalyzerChannelManager::get_channel(int index)
{
	if(index < lchannels.size())
		return lchannels[index];
	return nullptr;
//	return static_cast<LogicAnalyzerChannel *>(channel[index]);
}

void LogicAnalyzerChannelManager::add_channel_group(
		LogicAnalyzerChannelGroup *chg)
{
	channel_group.push_back(chg);
}

void LogicAnalyzerChannelManager::clearChannelGroups()
{
	for (auto ch : channel_group) {
		delete ch;
	}
	channel_group.erase(channel_group.begin(),channel_group.end());
}

void LogicAnalyzerChannelManager::clearTrigger()
{
	for(auto ch : lchannels) {
		ch->setTrigger("None");
	}
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
	g_slist_free(dL);
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
                QVBoxLayout *settingsLayout,
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
	hoverWidget(nullptr),
	generalSettings(nullptr),
	locationSettingsLayout(settingsLayout),
	streaming_mode(false)
{
	ui->setupUi(this);
	main_win = main_win_;
	this->chm = chm;
	this->la = la;
	this->chm->initDecoderList();
	this->trigger_mapping = la->get_iio_trigger_options();
	createColorButtons();
	showHighlight(false);
	chm->highlightChannel(chm->get_channel_group(0));
	showHighlight(true);
	ui->scrollAreaWidgetContents->installEventFilter(this);
	eventFilterGuard = new MouseWheelWidgetGuard(this);
	Q_EMIT(widthChanged(geometry().width()));
}

std::vector<std::string> LogicAnalyzerChannelManagerUI::getTriggerMapping()
{
	return trigger_mapping;
}

std::string LogicAnalyzerChannelManagerUI::getTriggerMapping(int i)
{
	if(i < trigger_mapping.size())
		return trigger_mapping[i];
	return "";
}

void LogicAnalyzerChannelManagerUI::createColorButtons()
{
	generalSettingsUi = new Ui::LChannelSettings();
	generalSettings = new QWidget(locationSettingsWidget);
	generalSettingsUi->setupUi(generalSettings);
	locationSettingsLayout->insertWidget(locationSettingsLayout->count()-1, generalSettings);

	/*Add color buttons */
	colour_button_edge = new ColourButton(
		pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
		generalSettingsUi->colorEdge);
	colour_button_edge->set_palette(pv::view::TracePalette::Colours);
	colour_button_edge->setProperty("type", QVariant("edge"));

	colour_button_BG = new ColourButton(
		pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
		generalSettingsUi->colorBG, true);
	colour_button_BG->set_palette(pv::view::TracePalette::Colours);
	colour_button_BG->setProperty("type", QVariant("background"));

	colour_button_low = new ColourButton(
		pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
		generalSettingsUi->colorLow);
	colour_button_low->set_palette(pv::view::TracePalette::Colours);
	colour_button_low->setProperty("type", QVariant("low"));

	colour_button_high = new ColourButton(
		pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
		generalSettingsUi->colorHigh);
	colour_button_high->set_palette(pv::view::TracePalette::Colours);
	colour_button_high->setProperty("type", QVariant("high"));

	connect(generalSettingsUi->cmbThickness, SIGNAL(currentTextChanged(QString)),
		this, SLOT(chThicknessChanged(QString)));
	connect(generalSettingsUi->btnCollapse, &QPushButton::clicked,
		[=](bool check) {
			if(check)
				generalSettingsUi->widget->hide();
			else
				generalSettingsUi->widget->show();
		});

	connect(colour_button_edge, SIGNAL(selected(const QColor)),
		this, SLOT(colorChanged(QColor)));
	connect(colour_button_BG, SIGNAL(selected(const QColor)),
		this, SLOT(colorChanged(QColor)));
	connect(colour_button_low, SIGNAL(selected(const QColor)),
		this, SLOT(colorChanged(QColor)));
	connect(colour_button_high, SIGNAL(selected(const QColor)),
		this, SLOT(colorChanged(QColor)));

	connect(generalSettingsUi->nameLineEdit, &QLineEdit::editingFinished,
		[=]() {
		QString text = generalSettingsUi->nameLineEdit->text();
		if(chm->getHighlightedChannel()) {
			getUiFromCh(chm->getHighlightedChannel())->
				ui->groupName->setText(text);
		}
		else if(chm->getHighlightedChannelGroup()) {
			getUiFromChGroup(chm->getHighlightedChannelGroup())->
				ui->groupName->setText(text);
		}
		set_label(text);
	});
}

LogicAnalyzerChannelManagerUI::~LogicAnalyzerChannelManagerUI()
{
	delete ui;
	for (auto ch : decChannelsUi) {
		delete ch;
	}
	decChannelsUi.erase(decChannelsUi.begin(),decChannelsUi.end());
	if(settingsUI)
		delete settingsUI;
	if(generalSettingsUi)
		delete generalSettingsUi;
	if(managerHeaderUI)
		delete managerHeaderUI;
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

void LogicAnalyzerChannelManagerUI::set_streaming_mode(bool en)
{
	streaming_mode = en;
}

bool LogicAnalyzerChannelManagerUI::is_streaming_mode()
{
	return streaming_mode;
}

void LogicAnalyzerChannelManagerUI::update_ui_children(LogicAnalyzerChannelGroupUI* chgroupUI)
{
	int index = 0;
	for(LogicAnalyzerChannelUI* lachannelUI : chgroupUI->ch_ui)
	{
		lachannelUI->ui->comboBox_2->clear();

		/* Populate role combo based on parent decoder */
		lachannelUI->ui->comboBox_2->addItem("None");
		for (auto var : chgroupUI->getChannelGroup()->get_decoder_roles_list()) {
			lachannelUI->ui->comboBox_2->addItem(var);
		}

		if(auto dec = chgroupUI->getChannelGroup()->getDecoder()) {
			if(strcmp(dec->id, "parallel") == 0) {
				lachannelUI->ui->comboBox_2->setCurrentText(
					chgroupUI->getChannelGroup()->get_decoder_roles_list().at(index+1));
				lachannelUI->ui->comboBox_2->setEnabled(false);
			}
			else {
				lachannelUI->ui->comboBox_2->setEnabled(true);
				if(lachannelUI->getChannel()->getChannel_role())
				{
					QString name = QString::fromUtf8(
								lachannelUI->getChannel()->getChannel_role()->name);
					int roleIndex = chgroupUI->getChannelGroup()->get_decoder_roles_list().indexOf(name)+1;
					lachannelUI->ui->comboBox_2->setCurrentIndex(roleIndex);
				}
				else
					lachannelUI->ui->comboBox_2->setCurrentIndex(0);
			}
		}
		else {
			lachannelUI->ui->comboBox_2->setEnabled(true);
			lachannelUI->getChannel()->setChannel_role(nullptr);
			lachannelUI->ui->comboBox_2->setCurrentIndex(0);
		}
		index++;
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
		delete managerHeaderUI;
		managerHeaderUI = nullptr;
	}

	for (auto sep : separators)    {
		sep->deleteLater();
	}
	separators.erase(separators.begin(),separators.end());

	hoverWidget = nullptr;
	managerHeaderWidget = new QWidget(ui->headerWidget);
	managerHeaderUI = new Ui::LAManagerHeader();
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
			if(!lachannelgroupUI->ui->collapseGroupBtn->isChecked())
				retainWidgetSizeWhenHidden(lachannelgroupUI->ui->collapseGroupBtn);
			setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 10);
			/* Manage widget visibility  */
			if (collapsed) {
				lachannelgroupUI->ui->leftWidget->setVisible(false);
				lachannelgroupUI->ui->rightWidget->setVisible(false);
				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(0);

			}

			connect(lachannelgroupUI->ui->selectCheckBox, SIGNAL(toggled(bool)),
			        chg_ui.back(), SLOT(select(bool)));
			connect(lachannelgroupUI, SIGNAL(requestUpdateUI()),
				this, SLOT(triggerUpdateUi()));

			offset+=(lachannelgroupUI->geometry().bottomRight().y());

			/* Grouped widget */
			if (ch->is_grouped()) {
				if(!collapsed) {
					setWidgetMinimumNrOfChars(lachannelgroupUI->ui->decoderCombo, 10);
				}

				auto trace1 = main_win->view_->add_decoder();
				lachannelgroupUI->setTrace(trace1);
				lachannelgroupUI->setupDecoder();

				/* Populate role combo based on parent decoder */
				lachannelgroupUI->ui->decoderCombo->addItem("None");
				int decoder_char_count = QString("None").count();

				for (auto var : chm->get_name_decoder_list()) {
					lachannelgroupUI->ui->decoderCombo->addItem(var);
					decoder_char_count = (var.count() > decoder_char_count) ? var.count() : decoder_char_count;
				}

				QFontMetrics labelm(lachannelgroupUI->ui->decoderCombo->font());
				auto label_min_width = labelm.width(QString(decoder_char_count+2, 'X'));
				lachannelgroupUI->ui->decoderCombo->view()->setMinimumWidth(label_min_width);

				connect(lachannelgroupUI->ui->decoderCombo,
					SIGNAL(currentIndexChanged(const QString&)),
					lachannelgroupUI, SLOT(decoderChanged(const QString&)));
				connect(lachannelgroupUI->ui->btnRemGroup, SIGNAL(pressed()),
					lachannelgroupUI, SLOT(remove()));

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


				lachannelgroupUI->ui->stackedWidget->setCurrentIndex(1);

				if(!lachannelgroupUI->getChannelGroup()->isCollapsed()) {
					lachannelgroupUI->chUiSep = addSeparator(
						lachannelgroupUI->ui->layoutChildren,
						lachannelgroupUI->ui->layoutChildren->count() );
					prevSep = lachannelgroupUI->chUiSep;

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
						auto trace1 = main_win->view_->get_clone_of(index);
						lachannelUI->setTrace(trace1);

						forceUpdate(lachannelUI);
						offset+=lachannelUI->geometry().bottomRight().y();

						auto str = QString::fromStdString(
									ch->get_channel(i)->get_label());
						lachannelUI->ui->groupName->setText(str);

						/* Set no of characters to widgets */
						setWidgetMinimumNrOfChars(lachannelUI->ui->groupName, 8);
						setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel2, 3);
						setWidgetMinimumNrOfChars(lachannelUI->ui->indexLabel, 3);
						setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox, 5);
						lachannelUI->ui->comboBox->setIconSize(QSize(30, 20));
						setWidgetMinimumNrOfChars(lachannelUI->ui->comboBox_2, 5);

						if(!lachannelUI->ui->collapseGroupBtn->isChecked())
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
						prevSep = lachannelUI->botSep;

						connect(lachannelUI->ui->btnRemGroup, SIGNAL(pressed()),
							lachannelUI, SLOT(remove()));
						connect(lachannelUI->ui->comboBox_2,
							SIGNAL(currentTextChanged(const QString&)),
							lachannelUI, SLOT(rolesChangedLHS(const QString&)));
						connect(lachannelUI, SIGNAL(requestUpdateUI()),
							this, SLOT(triggerUpdateUi()));
						connect(lachannelUI->ui->comboBox,
							SIGNAL(currentIndexChanged(int)),
							lachannelUI, SLOT(triggerChanged(int)));
						connect(lachannelUI->ui->comboBox,
							SIGNAL(currentIndexChanged(int)),
							la, SLOT(triggerChanged(int)));

						str = QString().number(ch->get_channel(i)->get_id());
						lachannelUI->ui->indexLabel2->setText(str);
						lachannelUI->ui->indexLabel->setText(str);
					}

					int id = 0;
					for(auto ch : lachannelgroupUI->ch_ui) {
						auto dec = lachannelgroupUI->getChannelGroup()->getDecoder();
						if(!dec)
							break;

						if(strcmp(dec->id, "parallel") != 0)
							break;

						ch->ui->comboBox_2->clear();

						/* Populate role combo based on parent decoder */
						ch->ui->comboBox_2->addItem("None");
						for (auto var : lachannelgroupUI->getChannelGroup()->get_decoder_roles_list()) {
							ch->ui->comboBox_2->addItem(var);
						}

						ch->ui->comboBox_2->setCurrentText(
									lachannelgroupUI->getChannelGroup()->get_decoder_roles_list().at(id+1));
						ch->ui->comboBox_2->setEnabled(false);
						id++;
					}

					lachannelgroupUI->ui->layoutChildren->removeWidget(prevSep);
					lachannelgroupUI->botSep = addSeparator(ui->verticalLayout,
							ui->verticalLayout->count()-1);
					prevSep = lachannelgroupUI->botSep;
					lachannelgroupUI->ch_ui.back()->botSep = prevSep;
					prevSep = lachannelgroupUI->botSep;
				}
				else {
					lachannelgroupUI->ui->collapseGroupBtn->setChecked(true);
					lachannelgroupUI->ui->subChannelWidget->setVisible(false);
					lachannelgroupUI->botSep = addSeparator(ui->verticalLayout,
							ui->verticalLayout->count()-1);
					prevSep = lachannelgroupUI->botSep;
				}
				lachannelgroupUI->resetSeparatorHighlight();
			} else {
				auto index = ch->get_channel(0)->get_id();
				auto trace1 = main_win->view_->get_clone_of(index);
				lachannelgroupUI->setTrace(trace1);
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
			lachannelgroupUI->ui->btnEnableChannel->setChecked(ch->is_enabled());
			lachannelgroupUI->enableControls(ch->is_enabled());
			connect(lachannelgroupUI->ui->btnEnableChannel, SIGNAL(toggled(bool)),
				lachannelgroupUI, SLOT(enable(bool)));
		}
	}

	showHighlight(true);

	ui->scrollArea->setMaximumWidth(managerHeaderWidget->sizeHint().width());
	main_win->view_->viewport()->setDivisionCount(10);
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
	        this, SLOT(chmScrollChanged(int)));
	connect(ui->scrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
	        this, SLOT(chmRangeChanged(int,int)));
	la->get_channel_groups_api();

	if(!eventFilterGuard)
		eventFilterGuard = new MouseWheelWidgetGuard(this);
	eventFilterGuard->installEventRecursively(this);
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
	int val = ui->scrollArea->verticalScrollBar()->value();
	update_ui();
	ui->scrollArea->verticalScrollBar()->setValue(val);
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

void LogicAnalyzerChannelManagerUI::groupSplit_clicked()
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
	ui->scrollArea->verticalScrollBar()->setValue(
		ui->scrollArea->verticalScrollBar()->maximum());
	update_ui();
}

void LogicAnalyzerChannelManagerUI::hideInactive_clicked(bool hide)
{
	hidden = hide;
	update_ui();
	auto chg = chm->getHighlightedChannelGroup();
	auto ch = chm->getHighlightedChannel();
	if(getUiFromCh(ch)==nullptr && getUiFromChGroup(chg)==nullptr) {
		enableCgSettings(false);
	}
	else {
		enableCgSettings(true);
	}
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
	if (check && (chUi || chGroupUi))
	{
		deleteSettingsWidget();
		createSettingsWidget();
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
					disconnect(var->ui->comboBox_2,
						SIGNAL(currentTextChanged(const QString&)),
						var, SLOT(rolesChangedLHS(const QString&)));
					var->ui->comboBox_2->setCurrentText(role_name);
					connect(var->ui->comboBox_2,
						SIGNAL(currentTextChanged(const QString&)),
						var, SLOT(rolesChangedLHS(const QString&)));
					return;
				}
			}
		}
	}
}

void LogicAnalyzerChannelManagerUI::chThicknessChanged(QString text)
{
	bool ok;
	double value = text.toDouble(&ok);
	if( !ok )
		return;
	auto chg = chm->getHighlightedChannelGroup();
	auto ch = chm->getHighlightedChannel();

	auto chgui = getUiFromChGroup(chg);
	auto chui = getUiFromCh(ch);

	if( chg) {
		chg->setCh_thickness(value);
		if(chg->is_grouped()) {
			for(LogicAnalyzerChannelUI* c : chgui->ch_ui){
				c->getChannel()->setCh_thickness(value);
				c->getTrace()->setCh_thickness(value);
			}
		}
		else {
			chgui->getTrace()->setCh_thickness(value);
		}
	}
	else {
		ch->setCh_thickness(value);
		chui->getTrace()->setCh_thickness(value);
	}
	main_win->view_->time_item_appearance_changed(false, true);
}

void LogicAnalyzerChannelManagerUI::colorChanged(QColor color)
{
	auto ch = chm->getHighlightedChannel();
	auto chg = chm->getHighlightedChannelGroup();

	auto chgui = getUiFromChGroup(chg);
	auto chui = getUiFromCh(ch);

	QPushButton* sender = static_cast<QPushButton *>(QObject::sender());
	QString type = sender->property("type").toString();
	if( type == "background") {
		if(!ch) {
			chg->setBgcolor(color);
			chgui->updateTrace();
		}
		else {
			ch->setBgcolor(color);
			chui->updateTrace();
		}
	}
	else if( type == "edge") {
		if(!ch) {
			chg->setEdgecolor(color);
			chgui->updateTrace();
		}
		else {
			ch->setEdgecolor(color);
			chui->updateTrace();
		}
	}
	else if( type == "low") {
		if(!ch) {
			chg->setLowcolor(color);
			chgui->updateTrace();
		}
		else {
			ch->setLowcolor(color);
			chui->updateTrace();
		}
	}
	else if( type == "high") {
		if(!ch) {
			chg->setHighcolor(color);
			chgui->updateTrace();
		}
		else {
			ch->setHighcolor(color);
			chui->updateTrace();
		}
	}
}

void LogicAnalyzerChannelManagerUI::showColorSettings(bool check)
{
	if(check) {
		generalSettingsUi->colorEdge->show();
		generalSettingsUi->colorLow->show();
		generalSettingsUi->colorHigh->show();
		generalSettingsUi->lblEdge->show();
		generalSettingsUi->lblLow->show();
		generalSettingsUi->lblHigh->show();
	}
	else {
		generalSettingsUi->colorEdge->hide();
		generalSettingsUi->colorLow->hide();
		generalSettingsUi->colorHigh->hide();
		generalSettingsUi->lblEdge->hide();
		generalSettingsUi->lblLow->hide();
		generalSettingsUi->lblHigh->hide();
	}
}

void LogicAnalyzerChannelManagerUI::enableCgSettings(bool en)
{
	generalSettingsUi->cmbThickness->setEnabled(en);
	generalSettingsUi->nameLineEdit->setEnabled(en);
	colour_button_BG->setEnabled(en);
	colour_button_edge->setEnabled(en);
	colour_button_high->setEnabled(en);
	colour_button_low->setEnabled(en);
}


void LogicAnalyzerChannelManagerUI::createSettingsWidget()
{
	settingsUI = new Ui::LASettingsWidget();
	currentSettingsWidget = new QWidget(locationSettingsWidget);
	locationSettingsLayout->insertWidget(locationSettingsLayout->count()-2,
		currentSettingsWidget);
	settingsUI->setupUi(currentSettingsWidget);
	ensurePolished();

	connect(settingsUI->btnNext, SIGNAL(pressed()),
		this, SLOT(highlightNext()));
	connect(settingsUI->btnPrevious, SIGNAL(pressed()),
		this, SLOT(highlightPrevious()));

	auto chg = chm->getHighlightedChannelGroup();
	auto ch = chm->getHighlightedChannel();
	if(getUiFromCh(ch)==nullptr && getUiFromChGroup(chg)==nullptr) {
		enableCgSettings(false);
		return;
	}
	else {
		enableCgSettings(true);
	}

	if (chm->getHighlightedChannelGroup()) {
		LogicAnalyzerChannelGroup *chGroup = chm->getHighlightedChannelGroup();
		LogicAnalyzerChannelGroupUI *chGroupUI = getUiFromChGroup(chGroup);
		settingsUI->nameLineEdit->setText(QString::fromStdString(chGroup->get_label()));
		generalSettingsUi->nameLineEdit->setText(QString::fromStdString(chGroup->get_label()));
		QString ch_thickness = QString::number(chGroup->getCh_thickness());
		generalSettingsUi->cmbThickness->setCurrentText(ch_thickness);

		colour_button_BG->set_colour(chGroup->getBgcolor());
		colour_button_edge->set_colour(chGroup->getEdgecolor());
		colour_button_high->set_colour(chGroup->getHighcolor());
		colour_button_low->set_colour(chGroup->getLowcolor());
		showColorSettings(!chGroup->is_grouped());

		if (chGroup->is_grouped()) {
			const srd_decoder *decoder = chGroup->getDecoder();

			if (!decoder) {
				settingsUI->requiredChn->hide();
				settingsUI->optionalChn->hide();
				settingsUI->options->hide();
				locationSettingsWidget->setVisible(true);
				return;
			}

			/* Create widgets for required channels */
			if(chGroup->decoderReqChannels.size() > 0)
				settingsUI->requiredChn->show();
			else
				settingsUI->requiredChn->hide();

			for (auto rqch : chGroup->decoderReqChannels) {

				if (rqch == nullptr) {
					break;
				}

				decChannelsUi.push_back(new Ui::LARequiredChannel());
				auto reqChUI = decChannelsUi.back();
				QWidget *r = new QWidget(currentSettingsWidget);
				reqChUI->setupUi(r);
				reqChUI->labelRole->setText(QString::fromUtf8(rqch->name));
				reqChUI->stackedWidget->setCurrentIndex(0);

				reqChUI->roleCombo->addItem("-");
				for (auto&& ch : *(chGroup->get_channels())) {
					reqChUI->roleCombo->addItem(QString::number(ch->get_id()));
					if(chGroup->get_channel_by_id(ch->get_id())->getChannel_role() &&
						(QString::fromUtf8(chGroup->get_channel_by_id(ch->get_id())->getChannel_role()->name)
							== rqch->name))
					{
						reqChUI->roleCombo->setCurrentText(QString::number(ch->get_id()));
					}
				}
				reqChUI->roleCombo->setProperty("id", QVariant(rqch->id));
				reqChUI->roleCombo->setProperty("name", QVariant(rqch->name));
				settingsUI->verticalLayout_1->insertWidget(
					settingsUI->verticalLayout_1->count() - 2, r);

				connect(reqChUI->roleCombo,
				        SIGNAL(currentIndexChanged(const QString&)),
				        this, SLOT(rolesChangedRHS(const QString&)));
			}

			/* Create widgets for optional channels */
			if(chGroup->decoderOptChannels.size() > 0)
				settingsUI->optionalChn->show();
			else
				settingsUI->optionalChn->hide();

			int index = 0;
			for (auto optch : chGroup->decoderOptChannels) {

				if (optch == nullptr) {
					break;
				}

				decChannelsUi.push_back(new Ui::LARequiredChannel());
				auto optChUI = decChannelsUi.back();
				QWidget *r = new QWidget(currentSettingsWidget);
				optChUI->setupUi(r);
				optChUI->labelRole->setText(QString::fromUtf8(optch->name));
				optChUI->stackedWidget->setCurrentIndex(0);

				if(strcmp(decoder->id, "parallel") == 0 &&
						strcmp(optch->name, "CLK") != 0) {
					auto currentCh = chGroup->get_channel_at_index(index);
					if(auto role = currentCh->getChannel_role()) {
						if(optch->name == role->name) {
							optChUI->roleCombo->addItem(QString::number(currentCh->get_id()));
							optChUI->roleCombo->setCurrentIndex(0);
							optChUI->roleCombo->setEnabled(false);
						}
					}

					index++;
					goto display_widget;
				}

				/* Add all the available channels + "none" to the list */
				optChUI->roleCombo->addItem("-");
				for (auto&& ch : *(chGroup->get_channels())) {
					optChUI->roleCombo->addItem(QString::number(ch->get_id()));
					if(chGroup->get_channel_by_id(ch->get_id())->getChannel_role() &&
						(QString::fromUtf8(chGroup->get_channel_by_id(ch->get_id())->getChannel_role()->name)
							== optch->name))
					{
						optChUI->roleCombo->setCurrentText(QString::number(ch->get_id()));
					}
				}

			display_widget:

				optChUI->roleCombo->setProperty("id", QVariant(optch->id));
				optChUI->roleCombo->setProperty("name", QVariant(optch->name));

				settingsUI->verticalLayout_1->insertWidget(
					settingsUI->verticalLayout_1->count()-1, r);

				connect(optChUI->roleCombo,
				        SIGNAL(currentIndexChanged(const QString&)),
				        this, SLOT(rolesChangedRHS(const QString&)));

				if( index >= chGroup->get_channel_count() )
					break;
			}

			/* Create widgets for options */
			if(!decoder->options) {
				settingsUI->options->hide();
			}
			else {
				binding_ = std::make_shared<binding::Decoder>(
							chGroupUI->getDecodeTrace()->decoder(),
							chGroupUI->getDecodeTrace()->pv_decoder(),
							chGroup->properties_);
				QFormLayout *layOpt = new QFormLayout(settingsUI->options);
				chGroup->properties_ = binding_->properties();
				binding_->add_properties_to_form(layOpt, true, settingsUI->options);
				layOpt->setMargin(0);
				settingsUI->verticalLayout_1->insertLayout(
					settingsUI->verticalLayout_1->count(), layOpt);
			}
		}
		else {
			settingsUI->requiredChn->hide();
			settingsUI->optionalChn->hide();
			settingsUI->options->hide();
		}
	}

	if (chm->getHighlightedChannel()) {
		settingsUI->requiredChn->hide();
		settingsUI->optionalChn->hide();
		LogicAnalyzerChannel *ch = chm->getHighlightedChannel();
		settingsUI->nameLineEdit->setText(QString::fromStdString(ch->get_label()));
		generalSettingsUi->nameLineEdit->setText(QString::fromStdString(ch->get_label()));
		QString ch_thickness = QString::number(ch->getCh_thickness());
		generalSettingsUi->cmbThickness->setCurrentText(ch_thickness);

		showColorSettings(true);

		colour_button_BG->set_colour(ch->getBgcolor());
		colour_button_edge->set_colour(ch->getEdgecolor());
		colour_button_high->set_colour(ch->getHighcolor());
		colour_button_low->set_colour(ch->getLowcolor());

		decChannelsUi.push_back(new Ui::LARequiredChannel());
		auto roleChUI = decChannelsUi.back();
		QWidget *r = new QWidget(currentSettingsWidget);
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

		settingsUI->verticalLayout_1->insertWidget(
			settingsUI->verticalLayout_1->count() - 1, r);
		settingsUI->options->hide();
	}

	locationSettingsWidget->setVisible(true);
	la->installWheelEventGuard();
}

void LogicAnalyzerChannelManagerUI::deleteSettingsWidget()
{
	if (settingsUI) {
		locationSettingsWidget->setVisible(false);
		locationSettingsWidget->layout()->removeWidget(currentSettingsWidget);
		delete currentSettingsWidget;
		currentSettingsWidget = nullptr;
		delete settingsUI;
		settingsUI = nullptr;
		for (auto ch : decChannelsUi) {
			delete ch;
		}
		decChannelsUi.erase(decChannelsUi.begin(),decChannelsUi.end());
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

void LogicAnalyzerChannelManagerUI::resizeEvent(QResizeEvent *event)
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
