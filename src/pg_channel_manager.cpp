#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "pg_channel_manager.hpp"
#include "pattern_generator.hpp"
#include "dynamicWidget.hpp"
#include "boost/math/common_factor.hpp"

#include "ui_pg_channel_group.h"
#include "ui_pg_channel_manager.h"
#include "ui_pg_channel_header.h"
#include "ui_pg_channel.h"


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
	qDebug()<<"currentChannelUI->ui created";
	ui = new Ui::PGChannelGroup();
}

void PatternGeneratorChannelUI::setTrace(
        std::shared_ptr<pv::view::TraceTreeItem> item)
{
	trace = item;
}

void PatternGeneratorChannelUI::enableControls(bool val)
{
	ui->DioLabel->setEnabled(val);
	ui->ChannelGroupLabel->setEnabled(val);
}


void PatternGeneratorChannelUI::mousePressEvent(QMouseEvent *)
{
	getManagerUi()->showHighlight(false);
	getManagerUi()->chm->highlightChannel(this->chgui->getChannelGroup(),
	                                      this->getChannel());
	getManagerUi()->showHighlight(true);
}


PatternGeneratorChannelUI::~PatternGeneratorChannelUI()
{
	delete ui;
	//qDebug()<<"currentChannelUI->ui destroyed";
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
	//qDebug()<<"pgchannelgroupdestroyed";
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
	getManagerUi()->updateUi();
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
        std::shared_ptr<pv::view::TraceTreeItem> item)
{
	trace = item;
}


void PatternGeneratorChannelGroupUI::mousePressEvent(QMouseEvent *)
{
	getManagerUi()->showHighlight(false);
	getManagerUi()->chm->highlightChannel(this->getChannelGroup());
	getManagerUi()->showHighlight(true);
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

	qDebug()<<"Final sample rate "<<computeSuggestedSampleRate();
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

    for(i=0;i<chg->get_channel_count();i++)
    {
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
	//uint32_t sampleRate = 1;
	uint32_t sampleDivider=0;

	for (auto &&chg : channel_group) {
		if (chg->is_enabled()) {
			auto patternSamplingFrequency = static_cast<PatternGeneratorChannelGroup *>
			                                (chg)->pattern->get_min_sampling_freq();
			uint32_t val = 80000000 / patternSamplingFrequency;

			//sampleRate = boost::math::lcm(patternSamplingFrequency, sampleRate);
			sampleDivider = boost::math::gcd(sampleDivider,val);
//            qDebug()<<static_cast<PatternGeneratorChannelGroup*>(chg)->pattern->get_min_sampling_freq();
			qDebug()<<80000000.0/val;

		}
	}

	qDebug()<<"final samplerate: "<<80000000.0/sampleDivider;

	if (!sampleDivider) {
		sampleDivider = 1;
	}

	return 80000000/sampleDivider;
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

void PatternGeneratorChannelManagerUI::updateUi()
{
	static const int channelGroupLabelMaxLength = 10;
	static const int dioLabelMaxLength = 2;
	static const int channelComboMaxLength = 15;
	static const int outputComboMaxLength = 5;


	for (auto ch : chg_ui) {
		delete ch;
	}

	chg_ui.erase(chg_ui.begin(),chg_ui.end());

    auto offset = 0;
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


	for (auto&& ch : *(chm->get_channel_groups())) {
		if (disabledShown==false && !ch->is_enabled()) {
			continue;
		}

		chg_ui.push_back(new PatternGeneratorChannelGroupUI(
		                         static_cast<PatternGeneratorChannelGroup *>(ch),this,
		                         0)); // create widget for channelgroup
		PatternGeneratorChannelGroupUI *currentChannelGroupUI = chg_ui.back();

		currentChannelGroupUI->ui->setupUi(chg_ui.back());

        ui->verticalLayout->insertWidget(chg_ui.size()-1,chg_ui.back());
		currentChannelGroupUI->ensurePolished();

		retainWidgetSizeWhenHidden(currentChannelGroupUI->ui->collapseBtn);
		retainWidgetSizeWhenHidden(currentChannelGroupUI->ui->line);
		retainWidgetSizeWhenHidden(currentChannelGroupUI->ui->line_2);

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

		connect(currentChannelGroupUI->ui->enableBox,SIGNAL(toggled(bool)),
		        chg_ui.back(),SLOT(enable(bool)));
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
		//connect(currentChannelGroupUI->getChannelGroup()->pattern, SIGNAL(generate_pattern),pg,SLOT())


        offset+=(currentChannelGroupUI->geometry().bottomRight().y());

		if (ch->is_grouped()) { // create subwidgets
           /* auto trace = main_win->view_->add_decoder();
            currentChannelGroupUI->setTrace(trace);
            trace->force_to_v_offset(offset);*/

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
				//QWidget *p = new QWidget(chg_ui.back());

				currentChannelUI->ui->setupUi(currentChannelUI);
				currentChannelGroupUI->ui->subChannelLayout->insertWidget(i,currentChannelUI);
				currentChannelGroupUI->ensurePolished();

				retainWidgetSizeWhenHidden(currentChannelUI->ui->collapseBtn);
				retainWidgetSizeWhenHidden(currentChannelUI->ui->line);
				retainWidgetSizeWhenHidden(currentChannelUI->ui->line_2);

				if (!detailsShown) {
					currentChannelUI->ui->wgChannelSelectionGroup->setVisible(false);
				} else {
					retainWidgetSizeWhenHidden(currentChannelUI->ui->wgChannelSettingsGroup);
					retainWidgetSizeWhenHidden(currentChannelUI->ui->wgChannelEnableGroup);
					retainWidgetSizeWhenHidden(currentChannelUI->ui->outputCombo);
					retainWidgetSizeWhenHidden(currentChannelUI->ui->selectBox);
				}

                currentChannelUI->ui->line->setVisible(false);
                currentChannelUI->ui->line_2->setVisible(false);

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

				auto str = QString().fromStdString(ch->get_channel(i)->get_label());
				currentChannelUI->ui->ChannelGroupLabel->setText(str);

				auto index = ch->get_channel(i)->get_id();
				auto trace = main_win->view_->get_clone_of(index);
				currentChannelUI->setTrace(trace);
				currentChannelUI->trace->force_to_v_offset(offset);


				str = QString().number(index);
				currentChannelUI->ui->DioLabel->setText(str);
                offset+=(currentChannelUI->geometry().bottomRight().y());

			}

			if (static_cast<PatternGeneratorChannelGroup *>(ch)->isCollapsed()) {
				currentChannelGroupUI->ui->collapseBtn->setChecked(true);
				currentChannelGroupUI->ui->subChannelWidget->setVisible(false);
			}
		} else {
			auto index = ch->get_channel()->get_id();
			auto trace = main_win->view_->get_clone_of(index);
			currentChannelGroupUI->setTrace(trace);
			qDebug()<<currentChannelGroupUI->pos();
			currentChannelGroupUI->trace->force_to_v_offset(offset);


			currentChannelGroupUI->ui->DioLabel->setText(QString().number(
			                        ch->get_channel()->get_id()));
			currentChannelGroupUI->ui->splitBtn->setVisible(false);
			currentChannelGroupUI->ui->collapseBtn->setVisible(false);
			currentChannelGroupUI->ui->line->setVisible(false);
			currentChannelGroupUI->ui->line_2->setVisible(false);

		}

		// only enable channelgroup controls after all channelgroup subchannels have been created
		currentChannelGroupUI->ui->enableBox->setChecked(ch->is_enabled());
		currentChannelGroupUI->enableControls(ch->is_enabled());
	}

 //   ui->scrollAreaWidgetContents->updateGeometry();

	if (highlightShown) {
		showHighlight(true);
	}

	Q_EMIT channelsChanged();

    //pg->setPlotStatusHeight(channelManagerHeaderWiget->sizeHint().height());
    ui->scrollArea->setMaximumWidth(channelManagerHeaderWiget->sizeHint().width());

    main_win->view_->viewport()->setDivisionHeight(44);
    main_win->view_->viewport()->setDivisionCount(6);
    main_win->view_->viewport()->setDivisionOffset(5);

    QScrollBar* chmVertScrollArea = ui->scrollArea->verticalScrollBar();
    connect(chmVertScrollArea, SIGNAL(valueChanged(int)),this,SLOT(chmScrollChanged(int)));
}

void PatternGeneratorChannelManagerUI::chmScrollChanged(int val)
{
    // check for user interaction ... tracking()
    main_win->view_->set_v_offset(val);
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
	connect(currentUI,SIGNAL(patternChanged()),this,SIGNAL(channelsChanged()));
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

}



