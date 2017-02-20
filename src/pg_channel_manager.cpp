#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "pulseview/pv/view/logicsignal.hpp"
#include "pulseview/pv/view/decodetrace.hpp"
#include "pg_channel_manager.hpp"
#include "pattern_generator.hpp"
#include "dynamicWidget.hpp"
#include "boost/math/common_factor.hpp"
#include "libsigrokdecode/libsigrokdecode.h"

#include "ui_pg_channel_group.h"
#include "ui_pg_channel_manager.h"
#include "ui_pg_channel_header.h"
#include "ui_pg_channel.h"

using std::dynamic_pointer_cast;

namespace Ui {
class PGChannelGroup;
class PGChannel;
class PGChannelManager;
class PGChannelManagerHeader;
}

namespace pv {
class MainWindow;
namespace view {
class View;
class Viewport;
class TraceTreeItem;
class LogicSignal;
class DecodeTrace;
}
}

namespace adiscope {

//////////////////////// CHANNEL
PatternGeneratorChannel::PatternGeneratorChannel(uint16_t id_,
                std::string label_) : Channel(id_,label_), channel_role("None"),
	trigger("rising")
{

}
std::string PatternGeneratorChannel::getTrigger() const
{
	return trigger;
}

void PatternGeneratorChannel::setTrigger(const std::string& value)
{
	trigger = value;
}

std::string PatternGeneratorChannel::getChannel_role() const
{
	return channel_role;
}

void PatternGeneratorChannel::setChannel_role(const std::string& value)
{
	channel_role = value;
}
///////////////////////// CHANNEL UI
PatternGeneratorChannelUI::PatternGeneratorChannelUI(PatternGeneratorChannel
                *ch, PatternGeneratorChannelGroup *chg, PatternGeneratorChannelGroupUI *chgui,
                PatternGeneratorChannelManagerUI *managerUi, QWidget *parent) : ChannelUI(ch,
	                                parent), managerUi(managerUi), chg(chg),chgui(chgui)
{
	this->ch = ch;
	ui = new Ui::PGChannelGroup();
	setAcceptDrops(true);
}

void PatternGeneratorChannelUI::setTrace(
	std::shared_ptr<pv::view::LogicSignal> item)
{
	trace = item;
}

std::shared_ptr<pv::view::LogicSignal> PatternGeneratorChannelUI::getTrace()
{
	return trace;
}


void PatternGeneratorChannelUI::enableControls(bool val)
{
	ui->DioLabel->setEnabled(val);
	ui->ChannelGroupLabel->setEnabled(val);
}



void PatternGeneratorChannelUI::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		dragStartPosition = event->pos();
	}

	getManagerUi()->showHighlight(false);
	getManagerUi()->chm->highlightChannel(this->chgui->getChannelGroup(),
	                                      this->getChannel());
	getManagerUi()->showHighlight(true);
}

void PatternGeneratorChannelUI::highlight(bool val)
{
	setDynamicProperty(ui->widget_2,"highlighted",val);

	if (trace) {
		trace->set_highlight(val);
	}
}

void PatternGeneratorChannelUI::highlightTopSeparator()
{
	resetSeparatorHighlight();
	topSep->setVisible(true);
}

void PatternGeneratorChannelUI::highlightBotSeparator()
{
	resetSeparatorHighlight();
	botSep->setVisible(true);
}

void PatternGeneratorChannelUI::resetSeparatorHighlight(bool force)
{
	topSep->setVisible(false);
	botSep->setVisible(false);
}

void PatternGeneratorChannelUI::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	} else {
		getManagerUi()->setHoverWidget(this);
	}

	if ((event->pos() - dragStartPosition).manhattanLength()
	    < QApplication::startDragDistance()) {
		return;
	}

	getManagerUi()->clearHoverWidget();

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	auto channelGroups = getManagerUi()->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
	auto chgIndex = chgIter-channelGroups->begin();

	auto channels = (*chgIter)->get_channels();
	auto chIter = std::find(channels->begin(),channels->end(),ch);
	auto chIndex = chIter-(channels->begin());

	dataStream << (short)chgIndex<<(short)chIndex;

	mimeData->setData("patterngenerator/channel", itemData);

	if (getManagerUi()->pixmapEnable) {
		QPixmap pix;
		pix = this->grab().scaled(this->geometry().width()/getManagerUi()->pixmapScale,
		                          this->geometry().height()/getManagerUi()->pixmapScale);
		this->setVisible(!getManagerUi()->pixmapGrab);

		if (getManagerUi()->pixmapRetainSize) {
			getManagerUi()->retainWidgetSizeWhenHidden(this);
		}

		drag->setPixmap(pix);
	}

	drag->setMimeData(mimeData);
	Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
	this->setVisible(true);

}

void PatternGeneratorChannelUI::dragEnterEvent(QDragEnterEvent *event)
{
	auto w = ui->widget_2->geometry().width();
	auto h = ui->widget_2->geometry().height();
	topDragbox.setRect(0, 0, w, h/2);
	botDragbox.setRect(0,h/2,w, h/2);

	if (event->mimeData()->hasFormat("patterngenerator/channelgroup")) {
		short from = (short)event->mimeData()->data("patterngenerator/channelgroup")[1];
		auto channelGroups = getManagerUi()->chm->get_channel_groups();
		auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
		auto chgIndex = chgIter-channelGroups->begin();

		if (from == chgIndex) {
			event->ignore();
			return;
		}
	}

	event->accept();
}


void PatternGeneratorChannelUI::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->answerRect().intersects(botDragbox)) {
		getManagerUi()->clearHoverWidget();
		chgui->resetSeparatorHighlight(true);
		highlightBotSeparator();
		event->accept();
	} else if (event->answerRect().intersects(topDragbox)) {
		getManagerUi()->clearHoverWidget();
		chgui->resetSeparatorHighlight(true);
		highlightTopSeparator();
		event->accept();
	} else if (event->answerRect().intersects(centerDragbox)) {
		getManagerUi()->setHoverWidget(this);
		chgui->resetSeparatorHighlight();
		event->accept();
	} else {
		chgui->resetSeparatorHighlight();
		event->ignore();
	}
}

void PatternGeneratorChannelUI::dragLeaveEvent(QDragLeaveEvent *event)
{
	resetSeparatorHighlight();
	event->accept();
}

void PatternGeneratorChannelUI::enterEvent(QEvent *event)
{
	getManagerUi()->setHoverWidget(this);
	QWidget::enterEvent(event);
}

void PatternGeneratorChannelUI::leaveEvent(QEvent *event)
{
	getManagerUi()->clearHoverWidget();
	QWidget::leaveEvent(event);
}

void PatternGeneratorChannelUI::dropEvent(QDropEvent *event)
{

	if (event->source() == this && event->possibleActions() & Qt::MoveAction) {
		return;
	}

	auto channelGroups = getManagerUi()->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
	auto chgIndex = chgIter-channelGroups->begin();

	auto toNrOfChannels = (*chgIter)->get_channel_count();

	auto channels = (*chgIter)->get_channels();
	auto chIter = std::find(channels->begin(),channels->end(),ch);
	auto chIndex = chIter-(channels->begin());

	bool dropAfter = botDragbox.contains(event->pos());

	if (event->mimeData()->hasFormat("patterngenerator/channelgroup")) {
		short from = (short)event->mimeData()->data("patterngenerator/channelgroup")[1];
		auto fromNrOfChannels = getManagerUi()->chm->get_channel_group(
		                                from)->get_channel_count();
		getManagerUi()->chm->join({(int)chgIndex,from});
		getManagerUi()->chm->highlightChannel(getManagerUi()->chm->get_channel_group(
		                chgIndex));
		chgIndex = chgIndex + (chgIndex > from ? -1 : 0);
		auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
		auto j=0;

		for (auto i=fromNrOfChannels; i>0; i--) {
			getManagerUi()->chm->moveChannel(chgIndex, (*chgIter)->get_channel_count()-i,
			                                 chIndex+j,dropAfter);
			j++;
		}
	}

	if (event->mimeData()->hasFormat("patterngenerator/channel")) {
		short fromChg = (short)event->mimeData()->data("patterngenerator/channel")[1];
		short fromCh  = (short)event->mimeData()->data("patterngenerator/channel")[3];

		auto fromNrOfChannels = getManagerUi()->chm->get_channel_group(
		                                fromChg)->get_channel_count();

		getManagerUi()->chm->splitChannel(fromChg, fromCh);

		if (fromNrOfChannels != 1) {
			fromChg++;
			chgIndex = chgIndex + (chgIndex > fromChg ? 1 : 0);
		}

		getManagerUi()->chm->join({(int)chgIndex,fromChg});

		chgIndex = chgIndex + (chgIndex > fromChg ? -1 : 0);
		auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
		//getManagerUi()->chm->move(fromChg, chgIndex + ((fromChg)>chgIndex ? 0 : 1),dropAfter);
		getManagerUi()->chm->moveChannel(chgIndex, (*chgIter)->get_channel_count()-1,
		                                 chIndex,dropAfter);
	}

	Q_EMIT requestUpdateUi();
}

void PatternGeneratorChannelUI::paintEvent(QPaintEvent *event)
{
	if(trace==nullptr)
	{
		auto index = ch->get_id();
		auto height = geometry().height();
		auto trace1 = getManagerUi()->main_win->view_->get_clone_of(index,height);
		setTrace(trace1);

	}

	auto height = geometry().height();
	auto v_offset = botSep->geometry().bottomRight().y() - (trace->v_extents().second) + chgui->traceOffset+3;//chgOffset.y();

	if(traceOffset!=v_offset || traceHeight!=height)
	{
		traceHeight = height;
		traceOffset = v_offset;
		trace->setSignal_height(traceHeight);
		trace->force_to_v_offset(v_offset);
	}

}


PatternGeneratorChannelUI::~PatternGeneratorChannelUI()
{
	delete ui;
}

PatternGeneratorChannelManagerUI *PatternGeneratorChannelUI::getManagerUi()
const
{
	return managerUi;
}

PatternGeneratorChannel *PatternGeneratorChannelUI::getChannel()
{
	return static_cast<PatternGeneratorChannel *>(ch);
}

PatternGeneratorChannelGroup *PatternGeneratorChannelUI::getChannelGroup()
{
	return static_cast<PatternGeneratorChannelGroup *>(chg);
}

/////////////////////// CHANNEL GROUP
PatternGeneratorChannelGroup::PatternGeneratorChannelGroup(
        PatternGeneratorChannel *ch, bool en) : ChannelGroup(ch)
{
	created_index = 0;
	collapsed = false;
	enabled = false;
	pattern=PatternFactory::create(0);
}

PatternGeneratorChannelGroup::~PatternGeneratorChannelGroup()
{
	if (pattern) {
		delete pattern;
		pattern = nullptr;
	}
}

bool PatternGeneratorChannelGroup::isCollapsed()
{
	return collapsed;
}

void PatternGeneratorChannelGroup::collapse(bool val)
{
	collapsed = val;
}

void PatternGeneratorChannelGroup::append(PatternGeneratorChannelGroup *tojoin)
{
	for (auto i=0; i<tojoin->channels.size(); i++) {
		channels.push_back(tojoin->channels[i]);
	}
}


////////////////////////// CHANNEL GROUP UI
PatternGeneratorChannelManagerUI *PatternGeneratorChannelGroupUI::getManagerUi()
const
{
	return managerUi;
}

PatternGeneratorChannelGroupUI::PatternGeneratorChannelGroupUI(
        PatternGeneratorChannelGroup *chg, PatternGeneratorChannelManagerUI *managerUi,
        QWidget *parent) : ChannelGroupUI(chg,parent), managerUi(managerUi)
{
	this->chg = chg;
	checked = false;
	ui = new Ui::PGChannelGroup();
	setAcceptDrops(true);
	trace = nullptr;
	decodeTrace = nullptr;
	//setDragEnabled(true);
	//setDropIndicatorShown(true);
}

PatternGeneratorChannelGroupUI::~PatternGeneratorChannelGroupUI()
{
	delete ui;
}

PatternGeneratorChannelGroup *PatternGeneratorChannelGroupUI::getChannelGroup()
{
	return static_cast<PatternGeneratorChannelGroup *>(this->chg);
}

void PatternGeneratorChannelGroupUI::patternChanged(int index)
{
	getChannelGroup()->pattern->deinit();
	delete getChannelGroup()->pattern;
	getChannelGroup()->created_index=index;
	getChannelGroup()->pattern = PatternFactory::create(index);

	if (getChannelGroup()==getManagerUi()->chm->getHighlightedChannelGroup()) {
		getManagerUi()->deleteSettingsWidget();
		getManagerUi()->createSettingsWidget();
	}
}

void PatternGeneratorChannelGroupUI::select(bool selected)
{
	ChannelGroupUI::select(selected);
	Q_EMIT channel_selected();
}

void PatternGeneratorChannelGroupUI::collapse()
{
	bool val = !getChannelGroup()->isCollapsed();
	getChannelGroup()->collapse(val);
	ui->subChannelWidget->setVisible(!val);
}

void PatternGeneratorChannelUI::split()
{
	auto channelGroups = getManagerUi()->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);

	if (chgIter == channelGroups->end()) {
		return;
	}

	auto chgIndex = chgIter-channelGroups->begin();
	auto channels = *(getManagerUi()->chm->get_channel_group(
	                          chgIndex)->get_channels());
	auto chIter = std::find(channels.begin(),channels.end(),ch);

	if (chIter==channels.end()) {
		return;
	}

	auto chIndex = chIter-channels.begin();
	bool changeHighlight = false;

	if (*chIter == getManagerUi()->chm->getHighlightedChannel() ||
	    (getManagerUi()->chm->get_channel_group(chgIndex) ==
	     getManagerUi()->chm->getHighlightedChannelGroup() &&
	     getManagerUi()->chm->getHighlightedChannel() == nullptr)) {
		changeHighlight = true;
	}

	getManagerUi()->chm->splitChannel(chgIndex,chIndex);

	if (changeHighlight) {
		getManagerUi()->chm->highlightChannel(getManagerUi()->chm->get_channel_group(
		                chgIndex));
	}

	getManagerUi()->updateUi();

}

void PatternGeneratorChannelGroupUI::split()
{
	getManagerUi()->chm->deselect_all();
	select(true);
	getManagerUi()->groupSplitSelected();
	Q_EMIT requestUpdateUi();
}

void PatternGeneratorChannelGroupUI::enableControls(bool enabled)
{
	ui->ChannelGroupLabel->setEnabled(enabled);
	ui->DioLabel->setEnabled(enabled);

	for (auto &&ch : ch_ui) {
		ch->enableControls(enabled);
	}
}

void PatternGeneratorChannelGroupUI::enable(bool enabled)
{
	ChannelGroupUI::enable(enabled);
	enableControls(enabled);
	Q_EMIT channel_enabled();
}

int PatternGeneratorChannelGroupUI::isChecked()
{
	return checked;
}

void PatternGeneratorChannelGroupUI::check(int val)
{
	checked = val;
}

void PatternGeneratorChannelGroupUI::setTrace(
	std::shared_ptr<pv::view::LogicSignal> item){
	trace = dynamic_pointer_cast<pv::view::TraceTreeItem>(item);
	logicTrace = item;
}

void PatternGeneratorChannelGroupUI::setTrace(
	std::shared_ptr<pv::view::DecodeTrace> item){
	trace = dynamic_pointer_cast<pv::view::TraceTreeItem>(item);
	decodeTrace = item;
}

std::shared_ptr<pv::view::TraceTreeItem> PatternGeneratorChannelGroupUI::getTrace()
{
	return trace;
}

void PatternGeneratorChannelGroupUI::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		dragStartPosition = event->pos();
	}

	getManagerUi()->showHighlight(false);
	getManagerUi()->chm->highlightChannel(this->getChannelGroup());
	getManagerUi()->showHighlight(true);
}

void PatternGeneratorChannelGroupUI::enterEvent(QEvent *event)
{
	getManagerUi()->setHoverWidget(this);
	QWidget::enterEvent(event);
}

void PatternGeneratorChannelGroupUI::leaveEvent(QEvent *event)
{
	getManagerUi()->clearHoverWidget();
	QWidget::leaveEvent(event);
}

void PatternGeneratorChannelGroupUI::highlight(bool val)
{
	setDynamicProperty(ui->widget_2,"highlighted",val);
	if (trace) {
		trace->set_highlight(val);
	}
}

void PatternGeneratorChannelGroupUI::highlightTopSeparator()
{
	resetSeparatorHighlight(true);
	topSep->setVisible(true);
}

void PatternGeneratorChannelGroupUI::highlightBotSeparator()
{
	resetSeparatorHighlight(true);
	botSep->setVisible(true);
}

void PatternGeneratorChannelGroupUI::resetSeparatorHighlight(bool force)
{
	if (force || (!chg->is_grouped())) {
		topSep->setVisible(false);
		botSep->setVisible(false);
	} else {
		topSep->setVisible(true);
		botSep->setVisible(true);
	}
}

void PatternGeneratorChannelGroupUI::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}

	if ((event->pos() - dragStartPosition).manhattanLength()
	    < QApplication::startDragDistance()) {
		return;
	}

	getManagerUi()->clearHoverWidget();

	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);

	auto channelGroups = getManagerUi()->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
	auto chgIndex = chgIter-channelGroups->begin();

	dataStream << (short)chgIndex;

	mimeData->setData("patterngenerator/channelgroup", itemData);
	drag->setMimeData(mimeData);

	if (getManagerUi()->pixmapEnable) {
		QPixmap pix;
		pix = this->grab().scaled(this->geometry().width()/getManagerUi()->pixmapScale,
		                          this->geometry().height()/getManagerUi()->pixmapScale);
		this->setVisible(!getManagerUi()->pixmapGrab);

		if (getManagerUi()->pixmapRetainSize) {
			getManagerUi()->retainWidgetSizeWhenHidden(this);
		}

		drag->setPixmap(pix);
	}

	Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
	this->setVisible(true);

}

void PatternGeneratorChannelGroupUI::dragEnterEvent(QDragEnterEvent *event)
{
	auto w = ui->widget_2->geometry().width();
	auto h = ui->widget_2->geometry().height(); // include lines
	topDragbox.setRect(0, 0, w, h/3);
	centerDragbox.setRect(0, h/3, w, h/3);
	botDragbox.setRect(0,2*h/3,w, h/3);
	qDebug()<<"top"<<topDragbox<<"center"<<centerDragbox<<"bot"<<botDragbox<<"geo"<<geometry();
	event->accept();
}


void PatternGeneratorChannelGroupUI::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->answerRect().intersects(botDragbox)) {
		getManagerUi()->clearHoverWidget();
		resetSeparatorHighlight(true);

		if (chg->is_grouped()) {
			ch_ui[0]->highlightTopSeparator();
		} else {
			highlightBotSeparator();
		}

		event->accept();

	} else if (event->answerRect().intersects(centerDragbox)) {
		getManagerUi()->setHoverWidget(this);
		resetSeparatorHighlight();
		event->accept();
	} else if (event->answerRect().intersects(topDragbox)) {
		getManagerUi()->clearHoverWidget();
		resetSeparatorHighlight(true);
		highlightTopSeparator();
		event->accept();
	}

	else {
		resetSeparatorHighlight();
		event->ignore();
	}
}

void PatternGeneratorChannelGroupUI::dragLeaveEvent(QDragLeaveEvent *event)
{
	resetSeparatorHighlight();
	getManagerUi()->clearHoverWidget();
	event->accept();
}

void PatternGeneratorChannelGroupUI::paintEvent(QPaintEvent *event)
{

	if(trace==nullptr && decodeTrace==nullptr)
	{
		auto index = chg->get_channel(0)->get_id();
		auto height = ui->widget_2->normalGeometry().height();
		if(!chg->is_grouped())
		{
			auto trace1 = getManagerUi()->main_win->view_->get_clone_of(index,height);
			setTrace(trace1);
		}
		else
		{
			auto trace1 = getManagerUi()->main_win->view_->add_decoder();
			setTrace(trace1);

			auto decoder = srd_decoder_get_by_id("parallel");
			decodeTrace->set_decoder(decoder);
			std::map<const srd_channel*,
					std::shared_ptr<pv::view::TraceTreeItem> > channel_map;

			QStringList decoderRolesNameList;
			std::vector<const srd_channel *> decoderRolesList;
			const srd_channel* d0;
			GSList *reqCh = g_slist_copy(decoder->opt_channels);

			auto i=0;
			reqCh = reqCh->next;
			auto ch_count = chg->get_channel_count();

			for (; i <  ch_count && reqCh; reqCh = reqCh->next) {
				const srd_channel *const rqch = (const srd_channel *)reqCh->data;

				if (rqch == nullptr) {
					break;
				}
				decoderRolesNameList << QString::fromUtf8(rqch->name);
				decoderRolesList.push_back(rqch);

				std::pair<const srd_channel*,
						std::shared_ptr<pv::view::TraceTreeItem> > chtracepair;

				auto index = chg->get_channel(i)->get_id();
				chtracepair.first = decoderRolesList.back();

				chtracepair.second = getManagerUi()->main_win->view_->get_clone_of(index);
				chtracepair.second->force_to_v_offset(-100);

				channel_map.insert(chtracepair);
				i++;

			}
			decodeTrace->set_channel_map(channel_map);
		}
	}

	auto chgOffset =  geometry().top()+ui->widget_2->geometry().bottom() + 3;
	auto height = ui->widget_2->geometry().height();
	auto v_offset = chgOffset - trace->v_extents().second;


	if(traceOffset!=v_offset || traceHeight!=height)
	{
		traceHeight = height;
		traceOffset = v_offset;
		trace->force_to_v_offset(v_offset);

		if(!chg->is_grouped())
		{
			logicTrace->setSignal_height(traceHeight);
		}
		else
		{
			decodeTrace->setDecodeHeight(traceHeight);
		}
	}
}

void PatternGeneratorChannelGroupUI::dropEvent(QDropEvent *event)
{
	resetSeparatorHighlight();
	qDebug()<<"    DROP   -- "<<event;

	if (event->source() == this && event->possibleActions() & Qt::MoveAction) {
		return;
	}

	auto channelGroups = getManagerUi()->chm->get_channel_groups();
	auto chgIter = std::find(channelGroups->begin(),channelGroups->end(),chg);
	auto chgIndex = chgIter-channelGroups->begin();

	bool dropAfter = false;
	bool formGroup = false;

	if (botDragbox.contains(event->pos())) {
		if (chg->is_grouped()) {
			formGroup = true;
		} else {
			dropAfter = true;
		}
	} else if (topDragbox.contains(event->pos())) {
		dropAfter = false;
	} else if (centerDragbox.contains(event->pos())) {
		formGroup = true;
	} else {
		Q_EMIT requestUpdateUi();
		return;
	}

	if (formGroup
	    && event->mimeData()->hasFormat("patterngenerator/channelgroup")) {
		short from = (short)event->mimeData()->data("patterngenerator/channelgroup")[1];
		getManagerUi()->chm->join({(int)chgIndex,from});
		chgIndex = chgIndex + ((from<chgIndex) ? -1 : 0);
		auto chgNOfChannels = getManagerUi()->chm->get_channel_group(
		                              chgIndex)->get_channel_count();
		getManagerUi()->chm->moveChannel(chgIndex,chgNOfChannels-1, 0, false);
		getManagerUi()->chm->highlightChannel(getManagerUi()->chm->get_channel_group(
		                chgIndex));
	} else {
		if (event->mimeData()->hasFormat("patterngenerator/channelgroup")) {
			short from = (short)event->mimeData()->data("patterngenerator/channelgroup")[1];
			getManagerUi()->chm->move(from,chgIndex,dropAfter);
		}

		if (event->mimeData()->hasFormat("patterngenerator/channel")) {
			short fromChg = (short)event->mimeData()->data("patterngenerator/channel")[1];
			short fromCh  = (short)event->mimeData()->data("patterngenerator/channel")[3];
			auto fromNrOfChannels = getManagerUi()->chm->get_channel_group(
			                                fromChg)->get_channel_count();
			getManagerUi()->chm->splitChannel(fromChg, fromCh);

			if (fromNrOfChannels != 1) {
				fromChg++;
				chgIndex = chgIndex + ((fromChg) > chgIndex ? 0 : 1);
			}

			getManagerUi()->chm->move(fromChg, chgIndex, dropAfter);
		}
	}

	Q_EMIT requestUpdateUi();

}

/////////////////////////// CHANNEL MANAGER
PatternGeneratorChannelManager::PatternGeneratorChannelManager() :
	ChannelManager()
{
	for (auto i=0; i<16; i++) {
		std::string temp = "DIO" + std::to_string(i);
		channel.push_back(new PatternGeneratorChannel(i,temp));
	}

	auto temp = static_cast<PatternGeneratorChannel *>(channel.back());

	for (auto&& ch : channel) {
		channel_group.push_back(new PatternGeneratorChannelGroup(
		                                static_cast<PatternGeneratorChannel *>(ch), false));
	}

	highlightedChannel = nullptr;
	highlightedChannelGroup = static_cast<PatternGeneratorChannelGroup *>
	                          (channel_group[0]);

}

PatternGeneratorChannelManager::~PatternGeneratorChannelManager()
{
	for (auto ch : channel_group) {
		delete ch;
	}

	for (auto ch : channel) {
		delete ch;
	}
}

PatternGeneratorChannelGroup *PatternGeneratorChannelManager::get_channel_group(
        int index)
{
	return static_cast<PatternGeneratorChannelGroup *>(channel_group[index]);
}

void PatternGeneratorChannelManager::join(std::vector<int> index)
{
	PatternGeneratorChannelGroup *ret;

	for (auto i=1; i<index.size(); i++) {
		auto it = std::next(channel_group.begin(), index[i]);
		static_cast<PatternGeneratorChannelGroup *>(channel_group[index[0]])->append(
		        static_cast<PatternGeneratorChannelGroup *>(channel_group[index[i]]));
		delete *it;
		channel_group.erase(it);

		for (auto j=0; j<index.size(); j++) {
			if (index[i] < index[j]) {
				index[j]--;
			}
		}
	}

	channel_group[index[0]]->group(true);
	channel_group[index[0]]->set_label("Group name");

}

void PatternGeneratorChannelManager::split(int index)
{
	auto it = std::next(channel_group.begin(), index);
	bool chgState = (*it)->is_enabled();
	it++;

	for (auto&& subch : *(channel_group[index]->get_channels())) {
		channel_group.insert(it,
		                     new PatternGeneratorChannelGroup(static_cast<PatternGeneratorChannel *>(subch),
		                                     chgState));
		it++;
	}

	it = std::next(channel_group.begin(), index);
	delete static_cast<PatternGeneratorChannelGroup *>(*it);
	channel_group.erase(it);
}

void PatternGeneratorChannelManager::move(int from, int to, bool after)
{
	if (from == to) {
		return;
	}

	auto fromElement = channel_group[from];
	channel_group.erase(channel_group.begin() + from);
	channel_group.insert(channel_group.begin() + to + ((after) ? 1 : 0) + ((
	                             from<to) ? -1 : 0), fromElement);
}


void PatternGeneratorChannelManager::moveChannel(int fromChgIndex, int from,
                int to, bool after)
{
	if (from == to) {
		return;
	}

	auto fromChg = channel_group[fromChgIndex];
	auto fromElement = fromChg->get_channel(from);


	/*channel_group.erase(channel_group.begin() + from);
	channel_group.insert(channel_group.begin() + to + ((after) ? 1 : 0) + ((from<to) ? -1 : 0), fromElement);*/

	fromChg->get_channels()->erase(fromChg->get_channels()->begin()+from);
	fromChg->get_channels()->insert(fromChg->get_channels()->begin()+to+((
	                                        after) ? 1 : 0) + ((from<to) ? -1 : 0),fromElement);
}


void PatternGeneratorChannelManager::splitChannel(int chgIndex, int chIndex)
{

	auto it = channel_group.begin()+chgIndex
	          +1; // Use this to insert split channel after channelgroup
	// auto it = channel_group.begin()+chgIndex; // Use this to insert split channel before channelgroup
	auto subch = channel_group[chgIndex]->get_channel(chIndex);
	auto chIt = channel_group[chgIndex]->get_channels()->begin()+chIndex;
	bool chgState = (*it)->is_enabled();
	channel_group.insert(it,
	                     new PatternGeneratorChannelGroup(static_cast<PatternGeneratorChannel *>(subch),
	                                     chgState));

	auto newChgIndex =
	        chgIndex; // Use this to insert split channel after channelgroup
	// auto newChgIndex = chgIndex; // Use this to insert split channel before channelgroup
	channel_group[newChgIndex]->get_channels()->erase(chIt);

	if (channel_group[newChgIndex]->get_channel_count()==0) {
		it = channel_group.begin()+newChgIndex;
		delete static_cast<PatternGeneratorChannelGroup *>(*it);
		channel_group.erase(it);
	}

}

void PatternGeneratorChannelManager::highlightChannel(
        PatternGeneratorChannelGroup *chg, PatternGeneratorChannel *ch)
{
	highlightedChannel = ch;
	highlightedChannelGroup = chg;
}

PatternGeneratorChannelGroup
*PatternGeneratorChannelManager::getHighlightedChannelGroup()
{
	return highlightedChannelGroup;
}

PatternGeneratorChannel *PatternGeneratorChannelManager::getHighlightedChannel()
{
	return highlightedChannel;
}


void PatternGeneratorChannelManager::preGenerate()
{
	for (auto&& chg : channel_group) {
		static_cast<PatternGeneratorChannelGroup *>(chg)->pattern->pre_generate();
	}
}

void PatternGeneratorChannelManager::generatePatterns(short *mainBuffer,
                uint32_t sampleRate, uint32_t bufferSize)
{
	for (auto&& chg : channel_group) {
		PatternGeneratorChannelGroup *pgchg =
		        static_cast<PatternGeneratorChannelGroup *>(chg);

		if (pgchg->is_enabled()) {
			pgchg->pattern->generate_pattern(sampleRate,bufferSize,
			                                 pgchg->get_channel_count());
			commitBuffer(pgchg, mainBuffer, bufferSize);
			pgchg->pattern->delete_buffer();
		}
	}
}



short PatternGeneratorChannelManager::remap_buffer(uint8_t *mapping,
                uint32_t val)
{
	short ret=0;
	int i=0;

	while (val) {
		if (val&0x01) {
			ret = ret | (1<<mapping[i]);
		}

		i++;
		val>>=1;
	}

	return ret;
}

void PatternGeneratorChannelManager::commitBuffer(PatternGeneratorChannelGroup
                *chg, short *buffer, uint32_t bufferSize)
{
	uint8_t channel_mapping[16];
	memset(channel_mapping,0x00,16*sizeof(uint8_t));
	short *bufferPtr = chg->pattern->get_buffer();
	int i=0,j=0;
	auto channel_enable_mask_temp = chg->get_mask();
	auto buffer_channel_mask = (1<<chg->get_channel_count())-1;

	for (i=0; i<chg->get_channel_count(); i++) {
		channel_mapping[i] = chg->get_channel(i)->get_id();
	}

	for (auto i=0; i< bufferSize; i++) {
		auto val = (bufferPtr[i] & buffer_channel_mask);
		buffer[i] = (buffer[i] & ~(chg->get_mask())) | remap_buffer(channel_mapping,
		                val);
	}
}


uint32_t PatternGeneratorChannelManager::computeSuggestedSampleRate()
{
	qDebug()<<"suggested sampleRates:";
	uint32_t sampleRate = 1;
	//uint32_t sampleDivider=0;

	for (auto &&chg : channel_group) {
		if (chg->is_enabled()) {
			auto patternSamplingFrequency = static_cast<PatternGeneratorChannelGroup *>
			                                (chg)->pattern->get_min_sampling_freq();
			//	uint32_t val = 80000000 / patternSamplingFrequency;

			sampleRate = boost::math::lcm(patternSamplingFrequency, sampleRate);
			//sampleDivider = boost::math::gcd(sampleDivider,val);
			qDebug()<<static_cast<PatternGeneratorChannelGroup *>
			        (chg)->pattern->get_min_sampling_freq();
//			qDebug()<<80000000.0/val;

		}
	}

	//qDebug()<<"final samplerate: "<<80000000.0/sampleDivider;
	qDebug()<<"final samplerate: "<<sampleRate;

	/*if (!sampleDivider) {
		sampleDivider = 1;
	}

	return 80000000/sampleDivider;*/
	return sampleRate;
}

uint32_t PatternGeneratorChannelManager::computeSuggestedBufferSize(
        uint32_t sample_rate)
{
	uint32_t bufferSize = 1;
	uint32_t maxNonPeriodic = 1;
	qDebug()<<"suggested buffersizes";

	for (auto &&chg : channel_group) {
		if (chg->is_enabled()) {
			auto patternBufferSize = static_cast<PatternGeneratorChannelGroup *>
			                         (chg)->pattern->get_required_nr_of_samples(sample_rate,
			                                         chg->get_channel_count());

			if (static_cast<PatternGeneratorChannelGroup *>(chg)->pattern->is_periodic()) {
				bufferSize = boost::math::lcm(patternBufferSize, bufferSize);
				qDebug()<<static_cast<PatternGeneratorChannelGroup *>
				        (chg)->pattern->get_required_nr_of_samples(sample_rate,
				                        chg->get_channel_count());

			} else {
				if (maxNonPeriodic<patternBufferSize) {
					maxNonPeriodic = patternBufferSize;        // get maximum nonperiodic buffer
				}
			}
		}
	}

	// if buffersize not big enough, create a buffer that is big enough and will fit maximum nonperiodic pattern
	// TBD if correct
	if (maxNonPeriodic > bufferSize) {
		bufferSize = boost::math::lcm(maxNonPeriodic, bufferSize);
	}

	qDebug()<<"final buffersize"<<bufferSize;
	return bufferSize;
}

////////////////////////////////////// CHANNEL MANAGER UI
QWidget *PatternGeneratorChannelManagerUI::getSettingsWidget() const
{
	return settingsWidget;
}


PatternGeneratorChannelManagerUI::PatternGeneratorChannelManagerUI(
        QWidget *parent, pv::MainWindow *main_win_, PatternGeneratorChannelManager *chm,
        QWidget *settingsWidget, PatternGenerator *pg)  : QWidget(parent),
	ui(new Ui::PGChannelManager), settingsWidget(settingsWidget),
	main_win(main_win_)
{
	ui->setupUi(this);
	this->chm = chm;
	this->pg = pg;
	currentUI = nullptr;
	disabledShown = true;
	detailsShown = true;
	highlightShown = true;
	channelManagerHeaderWiget = nullptr;
	hoverWidget = nullptr;
	chm->highlightChannel(chm->get_channel_group(0));
}
PatternGeneratorChannelManagerUI::~PatternGeneratorChannelManagerUI()
{
	delete ui;
}

void PatternGeneratorChannelManagerUI::retainWidgetSizeWhenHidden(QWidget *w)
{
	QSizePolicy sp_retain = w->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	w->setSizePolicy(sp_retain);
}

void PatternGeneratorChannelManagerUI::setWidgetNrOfChars(QWidget *w,
                int minNrOfChars, int maxNrOfChars)
{
	QFontMetrics labelm(w->font());

	auto label_min_width = labelm.width(QString(minNrOfChars,'X'));
	w->setMinimumWidth(label_min_width);

	if (maxNrOfChars!=0) {
		auto label_max_width = labelm.width(QString(maxNrOfChars,'X'));
		w->setMaximumWidth(label_max_width);
	}
}

QFrame *PatternGeneratorChannelManagerUI::addSeparator(QVBoxLayout *lay,
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

void PatternGeneratorChannelManagerUI::updateUi()
{
	static const int channelGroupLabelMaxLength = 10;
	static const int dioLabelMaxLength = 2;
	static const int channelComboMaxLength = 15;
	static const int outputComboMaxLength = 5;

	hoverWidget = nullptr;
	qDebug()<<"updateUI";

	for (auto ch : chg_ui) {
		ch->hide();
		ch->deleteLater();
	}

	chg_ui.erase(chg_ui.begin(),chg_ui.end());

	for (auto sep : separators)    {
		sep->deleteLater();
	}

	separators.erase(separators.begin(),separators.end());

	if (channelManagerHeaderWiget != nullptr) {
		delete channelManagerHeaderWiget;
		channelManagerHeaderWiget = nullptr;
	}

	main_win->view_->remove_trace_clones();

	channelManagerHeaderWiget = new QWidget(ui->chmHeaderSlot);
	Ui::PGChannelManagerHeader *chmHeader = new Ui::PGChannelManagerHeader();
	chmHeader->setupUi(channelManagerHeaderWiget);
	ui->chmHeaderSlotLayout->addWidget(channelManagerHeaderWiget);

	channelManagerHeaderWiget->ensurePolished();
	ensurePolished();

	setWidgetNrOfChars(chmHeader->labelName, channelGroupLabelMaxLength);
	setWidgetNrOfChars(chmHeader->labelDIO, dioLabelMaxLength);
	setWidgetNrOfChars(chmHeader->labelType, channelComboMaxLength);
	setWidgetNrOfChars(chmHeader->labelOutput, outputComboMaxLength);

	chmHeader->labelView->setMinimumWidth(40);
	chmHeader->labelSelect->setMinimumWidth(40);

	if (!detailsShown) {
		chmHeader->wgHeaderEnableGroup->setVisible(false);
		chmHeader->wgHeaderSettingsGroup->setVisible(false);
		chmHeader->wgHeaderSelectionGroup->setVisible(false);
	}

	ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);

	QFrame *prevSep = addSeparator(ui->verticalLayout,
	                               ui->verticalLayout->count()-1);
	for (auto&& ch : *(chm->get_channel_groups())) {
		if (disabledShown==false && !ch->is_enabled()) {
			continue;
		}

		chg_ui.push_back(new PatternGeneratorChannelGroupUI(
		                         static_cast<PatternGeneratorChannelGroup *>(ch),this,
		                         0)); // create widget for channelgroup
		PatternGeneratorChannelGroupUI *currentChannelGroupUI = chg_ui.back();

		currentChannelGroupUI->ui->setupUi(chg_ui.back());

		//ui->verticalLayout->insertWidget(chg_ui.size()-1,chg_ui.back());
		ui->verticalLayout->insertWidget(ui->verticalLayout->count()-1, chg_ui.back());
		currentChannelGroupUI->ensurePolished();
		currentChannelGroupUI->topSep = prevSep;

		retainWidgetSizeWhenHidden(currentChannelGroupUI->ui->collapseBtn);

		//   currentChannelGroupUI->ui->header->setVisible(false);
		if (!detailsShown) {
			currentChannelGroupUI->ui->wgChannelEnableGroup->setVisible(false);
			currentChannelGroupUI->ui->wgChannelSelectionGroup->setVisible(false);
			currentChannelGroupUI->ui->wgChannelSettingsGroup->setVisible(false);
		} else {
			retainWidgetSizeWhenHidden(currentChannelGroupUI->ui->splitBtn);
		}


		setWidgetNrOfChars(currentChannelGroupUI->ui->ChannelGroupLabel,
		                   channelGroupLabelMaxLength);

		QString channelGroupLabel = QString().fromStdString(ch->get_label());

		if (channelGroupLabel.length()>channelGroupLabelMaxLength) {
			channelGroupLabel.truncate(channelGroupLabelMaxLength-2);
			channelGroupLabel+="..";
		}

		currentChannelGroupUI->ui->ChannelGroupLabel->setText(channelGroupLabel);

		setWidgetNrOfChars(currentChannelGroupUI->ui->DioLabel, dioLabelMaxLength);
		setWidgetNrOfChars(currentChannelGroupUI->ui->patternCombo,
		                   channelComboMaxLength);
		setWidgetNrOfChars(currentChannelGroupUI->ui->outputCombo,
		                   outputComboMaxLength);

		int i = 0;

		for (auto var : PatternFactory::get_ui_list()) {
			currentChannelGroupUI->ui->patternCombo->addItem(var);
			currentChannelGroupUI->ui->patternCombo->setItemData(i,
			                (PatternFactory::get_description_list())[i],Qt::ToolTipRole);
			i++;
		}

		currentChannelGroupUI->ui->patternCombo->setCurrentIndex(
		        static_cast<PatternGeneratorChannelGroup *>(ch)->created_index);

//        connect(static_cast<PatternGeneratorChannelGroupUI*>(chg_ui.back()),SIGNAL(channel_selected()),pg,SLOT(onChannelSelectedChanged())); // TEMP
		connect(static_cast<PatternGeneratorChannelGroupUI *>(chg_ui.back()),
			SIGNAL(channel_enabled()),this,SIGNAL(channelsChanged())); // TEMP
		connect(currentChannelGroupUI->ui->selectBox,SIGNAL(toggled(bool)),
		        static_cast<PatternGeneratorChannelGroupUI *>(chg_ui.back()),
		        SLOT(select(bool)));
		connect(currentChannelGroupUI->ui->patternCombo,
		        SIGNAL(currentIndexChanged(int)),chg_ui.back(),SLOT(patternChanged(int)));
		connect(currentChannelGroupUI->ui->patternCombo,
			SIGNAL(currentIndexChanged(int)),this,SIGNAL(channelsChanged())); // TEMP
		connect(currentChannelGroupUI,SIGNAL(requestUpdateUi()),this,
		        SLOT(triggerUpdateUi()));
		//connect(currentChannelGroupUI->getChannelGroup()->pattern, SIGNAL(generate_pattern),pg,SLOT())

		if (ch->is_grouped()) { // create subwidgets
			currentChannelGroupUI->chUiSep = addSeparator(
			                                        currentChannelGroupUI->ui->subChannelLayout,
			                                        currentChannelGroupUI->ui->subChannelLayout->count());
			prevSep = currentChannelGroupUI->chUiSep;

			currentChannelGroupUI->ui->DioLabel->setText("");

			connect(currentChannelGroupUI->ui->collapseBtn,SIGNAL(clicked()),chg_ui.back(),
			        SLOT(collapse()));
			connect(currentChannelGroupUI->ui->splitBtn,SIGNAL(clicked()),chg_ui.back(),
			        SLOT(split()));

			for (auto i=0; i<ch->get_channel_count(); i++) {

				currentChannelGroupUI->ch_ui.push_back(new PatternGeneratorChannelUI(
				                static_cast<PatternGeneratorChannel *>(ch->get_channel(i)),
				                static_cast<PatternGeneratorChannelGroup *>(ch), currentChannelGroupUI, this,
				                0)); // create widget for channelgroup
				PatternGeneratorChannelUI *currentChannelUI =
				        currentChannelGroupUI->ch_ui.back();

				currentChannelUI->ui->setupUi(currentChannelUI);
				currentChannelGroupUI->ui->subChannelLayout->insertWidget(
				        currentChannelGroupUI->ui->subChannelLayout->count(),currentChannelUI);
				currentChannelGroupUI->ensurePolished();

				currentChannelUI->botSep = addSeparator(
				                                   currentChannelGroupUI->ui->subChannelLayout,
				                                   currentChannelGroupUI->ui->subChannelLayout->count());
				currentChannelUI->topSep = prevSep;
				prevSep =  currentChannelUI->botSep;

				retainWidgetSizeWhenHidden(currentChannelUI->ui->collapseBtn);

				if (!detailsShown) {
					currentChannelUI->ui->wgChannelSelectionGroup->setVisible(false);
				} else {
					retainWidgetSizeWhenHidden(currentChannelUI->ui->wgChannelSettingsGroup);
					retainWidgetSizeWhenHidden(currentChannelUI->ui->wgChannelEnableGroup);
					retainWidgetSizeWhenHidden(currentChannelUI->ui->outputCombo);
					retainWidgetSizeWhenHidden(currentChannelUI->ui->selectBox);
				}

				setWidgetNrOfChars(currentChannelUI->ui->ChannelGroupLabel,
				                   channelGroupLabelMaxLength);
				setWidgetNrOfChars(currentChannelUI->ui->DioLabel, dioLabelMaxLength);
				setWidgetNrOfChars(currentChannelUI->ui->patternCombo, channelComboMaxLength);
				setWidgetNrOfChars(currentChannelUI->ui->outputCombo, outputComboMaxLength);

				currentChannelUI->ui->wgChannelEnableGroup->setVisible(false);
				currentChannelUI->ui->wgChannelSettingsGroup->setVisible(false);
				currentChannelUI->ui->collapseBtn->setVisible(false);
				currentChannelUI->ui->selectBox->setVisible(false);

				connect(currentChannelUI->ui->splitBtn,SIGNAL(clicked()),currentChannelUI,
				        SLOT(split()));
				connect(currentChannelUI,SIGNAL(requestUpdateUi()),this,
				        SLOT(triggerUpdateUi()));

				auto str = QString().fromStdString(ch->get_channel(i)->get_label());
				currentChannelUI->ui->ChannelGroupLabel->setText(str);

				auto index = ch->get_channel(i)->get_id();
				str = QString().number(index);
				currentChannelUI->ui->DioLabel->setText(str);
			}

			if (static_cast<PatternGeneratorChannelGroup *>(ch)->isCollapsed()) {
				currentChannelGroupUI->ui->collapseBtn->setChecked(true);
				currentChannelGroupUI->ui->subChannelWidget->setVisible(false);
			}

			currentChannelGroupUI->botSep = prevSep;
			currentChannelGroupUI->resetSeparatorHighlight();

		} else {

			currentChannelGroupUI->ui->DioLabel->setText(QString().number(
			                        ch->get_channel()->get_id()));
			currentChannelGroupUI->ui->splitBtn->setVisible(false);
			currentChannelGroupUI->ui->collapseBtn->setVisible(false);
			currentChannelGroupUI->botSep = addSeparator(ui->verticalLayout,
			                                ui->verticalLayout->count()-1);
			prevSep = currentChannelGroupUI->botSep;
			currentChannelGroupUI->chUiSep = currentChannelGroupUI->botSep;
		}

		// only enable channelgroup controls after all channelgroup subchannels have been created
		currentChannelGroupUI->ui->enableBox->setChecked(ch->is_enabled());
		currentChannelGroupUI->enableControls(ch->is_enabled());
		connect(currentChannelGroupUI->ui->enableBox,SIGNAL(toggled(bool)),
			chg_ui.back(),SLOT(enable(bool)));
	}

//   ui->scrollAreaWidgetContents->updateGeometry();

	if (highlightShown) {
		showHighlight(true);
	}

	Q_EMIT channelsChanged();

	//pg->setPlotStatusHeight(channelManagerHeaderWiget->sizeHint().height());
	ui->scrollArea->setMaximumWidth(channelManagerHeaderWiget->sizeHint().width());

	main_win->view_->viewport()->setDivisionCount(10);

	QScrollBar *chmVertScrollArea = ui->scrollArea->verticalScrollBar();

	connect(chmVertScrollArea,SIGNAL(rangeChanged(int,int)),this,SLOT(chmRangeChanged(int,int)));
	connect(chmVertScrollArea, SIGNAL(valueChanged(int)),this,
	        SLOT(chmScrollChanged(int)));
}

void PatternGeneratorChannelManagerUI::chmScrollChanged(int val)
{
	// check for user interaction ... tracking()
	main_win->view_->set_v_offset(val);
}

void PatternGeneratorChannelManagerUI::chmRangeChanged(int min, int max)
{
	main_win->view_->verticalScrollBar()->setMinimum(min);
	main_win->view_->verticalScrollBar()->setMaximum(max);
}


void PatternGeneratorChannelManagerUI::triggerUpdateUi()
{
	updateUi();
}

void PatternGeneratorChannelManagerUI::deleteSettingsWidget()
{
	if (currentUI!=nullptr) {
		currentUI->setVisible(false);
		currentUI->destroy_ui();
		delete currentUI;
		currentUI = nullptr;
	}
}


void PatternGeneratorChannelManagerUI::createSettingsWidget()
{
	currentUI = PatternFactory::create_ui(
	                    chm->getHighlightedChannelGroup()->pattern,
	                    chm->getHighlightedChannelGroup()->created_index);
	currentUI->build_ui(settingsWidget);
	currentUI->get_pattern()->init();
	currentUI->post_load_ui();
	currentUI->setVisible(true);
	//connect(currentUI,SIGNAL(patternChanged()),this,SIGNAL(channelsChanged()));
}


PatternGeneratorChannelGroupUI
*PatternGeneratorChannelManagerUI::findUiByChannelGroup(
        PatternGeneratorChannelGroup *toFind)
{
	for (auto &&ui : chg_ui) {
		if (ui->getChannelGroup() == toFind) {
			return ui;
		}
	}

	return nullptr;
}

PatternGeneratorChannelUI *PatternGeneratorChannelManagerUI::findUiByChannel(
        PatternGeneratorChannel *toFind)
{
	for (auto &&ui : chg_ui) {
		if (ui->ch_ui.size()!=0) {
			for (auto &&chUi : ui->ch_ui) {
				if (chUi->getChannel() == toFind) {
					return chUi;
				}
			}
		}
	}

	return nullptr;
}

bool PatternGeneratorChannelManagerUI::isDisabledShown()
{
	return disabledShown;
}

void PatternGeneratorChannelManagerUI::showDisabled()
{
	disabledShown = true;
}

void PatternGeneratorChannelManagerUI::hideDisabled()
{
	disabledShown = false;
}

bool PatternGeneratorChannelManagerUI::areDetailsShown()
{
	return detailsShown;
}

void PatternGeneratorChannelManagerUI::showDetails()
{
	detailsShown = true;
}

void PatternGeneratorChannelManagerUI::hideDetails()
{
	detailsShown = false;
}


void PatternGeneratorChannelManagerUI::groupSplitSelected()
{
	std::vector<int> selection = chm->get_selected_indexes();
	bool changeHighlight = false;
	auto highlightedIter = std::find(chm->get_channel_groups()->begin(),
	                                 chm->get_channel_groups()->end(),chm->getHighlightedChannelGroup());

	if (highlightedIter != chm->get_channel_groups()->end()) {
		auto highlightedIndex = highlightedIter -chm->get_channel_groups()->begin();

		if (std::find(selection.begin(),selection.end(),
		              highlightedIndex)!= selection.end()) {
			changeHighlight = true;
		}
	}


	if (selection.size() == 0) {
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

	if (changeHighlight) {
		chm->highlightChannel(chm->get_channel_group(selection[0]));
	}

	Q_EMIT channelsChanged();
}

void PatternGeneratorChannelManagerUI::showHighlight(bool val)
{
	PatternGeneratorChannelGroupUI *uiChg = findUiByChannelGroup(
	                chm->getHighlightedChannelGroup());
	PatternGeneratorChannelUI *uiCh = findUiByChannel(chm->getHighlightedChannel());

	if (uiCh!=nullptr) {
		setDynamicProperty(uiCh->ui->widget_2,"highlighted",val);

		if (uiCh->trace) {
			uiCh->trace->set_highlight(val);
		}

		return;
	}

	if (uiChg!=nullptr) {
		setDynamicProperty(uiChg->ui->widget_2,"highlighted",val);

		if (uiChg->trace) {
			uiChg->trace->set_highlight(val);
		}
	}

	if (val) {
		deleteSettingsWidget();
		createSettingsWidget();
		highlightShown = true;
	} else {
		highlightShown = false;
	}
}

void PatternGeneratorChannelManagerUI::setHoverWidget(QWidget *hover)
{
	if (hoverWidget!=nullptr) {
		setDynamicProperty(hoverWidget,"hover-property",false);
	}

	hoverWidget = hover->findChild<QWidget *>("widget_2");

	if (hoverWidget) {
		setDynamicProperty(hoverWidget,"hover-property",true);
	}
}

void PatternGeneratorChannelManagerUI::clearHoverWidget()
{
	if (hoverWidget!=nullptr) {
		setDynamicProperty(hoverWidget,"hover-property",false);
		hoverWidget = nullptr;
	}
}

}



