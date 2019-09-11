#include "oscilloscope_api.hpp"

#include "ui_oscilloscope.h"
#include "measure_settings.h"
#include "ui_measure_settings.h"
#include "ui_cursors_settings.h"
#include "ui_trigger_settings.h"
#include "channel_widget.hpp"
#include "ui_channel_settings.h"
#include "ui_osc_general_settings.h"

namespace adiscope
{
/*
 * class Oscilloscope_API
 */

bool Oscilloscope_API::hasCursors() const
{
	return osc->ui->boxCursors->isChecked();
}

void Oscilloscope_API::setCursors(bool en)
{
	osc->ui->boxCursors->setChecked(en);
}

bool Oscilloscope_API::autosetEnabled() const
{
	return osc->autosetEnabled;
}
void Oscilloscope_API::enableAutoset(bool en)
{
	osc->autosetEnabled=en;
}

bool Oscilloscope_API::hasMeasure() const
{
	return osc->ui->boxMeasure->isChecked();
}

void Oscilloscope_API::setMeasure(bool en)
{
	osc->ui->boxMeasure->setChecked(en);
}

bool Oscilloscope_API::measureAll() const
{
	return osc->measure_settings->m_ui->button_measDisplayAll->isChecked();
}

void Oscilloscope_API::setMeasureAll(bool en)
{
	osc->measure_settings->m_ui->button_measDisplayAll->setChecked(en);
}

bool Oscilloscope_API::hasCounter() const
{
	return osc->measure_settings->m_ui->button_Counter->isChecked();
}

void Oscilloscope_API::setCounter(bool en)
{
	osc->measure_settings->m_ui->button_Counter->setChecked(en);
}

bool Oscilloscope_API::hasStatistics() const
{
	return osc->measure_settings->m_ui->button_StatisticsEn->isChecked();
}

void Oscilloscope_API::setStatistics(bool en)
{
	osc->measure_settings->m_ui->button_StatisticsEn->setChecked(en);
}

bool Oscilloscope_API::horizontalCursors() const
{
	return osc->cr_ui->hCursorsEnable->isChecked();
}

void Oscilloscope_API::setHorizontalCursors(bool en)
{
	osc->cr_ui->hCursorsEnable->setChecked(en);
}

bool Oscilloscope_API::verticalCursors() const
{
	return osc->cr_ui->vCursorsEnable->isChecked();
}

void Oscilloscope_API::setVerticalCursors(bool en)
{
	osc->cr_ui->vCursorsEnable->setChecked(en);
}

double Oscilloscope_API::cursorV1() const
{
	return osc->plot.value_v1;
}

double Oscilloscope_API::cursorV2() const
{
	return osc->plot.value_v2;
}

double Oscilloscope_API::cursorH1() const
{
	return osc->plot.value_h1;
}

double Oscilloscope_API::cursorH2() const
{
	return osc->plot.value_h2;
}

void Oscilloscope_API::setCursorV1(double val)
{
	osc->plot.d_vBar1->setPosition(val);
}

void Oscilloscope_API::setCursorV2(double val)
{
	osc->plot.d_vBar2->setPosition(val);
}

void Oscilloscope_API::setCursorH1(double val)
{
	osc->plot.d_hBar1->setPosition(val);
}

void Oscilloscope_API::setCursorH2(double val)
{
	osc->plot.d_hBar2->setPosition(val);
}

bool Oscilloscope_API::autoTrigger() const
{
	return osc->trigger_settings.ui->btnTrigger->isChecked();
}

void Oscilloscope_API::setAutoTrigger(bool en)
{
	osc->trigger_settings.ui->btnTrigger->setChecked(en);
}

bool Oscilloscope_API::internalTrigger() const
{
	return osc->trigger_settings.ui->intern_en->isChecked();
}

void Oscilloscope_API::setInternalTrigger(bool en)
{
	osc->trigger_settings.ui->intern_en->setChecked(en);
}

bool Oscilloscope_API::externalTrigger() const
{
	return osc->trigger_settings.ui->extern_en->isChecked();
}

void Oscilloscope_API::setExternalTrigger(bool en)
{
	osc->trigger_settings.ui->extern_en->setChecked(en);
}


int Oscilloscope_API::externalTriggerSource() const
{
	return osc->trigger_settings.ui->cmb_extern_src->currentIndex();
}
void Oscilloscope_API::setExternalTriggerSource(int src)
{
	if (src >= 0 && src < osc->trigger_settings.ui->cmb_extern_src->count()) {
		osc->trigger_settings.ui->cmb_extern_src->setCurrentIndex(src);
	} else {
		osc->trigger_settings.ui->cmb_extern_src->setCurrentIndex(0);
	}

}

int Oscilloscope_API::externalTriggerDaisyOrder() const
{
	return osc->trigger_settings.ui->spin_daisyChain->value();
}

void Oscilloscope_API::setExternalTriggerDaisyOrder(int src)
{
	osc->trigger_settings.ui->spin_daisyChain->setValue(src);
}

bool Oscilloscope_API::externalTriggerOut() const
{
	return osc->trigger_settings.ui->extern_to_en->isChecked();
}
void Oscilloscope_API::setExternalTriggerOut(bool en)
{
	osc->trigger_settings.ui->extern_to_en->setChecked(en);
}

int Oscilloscope_API::externalTriggerOutSource() const
{
	return osc->trigger_settings.ui->cmb_extern_to_src->currentIndex();
}
void Oscilloscope_API::setExternalTriggerOutSource(int src)
{
	if (src >= 0 && src < osc->trigger_settings.ui->cmb_extern_to_src->count()) {
		osc->trigger_settings.ui->cmb_extern_to_src->setCurrentIndex(src);
	} else {
		osc->trigger_settings.ui->cmb_extern_to_src->setCurrentIndex(0);
	}
}

int Oscilloscope_API::triggerSource() const
{
	return osc->trigger_settings.ui->cmb_source->currentIndex();
}

void Oscilloscope_API::setTriggerSource(int idx)
{
	if(idx > 0)
		osc->trigger_settings.ui->cmb_source->setCurrentIndex(idx);
}

double Oscilloscope_API::getTriggerLevel() const
{
	return osc->trigger_settings.trigger_level->value();
}

void Oscilloscope_API::setTriggerLevel(double level)
{
	osc->trigger_settings.trigger_level->setValue(level);
}

double Oscilloscope_API::getTriggerHysteresis() const
{
	return osc->trigger_settings.trigger_hysteresis->value();
}

void Oscilloscope_API::setTriggerHysteresis(double hyst)
{
	osc->trigger_settings.trigger_hysteresis->setValue(hyst);
}

int Oscilloscope_API::internalCondition() const
{
	return osc->trigger_settings.ui->cmb_condition->currentIndex();
}

void Oscilloscope_API::setInternalCondition(int cond)
{
	if (cond >= osc->trigger_settings.ui->cmb_condition->count()) {
		cond = osc->trigger_settings.ui->cmb_condition->count() - 1;
	}
	osc->trigger_settings.ui->cmb_condition->setCurrentIndex(cond);
}

int Oscilloscope_API::externalCondition() const
{
	return osc->trigger_settings.ui->cmb_extern_condition->currentIndex();
}

bool Oscilloscope_API::getTriggerInput() const
{
	return osc->getTrigger_input();
}

void Oscilloscope_API::setTriggerInput(bool en)
{
	osc->setTrigger_input(en);
}

void Oscilloscope_API::setExternalCondition(int cond)
{
	if (cond >= osc->trigger_settings.ui->cmb_extern_condition->count()) {
		cond = osc->trigger_settings.ui->cmb_extern_condition->count() - 1;
	}
	osc->trigger_settings.ui->cmb_extern_condition->setCurrentIndex(cond);
}

int Oscilloscope_API::internExtern() const
{
	return osc->trigger_settings.ui->cmb_analog_extern->currentIndex();
}

void Oscilloscope_API::setInternExtern(int option)
{
	osc->trigger_settings.ui->cmb_analog_extern->setCurrentIndex(option);
}

QList<QString> Oscilloscope_API::getMathChannels() const
{
	QList<QString> list;

	for (unsigned int i = 0; i < osc->nb_math_channels + osc->nb_ref_channels; i++) {
		QWidget *obj = osc->ui->channelsList->itemAt(
		                       osc->nb_channels + i)->widget();
		ChannelWidget *cw = static_cast<ChannelWidget *>(obj);

		if (cw->isMathChannel()) {
			list.append(obj->property("function").toString());
		}
	}

	return list;
}

void Oscilloscope_API::setMathChannels(const QList<QString>& list)
{
	osc->clearMathChannels();
	for (unsigned int i = 0; i < list.size(); i++)
		osc->add_math_channel(list.at(i).toStdString());
}

double Oscilloscope_API::getTimePos() const
{
	return osc->timePosition->value();
}

void Oscilloscope_API::setTimePos(double value)
{
	osc->timePosition->setValue(value);
}

double Oscilloscope_API::getTimeBase() const
{
	return osc->timeBase->value();
}

void Oscilloscope_API::setTimeBase(double value)
{
	osc->timeBase->setValue(value);
}

void Oscilloscope_API::setMemoryDepth(int val)
{
	bool ok = false;
	unsigned long buffersize = 0;
	int i = 0;
	QString currentText;
	for (i = 0; i < osc->ch_ui->cmbMemoryDepth->count(); i++) {
		ok = false;
		currentText = osc->ch_ui->cmbMemoryDepth->itemText(i);
		buffersize = currentText.toInt(&ok);
		if (ok && (val == buffersize)) {
			break;
		}
	}
	if (i >= osc->ch_ui->cmbMemoryDepth->count()) {
		i = 0;
	}
	osc->ch_ui->cmbMemoryDepth->setCurrentIndex(i);
}

int Oscilloscope_API::getMemoryDepth()
{
	bool ok;
	QString currentText = osc->ch_ui->cmbMemoryDepth->currentText();
	unsigned long bufferSize = currentText.toInt(&ok);
	return bufferSize;
}

void Oscilloscope_API::show()
{
	Q_EMIT osc->showTool();
}

bool Oscilloscope_API::running() const
{
	return osc->ui->runSingleWidget->runButtonChecked() || osc->ui->runSingleWidget->singleButtonChecked();
}

void Oscilloscope_API::run(bool en)
{
	osc->ui->runSingleWidget->toggle(en);
}

bool Oscilloscope_API::isSingle() const
{
	return osc->ui->runSingleWidget->singleButtonChecked();
}
void Oscilloscope_API::single(bool en)
{
	if(!osc->ui->runSingleWidget->singleButtonChecked())
		osc->ui->runSingleWidget->single();
}

QList<int> Oscilloscope_API::measureEn() const
{
	QList<int> list;

	for (unsigned int i = 0; i < osc->nb_channels; i++) {
		auto measurements = osc->plot.measurements(i);
		int mask = 0;

		if (measurements.size() > (sizeof(int) * 8))
			throw std::runtime_error("Too many measurements");

		for (unsigned int j = 0; j < measurements.size(); j++) {
			if (measurements[j]->enabled())
				mask |= 1 << j;
		}

		list.append(mask);
	}

	return list;
}

void Oscilloscope_API::setMeasureEn(const QList<int>& list)
{
	if (list.size() != osc->nb_channels)
		return;

	osc->measure_settings->m_selectedMeasurements.clear();

	for (unsigned int i = 0; i < osc->nb_channels; i++) {
		auto measurements = osc->plot.measurements(i);
		int mask = list.at(i);

		if (measurements.size() > (sizeof(int) * 8))
			throw std::runtime_error("Too many measurements");

		for (unsigned int j = 0; j < measurements.size(); j++) {
			measurements[j]->setEnabled(!!(mask & (1 << j)));
			osc->measure_settings->onMeasurementActivated(
					i, j, !!(mask & (1 << j)));
		}
	}

	osc->measure_settings->loadMeasurementStatesFromData();
	osc->onMeasurementSelectionListChanged();
}

QList<int> Oscilloscope_API::statisticEn() const
{
	QList<int> list;
	auto statistics = osc->measure_settings->statisticSelection();

	for (int i = 0; i < osc->nb_channels; i++) {
		int mask = 0;
		list.append(mask);
	}
	for (unsigned int i = 0; i < statistics.size(); i++)
		list[statistics[i].channel_id()] |= 1 << statistics[i].id();

	return list;
}

void Oscilloscope_API::setStatisticEn(const QList<int>& list)
{
	if (list.size() != osc->nb_channels)
		return;

	osc->measure_settings->deleteAllStatistics();
	for (unsigned int i = 0; i < osc->nb_channels; i++) {
		auto measurements = osc->plot.measurements(i);
		int mask = list.at(i);

		if (measurements.size() > (sizeof(int) * 8))
			throw std::runtime_error("Too many measurements");

		for (unsigned int j = 0; j < measurements.size(); j++) {
			if (!!(mask & (1 << j)))
				osc->measure_settings->addStatistic(j, i);
		}
	}

	osc->measure_settings->loadStatisticStatesForChannel(
		osc->current_channel);
	osc->onStatisticSelectionListChanged();
}

int Oscilloscope_API::getCurrentChannel() const
{
	return osc->current_channel;
}

void Oscilloscope_API::setCurrentChannel(int chn_id)
{
	ChannelWidget *chn_widget = osc->channelWidgetAtId(chn_id);
	if (!chn_widget)
		return;

	if (chn_widget->enableButton()->isChecked()) {
		osc->setChannelWidgetIndex(chn_id);
		osc->current_channel = chn_id;
		chn_widget->nameButton()->setChecked(true);
		chn_widget->menuButton()->setChecked(true);
	}
}

bool Oscilloscope_API::getFftEn() const
{
	return osc->fft_is_visible;
}

void Oscilloscope_API::setFftEn(bool en)
{
	osc->gsettings_ui->FFT_view->setChecked(en);
}

bool Oscilloscope_API::getXyEn() const
{
	return osc->xy_is_visible;
}

void Oscilloscope_API::setXyEn(bool en)
{
	osc->gsettings_ui->XY_view->setChecked(en);
}

bool Oscilloscope_API::getHistEn() const
{
	return osc->hist_is_visible;
}

void Oscilloscope_API::setHistEn(bool en)
{
	osc->gsettings_ui->Histogram_view->setChecked(en);
}

bool Oscilloscope_API::getExportAll() const
{
	return osc->exportSettings->getExportAllButton()->isChecked();
}

void Oscilloscope_API::setExportAll(bool en)
{
	osc->exportSettings->getExportAllButton()->setChecked(en);
}

int Oscilloscope_API::getCursorsPosition() const
{
	if (!hasCursors()) {
		return 0;
	}
	auto currentPos = osc->plot.d_cursorReadouts->getCurrentPosition();
	switch (currentPos) {
	case CustomPlotPositionButton::ReadoutsPosition::topLeft:
	default:
		return 0;
	case CustomPlotPositionButton::ReadoutsPosition::topRight:
		return 1;
	case CustomPlotPositionButton::ReadoutsPosition::bottomLeft:
		return 2;
	case CustomPlotPositionButton::ReadoutsPosition::bottomRight:
		return 3;
	}
}

void Oscilloscope_API::setCursorsPosition(int val)
{
	if (!hasCursors()) {
		return;
	}
	enum CustomPlotPositionButton::ReadoutsPosition types[] = {
		CustomPlotPositionButton::ReadoutsPosition::topLeft,
		CustomPlotPositionButton::ReadoutsPosition::topRight,
		CustomPlotPositionButton::ReadoutsPosition::bottomLeft,
		CustomPlotPositionButton::ReadoutsPosition::bottomRight
	};
	osc->cursorsPositionButton->setPosition(types[val]);
	osc->plot.replot();
}

int Oscilloscope_API::getCursorsTransparency() const
{
	if (!hasCursors()) {
		return 0;
	}
	return osc->cr_ui->horizontalSlider->value();
}

void Oscilloscope_API::setCursorsTransparency(int val)
{
	if (!hasCursors()) {
		return;
	}
	osc->cr_ui->horizontalSlider->setValue(val);
}

bool Oscilloscope_API::gatingEnabled() const
{
	return osc->measure_settings->m_ui->button_GatingEnable->isChecked();
}

void Oscilloscope_API::setGatingEnabled(bool en)
{
	osc->measure_settings->m_ui->button_GatingEnable->setChecked(en);
}

double Oscilloscope_API::cursorGateLeft() const
{
	return osc->plot.value_gateLeft;
}

double Oscilloscope_API::cursorGateRight() const
{
	return osc->plot.value_gateRight;
}

void Oscilloscope_API::setCursorGateLeft(double val)
{
	osc->plot.d_gateBar1->setPosition(val);
}

void Oscilloscope_API::setCursorGateRight(double val)
{
	osc->plot.d_gateBar2->setPosition(val);
}

QVariantList Oscilloscope_API::getChannels()
{
	QVariantList list;

	for (Channel_API *each : osc->channels_api)
		list.append(QVariant::fromValue(each));

	return list;
}

/*
 * Channel_API
 */

bool Channel_API::channelEn() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	ChannelWidget *w = osc->channelWidgetAtId(index);

	return w->enableButton()->isChecked();
}

void Channel_API::setChannelEn(bool en)
{
	int index = osc->channels_api.indexOf(this);
	ChannelWidget *w = osc->channelWidgetAtId(index);

	w->enableButton()->setChecked(en);
}

double Channel_API::getVoltsPerDiv() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	if(index<0)
		return 1.0;
	return osc->plot.VertUnitsPerDiv(index);
}

void Channel_API::setVoltsPerDiv(double val)
{
	int index = osc->channels_api.indexOf(this);
	int prevSelectedChannel = osc->current_ch_widget;
	if (index == prevSelectedChannel) {
		osc->voltsPerDiv->setValue(val);
		return;
	}
	QWidget *obj = osc->ui->channelsList->itemAt(index)->widget();
	ChannelWidget *cw = static_cast<ChannelWidget *>(obj);
	if (cw) {
		cw->menuButton()->setChecked(true);
		if (osc->voltsPerDiv->value() == val) {
			osc->onVertScaleValueChanged(val);
		} else {
			osc->voltsPerDiv->setValue(val);
		}
	}

	if (prevSelectedChannel < 0) {
		return;
	}
	QWidget *prevObj = osc->ui->channelsList->itemAt(prevSelectedChannel)->widget();
	ChannelWidget *prevCw = static_cast<ChannelWidget *>(prevObj);
	if (prevCw) {
		prevCw->menuButton()->setChecked(true);
	}
}

double Channel_API::getVOffset() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	return -osc->plot.VertOffset(index);
}

void Channel_API::setVOffset(double val)
{
	int index = osc->channels_api.indexOf(this);
	int prevSelectedChannel = osc->current_ch_widget;
	if (index == prevSelectedChannel) {
		osc->voltsPosition->setValue(val);
		return;
	}

	QWidget *obj = osc->ui->channelsList->itemAt(index)->widget();
	ChannelWidget *cw = static_cast<ChannelWidget *>(obj);
	if (cw) {
		cw->menuButton()->setChecked(true);
		if (osc->voltsPosition->value() == val) {
			osc->onVertOffsetValueChanged(val);
		} else {
			osc->voltsPosition->setValue(val);
		}
	}

	if (prevSelectedChannel < 0) {
		return;
	}
	QWidget *prevObj = osc->ui->channelsList->itemAt(prevSelectedChannel)->widget();
	ChannelWidget *prevCw = static_cast<ChannelWidget *>(prevObj);
	if (prevCw) {
		prevCw->menuButton()->setChecked(true);
	}
}

double Channel_API::getLineThickness() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));

	return osc->plot.getLineWidthF(index);
}

void Channel_API::setLineThickness(double val)
{
	int index = osc->channels_api.indexOf(this);
	int cmbIdx = (int)(val / 0.5) - 1;
	if (cmbIdx > osc->ch_ui->cmbChnLineWidth->count()) {
		cmbIdx = osc->ch_ui->cmbChnLineWidth->count() - 1;
		val = (cmbIdx + 1) * 0.5;
	}
	if (cmbIdx < 0) {
		cmbIdx = 0;
		val = 0.5;
	}
	if (index == osc->current_ch_widget) {
		osc->ch_ui->cmbChnLineWidth->setCurrentIndex(cmbIdx);
	}
	osc->plot.setLineWidthF(index, val);
}

double Channel_API::getProbeAttenuation() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));

	return osc->probe_attenuation[index];
}

void Channel_API::setProbeAttenuation(double val)
{
	int index = osc->channels_api.indexOf(this);

	int idx = std::log10(val / 0.1);
	if (idx >= osc->ch_ui->probe_attenuation->count()) {
		idx = osc->ch_ui->probe_attenuation->count() - 1;
	} else if (idx <= 0) {
		idx = 0;
	}
	val = std::pow(10, idx) * 0.1;

	if (index == osc->current_ch_widget) {
		osc->ch_ui->probe_attenuation->setCurrentIndex(idx);
	} else {
		osc->probe_attenuation[index] = val;
	}
}

void Channel_API::setColor(int r, int g, int b, int a)
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	QWidget *obj = osc->ui->channelsList->itemAt(index)->widget();
	ChannelWidget *cw = static_cast<ChannelWidget *>(obj);
	QColor color(r, g, b, a);
	if (color.isValid()) {
		osc->plot.setLineColor(index, color);
		osc->plot.d_offsetHandles.at(index)->setRoundRectColor(color);
		osc->plot.d_offsetHandles.at(index)->setPen(QPen(color, 2, Qt::SolidLine));
		static_cast<QLabel *>(
			osc->ui->chn_scales->itemAt(index)->widget())->setStyleSheet(QString("QLabel {"
						"color: %1;"
						"font-weight: bold;"
						"}").arg(color.name()));
		if (cw) {
			cw->setColor(osc->plot.getLineColor(index));
		}
		if (!osc->runButton()->isChecked()) {
			osc->plot.replot();
		}
	}
}

bool Channel_API::getAcCoupling() const
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	return osc->chnAcCoupled[index];
}

void Channel_API::setAcCoupling(bool val)
{
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	if(index < 0)
		return;
	if (osc->current_channel == index) {
		osc->ch_ui->btnCoupled->setChecked(val);
	} else {
		osc->configureAcCoupling(index, val);
	}
}

QList<double> Channel_API::data() const
{
	QList<double> list;
	int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));
	if(index < 0)
		return list;
	/*if(osc->ui->pushButtonRunStop->isChecked() ||
	   osc->ui->pushButtonSingle->isChecked())
		return list;*/
	auto num_of_samples = osc->plot.Curve(index)->data()->size();
	for(auto i=0; i<num_of_samples;i++)
	{
		double d =  osc->plot.Curve(index)->data()->sample(i).y();
		list.append(d);
	}
	return list;
}

#define DECLARE_MEASURE(m, t) \
	double Channel_API::measured_ ## m () const\
	{\
		int index = osc->channels_api.indexOf(const_cast<Channel_API*>(this));\
		auto measData = osc->plot.measurement(Measure::t, index);\
		return measData->value();\
	}
DECLARE_MEASURE(period, PERIOD)
DECLARE_MEASURE(frequency, FREQUENCY)
DECLARE_MEASURE(min, MIN)
DECLARE_MEASURE(max, MAX)
DECLARE_MEASURE(peak_to_peak, PEAK_PEAK)
DECLARE_MEASURE(mean, MEAN)
DECLARE_MEASURE(cycle_mean, CYCLE_MEAN)
DECLARE_MEASURE(rms, RMS)
DECLARE_MEASURE(cycle_rms, CYCLE_RMS)
DECLARE_MEASURE(ac_rms, AC_RMS)
DECLARE_MEASURE(area, AREA)
DECLARE_MEASURE(cycle_area, CYCLE_AREA)
DECLARE_MEASURE(low, LOW)
DECLARE_MEASURE(high, HIGH)
DECLARE_MEASURE(amplitude, AMPLITUDE)
DECLARE_MEASURE(middle, MIDDLE)
DECLARE_MEASURE(pos_overshoot, P_OVER)
DECLARE_MEASURE(neg_overshoot, N_OVER)
DECLARE_MEASURE(rise, RISE)
DECLARE_MEASURE(fall, FALL)
DECLARE_MEASURE(pos_width, P_WIDTH)
DECLARE_MEASURE(neg_width, N_WIDTH)
DECLARE_MEASURE(pos_duty, P_DUTY)
DECLARE_MEASURE(neg_duty, N_DUTY)
}
