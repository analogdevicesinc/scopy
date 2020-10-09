/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "spectrum_analyzer_api.hpp"
#include "ui_spectrum_analyzer.h"
#include "ui_cursors_settings.h"
#include "channel_widget.hpp"
#include "db_click_buttons.hpp"

namespace adiscope {
int SpectrumChannel_API::type()
{
	return	spch->averageType();
}

int SpectrumChannel_API::window()
{
	return spch->fftWindow();
}

int SpectrumChannel_API::averaging()
{
	return spch->averaging();
}

float SpectrumChannel_API::lineThickness()
{
    return spch->lineWidth();
}

bool SpectrumChannel_API::enabled()
{
	return spch->widget()->enableButton()->isChecked();
}

void SpectrumChannel_API::enable(bool en)
{
	spch->widget()->enableButton()->setChecked(en);
}

void SpectrumChannel_API::setType(int type)
{
	spch->setAverageType((adiscope::FftDisplayPlot::AverageType)type);
}

void SpectrumChannel_API::setWindow(int win)
{
	auto taps=sp->fft_size;
	spch->setFftWindow((adiscope::SpectrumAnalyzer::FftWinType)win,taps);
}

void SpectrumChannel_API::setAveraging(int avg)
{
	spch->setAveraging(avg);
}

void SpectrumChannel_API::setLineThickness(float val)
{
    spch->setLinewidth(val);
}

QList<double> SpectrumChannel_API::data() const
{
	QList<double> list;
	int i = sp->ch_api.indexOf(const_cast<SpectrumChannel_API*>(this));
	int nr_samples = sp->fft_plot->Curve(0)->data()->size();
	for (int j = 0; j < nr_samples; ++j) {
		list.push_back(sp->fft_plot->Curve(i)->sample(j).y());
	}
	return list;
}

QList<double> SpectrumChannel_API::freq() const
{
	QList<double> frequency_data;
	int nr_samples = sp->fft_plot->Curve(0)->data()->size();
	for (int i = 0; i < nr_samples; ++i) {
		frequency_data.push_back(sp->fft_plot->Curve(0)->sample(i).x());
	}
	return frequency_data;
}

int SpectrumMarker_API::chId()
{
	return m_chid;
}

void SpectrumMarker_API::setChId(int val)
{
	m_chid=val;
}

int SpectrumMarker_API::mkId()
{
	return m_mkid;
}

void SpectrumMarker_API::setMkId(int val)
{
	m_mkid=val;
}

int SpectrumMarker_API::type()
{
	if (sp->fft_plot->markerEnabled(m_chid,m_mkid)) {
		return sp->fft_plot->markerType(m_chid,m_mkid);
	}
	return -1;

}

void SpectrumMarker_API::setType(int val)
{
	if (val > sp->markerTypes.size()) {
		val = 0;
	}

	m_type = val;

	if (m_type == 1) { //if type is peak
		sp->fft_plot->marker_to_max_peak(m_chid, m_mkid);
	}
}

double SpectrumMarker_API::freq()
{
	if (sp->fft_plot->markerEnabled(m_chid,m_mkid)) {
		return sp->fft_plot->markerFrequency(m_chid,m_mkid);
	} else {
		return 0;
	}
}

void SpectrumMarker_API::setFreq(double pos)
{
	if (sp->fft_plot->markerEnabled(m_chid,m_mkid)) {
		if (m_type != 1) { //if type is not peak
			sp->fft_plot->setMarkerAtFreq(m_chid,m_mkid,pos);
		}

		//sp->crt_channel_id=m_chid;
		sp->updateWidgetsRelatedToMarker(m_mkid);
		sp->fft_plot->updateMarkerUi(m_chid, m_mkid);

		if (sp->crt_channel_id==m_chid) {
			sp->marker_selector->blockSignals(true);
			sp->marker_selector->setButtonChecked(m_mkid, true);
			sp->marker_selector->blockSignals(false);
		}

		if (m_type != 1) { //if type is not peak
			sp->ui->markerTable->updateMarker(m_mkid, m_chid,
			                                  sp->fft_plot->markerFrequency(m_chid, m_mkid),
							  sp->fft_plot->markerMagnitude(m_chid, m_mkid),
			                                  sp->markerTypes[m_type]);

		}
	}
}

double SpectrumMarker_API::magnitude()
{
	if (sp->fft_plot->markerEnabled(m_chid,m_mkid)) {
		return sp->fft_plot->markerMagnitude(m_chid,m_mkid);
	} else {
		return 0;
	}
}

bool SpectrumMarker_API::enabled()
{
	return sp->fft_plot->markerEnabled(m_chid,m_mkid);
}

void SpectrumMarker_API::setEnabled(bool en)
{
	bool enabled = sp->channels[m_chid]->widget()->enableButton()->isChecked();
	sp->channels[m_chid]->widget()->enableButton()->setChecked(true);
	sp->channels[m_chid]->widget()->nameButton()->setChecked(true);
	sp->fft_plot->setMarkerEnabled(m_chid,m_mkid,en);
	sp->marker_selector->setButtonChecked(m_mkid, en);
	sp->updateWidgetsRelatedToMarker(m_mkid);
	sp->fft_plot->updateMarkerUi(m_chid, m_mkid);
	sp->channels[m_chid]->widget()->enableButton()->setChecked(enabled);
}

bool SpectrumMarker_API::visible()
{
	return sp->fft_plot->markerVisible(m_chid,m_mkid);
}

void SpectrumMarker_API::setVisible(bool en)
{
	sp->fft_plot->setMarkerVisible(m_chid, m_mkid, en);
}

void SpectrumAnalyzer_API::show()
{
	Q_EMIT sp->showTool();
}

QVariantList SpectrumAnalyzer_API::getMarkers()
{
	QVariantList list;

	for (SpectrumMarker_API *each : qAsConst(sp->marker_api)) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}


bool SpectrumAnalyzer_API::hasCursors() const
{
	return sp->ui->boxCursors->isChecked();
}

void SpectrumAnalyzer_API::setCursors(bool en)
{
	sp->ui->boxCursors->setChecked(en);
}

bool SpectrumAnalyzer_API::running()
{
	return sp->runButton()->isChecked();
}

void SpectrumAnalyzer_API::run(bool chk)
{
	sp->ui->runSingleWidget->toggle(chk);
}

bool SpectrumAnalyzer_API::isSingle()
{
	return sp->ui->runSingleWidget->singleButtonChecked();
}
void SpectrumAnalyzer_API::single(bool chk)
{
	sp->ui->runSingleWidget->single();
}

QVariantList SpectrumAnalyzer_API::getChannels()
{
	QVariantList list;

	for (SpectrumChannel_API *each : qAsConst(sp->ch_api)) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}


int SpectrumAnalyzer_API::currentChannel()
{
	return (sp->crt_channel_id < 2 ? sp->crt_channel_id : 0);
}

void SpectrumAnalyzer_API::setCurrentChannel(int ch)
{
	sp->channels[ch]->widget()->nameButton()->setChecked(true);
	sp->channels[ch]->widget()->selected(true);
}

double SpectrumAnalyzer_API::startFreq()
{
	return sp->startStopRange->getStartValue();
}
void SpectrumAnalyzer_API::setStartFreq(double val)
{
	sp->startStopRange->setStartValue(val);
}

double SpectrumAnalyzer_API::stopFreq()
{
	return sp->startStopRange->getStopValue();
}
void SpectrumAnalyzer_API::setStopFreq(double val)
{
	sp->startStopRange->setStopValue(val);
}

QString SpectrumAnalyzer_API::resBW()
{
	return sp->ui->cmb_rbw->currentText();
}
void SpectrumAnalyzer_API::setResBW(QString s)
{
	sp->ui->cmb_rbw->setCurrentText(s);
}


QString SpectrumAnalyzer_API::units()
{
	return sp->ui->cmb_units->currentText();
}

void SpectrumAnalyzer_API::setUnits(QString s)
{
	sp->ui->cmb_units->setCurrentText(s);
}

double SpectrumAnalyzer_API::topScale()
{
	if (sp->ui->topWidget->currentIndex() == 0) {
		return sp->top->value();
	} else {
		return sp->top_scale->value();
	}
}
void SpectrumAnalyzer_API::setTopScale(double val)
{
	if (sp->ui->topWidget->currentIndex() == 0) {
		sp->top->setValue(val);
	} else {
		sp->top_scale->setValue(val);
	}
}

double SpectrumAnalyzer_API::bottomScale()
{
	if (sp->ui->topWidget->currentIndex() == 0) {
		return sp->bottom->value();
	} else {
		return sp->bottom_scale->value();
	}
}
void SpectrumAnalyzer_API::setBottomScale(double val)
{
	if (sp->ui->topWidget->currentIndex() == 0) {
		sp->bottom->setValue(val);
	} else {
		sp->bottom_scale->setValue(val);
	}
}

double SpectrumAnalyzer_API::unitPerDiv()
{
	if (sp->ui->topWidget->currentIndex() == 0) {
		return sp->unit_per_div->value();
	}
	return 0;
}
void SpectrumAnalyzer_API::setunitPerDiv(double val)
{
	if (sp->ui->topWidget->currentIndex() == 0) {
		sp->unit_per_div->setValue(val);
	}
}

bool SpectrumAnalyzer_API::markerTableVisible()
{
	return sp->ui->btnMarkerTable->isChecked();
}

void SpectrumAnalyzer_API::setMarkerTableVisible(bool en)
{
	sp->ui->btnMarkerTable->setChecked(en);
}

bool SpectrumAnalyzer_API::horizontalCursors() const
{
	return sp->cr_ui->hCursorsEnable->isChecked();
}

void SpectrumAnalyzer_API::setHorizontalCursors(bool en)
{
	sp->cr_ui->hCursorsEnable->setChecked(en);
}

bool SpectrumAnalyzer_API::verticalCursors() const
{
	return sp->cr_ui->vCursorsEnable->isChecked();
}

void SpectrumAnalyzer_API::setVerticalCursors(bool en)
{
	sp->cr_ui->vCursorsEnable->setChecked(en);
}

bool SpectrumAnalyzer_API::getLogScale() const
{
	return sp->fft_plot->getLogScale();
}

int SpectrumAnalyzer_API::getCursorsPosition() const
{
	if (!hasCursors()) {
		return 0;
	}
	auto currentPos = sp->fft_plot->d_cursorReadouts->getCurrentPosition();
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
	return 3;
}

void SpectrumAnalyzer_API::setCursorsPosition(int val)
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
	sp->cursorsPositionButton->setPosition(types[val]);
	sp->fft_plot->replot();
}

int SpectrumAnalyzer_API::getCursorsTransparency() const
{
	if (!hasCursors()) {
		return 0;
	}
	return sp->cr_ui->horizontalSlider->value();
}

void SpectrumAnalyzer_API::setCursorsTransparency(int val)
{
	if (!hasCursors()) {
		return;
	}
	sp->cr_ui->horizontalSlider->setValue(val);
}

void SpectrumAnalyzer_API::setLogScale(bool useLogScale)
{
	sp->ui->logBtn->setChecked(useLogScale);
}

QString SpectrumAnalyzer_API::getNotes()
{
	return sp->ui->instrumentNotes->getNotes();
}
void SpectrumAnalyzer_API::setNotes(QString str)
{
	sp->ui->instrumentNotes->setNotes(str);
}
}
