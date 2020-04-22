#include "logic_analyzer_api.hpp"

#include "osc_export_settings.h"
#include "spinbox_a.hpp"

#include "ui_digital_trigger_settings.h"
#include "ui_la_channel_group.h"
#include "ui_logic_analyzer.h"

#include <pulseview/pv/data/logic.hpp>
#include <pulseview/pv/data/logicsegment.hpp>

namespace adiscope {
/*
 * class LogicAnalyzer_API
 */

void LogicAnalyzer_API::save(QSettings& settings)
{
	lga->get_channel_groups_api();
	ApiObject::save(settings);
}

void LogicAnalyzer_API::show() { Q_EMIT lga->showTool(); }

bool LogicAnalyzer_API::running() const { return lga->ui->btnRunStop->isChecked(); }

int LogicAnalyzer_API::channel_groups_list_size() const { return lga->channel_groups_api.size(); }
void LogicAnalyzer_API::setChannelGroupsListSize(int size)
{
	qDeleteAll(lga->channel_groups_api);
	lga->channel_groups_api.clear();

	lga->chm.clearChannelGroups();
	for (int i = 0; i < size; i++) {
		lga->channel_groups_api.append(new ChannelGroup_API(lga, i));
		lga->chm.add_channel_group(new LogicAnalyzerChannelGroup());
	}
}

QVariantList LogicAnalyzer_API::getChannelGroups()
{
	QVariantList list;
	for (ChannelGroup_API* each : lga->channel_groups_api)
		list.append(QVariant::fromValue(each));
	return list;
}

void LogicAnalyzer_API::run(bool en)
{
	if (lga->ui->btnRunStop->isEnabled()) {
		lga->ui->btnRunStop->setChecked(en);
	}
}

bool LogicAnalyzer_API::single() const { return lga->ui->btnSingleRun->isChecked(); }

void LogicAnalyzer_API::runSingle(bool en)
{
	if (lga->ui->btnSingleRun->isEnabled()) {
		lga->ui->btnSingleRun->setChecked(en);
	}
}

double LogicAnalyzer_API::getTimePos() const { return lga->timePosition->value(); }

void LogicAnalyzer_API::setTimePos(double value)
{
	lga->timePosition->setValue(value);
	lga->initialised = true;
}

double LogicAnalyzer_API::getTimeBase() const { return lga->timeBase->value(); }

void LogicAnalyzer_API::setTimeBase(double value) { lga->timeBase->setValue(value); }

bool LogicAnalyzer_API::externalTrigger() const { return lga->trigger_settings_ui->trigg_extern_en->isChecked(); }

void LogicAnalyzer_API::setExternalTrigger(bool en) { lga->trigger_settings_ui->trigg_extern_en->setChecked(en); }

int LogicAnalyzer_API::externalTriggerSource() const
{
	return lga->trigger_settings_ui->cmb_extern_src->currentIndex();
}

void LogicAnalyzer_API::setExternalTriggerSource(int en)
{
	if (en >= 0 && en < lga->trigger_settings_ui->cmb_extern_src->count()) {
		lga->trigger_settings_ui->cmb_extern_src->setCurrentIndex(en);
	} else {
		lga->trigger_settings_ui->cmb_extern_src->setCurrentIndex(0);
	}
}

int LogicAnalyzer_API::externalTriggerCnd() const
{
	return lga->trigger_settings_ui->cmb_trigg_extern_cond_1->currentIndex();
}

void LogicAnalyzer_API::setExternalTriggerCnd(int en)
{
	if (en >= 0 && en < lga->trigger_settings_ui->cmb_trigg_extern_cond_1->count()) {
		lga->trigger_settings_ui->cmb_trigg_extern_cond_1->setCurrentIndex(en);
	} else {
		lga->trigger_settings_ui->cmb_trigg_extern_cond_1->setCurrentIndex(0);
	}
}

bool LogicAnalyzer_API::cursorsActive() const { return lga->ui->boxCursors->isChecked(); }

void LogicAnalyzer_API::setCursorsActive(bool en) { lga->ui->boxCursors->setChecked(en); }

bool LogicAnalyzer_API::cursorsLocked() const { return lga->ui->btnCursorsLock->isChecked(); }

void LogicAnalyzer_API::setCursorsLocked(bool en)
{
	lga->ui->btnCursorsLock->setChecked(en);
	lga->ui->btnCursorsLock->toggled(en);
}

bool LogicAnalyzer_API::inactiveHidden() const { return lga->ui->btnShowChannels->isChecked(); }

void LogicAnalyzer_API::setInactiveHidden(bool en) { lga->ui->btnShowChannels->clicked(en); }

QString LogicAnalyzer_API::runMode() const
{
	if (lga->acquisition_mode == 0) {
		return "REPEATED";
	} else if (lga->acquisition_mode == 1) {
		return "STREAM";
	} else {
		return "SCREEN";
	}
}

void LogicAnalyzer_API::setRunMode(QString value)
{
	if (value == "STREAM") {
		lga->ui->btnStream->setChecked(true);
	} else {
		lga->ui->btnRepeated->setChecked(true);
	}
}

bool LogicAnalyzer_API::getExportAll() const { return lga->exportSettings->getExportAllButton()->isChecked(); }

void LogicAnalyzer_API::setExportAll(bool en) { lga->exportSettings->getExportAllButton()->setChecked(en); }

void LogicAnalyzer_API::load(QSettings& s)
{
	lga->apiLoading = true;
	ApiObject::load(s);
	lga->apiLoading = false;
}

/*
 * ChannelGroup_API
 */
bool ChannelGroup_API::chEnabled() const
{
	if (lga->chm.get_channel_group(getIndex()))
		return lga->chm.get_channel_group(getIndex())->is_enabled();
	return false;
}

void ChannelGroup_API::setChEnabled(bool en)
{
	auto chGroupUI = lga->chm_ui->getUiFromChGroup(lga->chm.get_channel_group(getIndex()));
	if (chGroupUI && !(lga->apiLoading)) {
		chGroupUI->ui->btnEnableChannel->setChecked(en);
	} else
		lga->chm.get_channel_group(getIndex())->enable(en);
}

bool ChannelGroup_API::chGrouped() const { return lga->chm.get_channel_group(getIndex())->is_grouped(); }

void ChannelGroup_API::setChGrouped(bool en) { lga->chm.get_channel_group(getIndex())->group(en); }

QString ChannelGroup_API::getName() const
{
	auto ch_group = lga->chm.get_channel_group(getIndex());
	if (ch_group) {
		return QString::fromStdString(ch_group->get_label());
	}
	return "";
}

void ChannelGroup_API::setName(QString val) { lga->chm.get_channel_group(getIndex())->set_label(val.toStdString()); }

bool ChannelGroup_API::getChCollapsed() const { return lga->chm.get_channel_group(this->getIndex())->isCollapsed(); }

void ChannelGroup_API::setChCollapsed(bool en) { lga->chm.get_channel_group(this->getIndex())->collapse(en); }

int ChannelGroup_API::getIndex() const
{
	if (index == -1) {
		return lga->channel_groups_api.indexOf(const_cast<ChannelGroup_API*>(this));
	}
	return index;
}

int ChannelGroup_API::channels_list_size() const { return channels_api.size(); }
void ChannelGroup_API::setChannelsListSize(int size)
{
	qDeleteAll(channels_api);
	channels_api.clear();
	for (int i = 0; i < size; i++) {
		channels_api.append(new LogicChannel_API(lga, this));
	}
}

QVariantList ChannelGroup_API::getChannels()
{
	QVariantList list;

	for (LogicChannel_API* each : channels_api)
		list.append(QVariant::fromValue(each));

	return list;
}

void ChannelGroup_API::set_channels_api()
{
	qDeleteAll(channels_api);
	channels_api.clear();
	auto ch_group = lga->chm.get_channel_group(index);
	if (ch_group) {
		int size = ch_group->get_channel_count();
		int ch_index;
		for (int i = 0; i < size; i++) {
			if (ch_group->get_channel(i)) {
				ch_index = ch_group->get_channel(i)->get_id();
				channels_api.append(new LogicChannel_API(lga, this, ch_index));
			}
		}
	}
}

QString ChannelGroup_API::getDecoder() const
{
	if (lga->chm.get_channel_group(this->getIndex())->getDecoder())
		return QString::fromUtf8(lga->chm.get_channel_group(this->getIndex())->getDecoder()->name);
	return "";
}

void ChannelGroup_API::setDecoder(QString val)
{
	lga->chm.get_channel_group(getIndex())->setDecoder(lga->chm.get_decoder_from_name(val.toUtf8()));
}

QString ChannelGroup_API::getDecoderSettings() const
{
	if (lga->chm.get_channel_group(this->getIndex())->getDecoder()) {
		lga->chm.get_channel_group(this->getIndex())->saveDecoderSettings();
		return lga->chm.get_channel_group(this->getIndex())->getDecoderSettings();
	}
	return "";
}
void ChannelGroup_API::setDecoderSettings(QString val)
{
	lga->chm.get_channel_group(getIndex())->setDecoderSettings(val);
}

/*
 * Channel_API
 */

QString LogicChannel_API::getTrigger() const
{
	return QString::fromStdString(lga->chm.get_channel(getIndex())->getTrigger());
}

void LogicChannel_API::setTrigger(QString val)
{
	lga->chm.get_channel(getIndex())->setTrigger(val.toStdString());
	lga->setTriggerCache(getIndex(), val.toStdString());
}

QString LogicChannel_API::getName() const
{
	return QString::fromStdString(lga->chm.get_channel(getIndex())->get_label());
}

void LogicChannel_API::setName(QString val) { lga->chm.get_channel(getIndex())->set_label(val.toStdString()); }

int LogicChannel_API::getIndex() const { return index; }

void LogicChannel_API::setIndex(int val)
{
	index = val;
	lga->chm.get_channel_group(lchg->getIndex())->add_logic_channel(lga->chm.get_channel(val));
}

QString LogicChannel_API::getRole() const
{
	if (lga->chm.get_channel_group(lchg->getIndex())->is_grouped()) {
		auto ch =
			lga->chm.get_channel_group(lchg->getIndex())->get_channel_by_id(getIndex())->getChannel_role();
		if (ch) {
			return ch->name;
		}
	}
	return "";
}

void LogicChannel_API::setRole(QString val)
{
	auto ch = lga->chm.get_channel_group(lchg->getIndex())->get_srd_channel_from_name(val.toUtf8());
	lga->chm.get_channel_group(lchg->getIndex())->get_channel_by_id(getIndex())->setChannel_role(ch);
}

QList<int> LogicAnalyzer_API::data() const
{
	QList<int> list;

	std::shared_ptr<pv::data::Logic> logic_data = lga->main_win->session_.get_logic_data();
	if (!logic_data) {
		return list;
	} else {
		std::shared_ptr<pv::data::LogicSegment> segment = logic_data->logic_segments().front();
		if (!segment)
			return list;
		for (unsigned int i = 0; i < segment->get_sample_count(); ++i) {
			uint64_t sample = segment->get_sample(i);
			list.append((int)sample);
		}
	}
	return list;
}
} // namespace adiscope
