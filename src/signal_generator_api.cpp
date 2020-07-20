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
#include "signal_generator_api.hpp"
#include "ui_signal_generator.h"
#include "channel_widget.hpp"
#include "spinbox_a.hpp"

namespace adiscope {
void SignalGenerator_API::show()
{
	Q_EMIT gen->showTool();
}

bool SignalGenerator_API::running() const
{
	return gen->ui->run_button->isChecked();
}

void SignalGenerator_API::run(bool en)
{
	gen->ui->run_button->setChecked(en);
}

QList<int> SignalGenerator_API::getMode() const
{
	QList<int> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<int>(ptr->type));
	}

	return list;
}

void SignalGenerator_API::setMode(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->type = static_cast<enum SIGNAL_TYPE>(list.at(i));
	}

	gen->ui->tabWidget->setCurrentIndex(gen->getCurrentData()->type);
}


QList<bool> SignalGenerator_API::enabledChannels() const
{
	QList<bool> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		list.append(static_cast<bool>(gen->channels[i]->enableButton()->isChecked()));
	}

	return list;
}

void SignalGenerator_API::enableChannels(const QList<bool>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		gen->channels[i]->enableButton()->setChecked(list.at(i));
	}
}


QList<double> SignalGenerator_API::getConstantValue() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<double>(ptr->constant));
	}

	return list;
}

void SignalGenerator_API::setConstantValue(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->constant = static_cast<float>(list.at(i));
	}

	gen->constantValue->setValue(gen->getCurrentData()->constant);
}

QList<int> SignalGenerator_API::getWaveformType() const
{
	QList<int> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);
		list.append(SignalGenerator::sg_waveform_to_idx(ptr->waveform));
	}

	return list;
}

void SignalGenerator_API::setWaveformType(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	enum sg_waveform types[] = {
		SG_SIN_WAVE,
		SG_SQR_WAVE,
		SG_TRI_WAVE,
		SG_TRA_WAVE,
		SG_SAW_WAVE,
		SG_INV_SAW_WAVE,
		SG_STAIR_WAVE,
	};

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->waveform = types[list.at(i)];
		if (i == gen->currentChannel) {
			gen->ui->type->setCurrentIndex(list.at(i));
			gen->updateRightMenuForChn(i);
            gen->resetZoom();
		}
	}
}

QList<double> SignalGenerator_API::getWaveformAmpl() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->amplitude);
	}

	return list;
}

void SignalGenerator_API::setWaveformAmpl(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->amplitude = list.at(i);
	}

	gen->amplitude->setValue(gen->getCurrentData()->amplitude);
}

QList<double> SignalGenerator_API::getWaveformFreq() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->frequency);
	}

	return list;
}

void SignalGenerator_API::setWaveformFreq(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->frequency = list.at(i);
	}

	gen->frequency->setValue(gen->getCurrentData()->frequency);
}

QList<double> SignalGenerator_API::getWaveformOfft() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<double>(ptr->offset));
	}

	return list;
}

void SignalGenerator_API::setWaveformOfft(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->offset = static_cast<float>(list.at(i));
	}

	gen->offset->setValue(gen->getCurrentData()->offset);
}

QList<int> SignalGenerator_API::getStairWaveformStepsUp() const
{
	QList<int> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<int>(ptr->steps_up));
	}

	return list;
}

void SignalGenerator_API::setStairWaveformStepsUp(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->steps_up = static_cast<int>(list.at(i));
	}

	gen->stepsUp->setValue(gen->getCurrentData()->steps_up);
}

QList<int> SignalGenerator_API::getStairWaveformStepsDown() const
{
	QList<int> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<int>(ptr->steps_down));
	}

	return list;
}

void SignalGenerator_API::setStairWaveformStepsDown(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->steps_down = static_cast<int>(list.at(i));
	}

	gen->stepsDown->setValue(gen->getCurrentData()->steps_down);
}
QList<int> SignalGenerator_API::getStairWaveformPhase() const
{
	QList<int> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(static_cast<int>(ptr->stairphase));
	}

	return list;
}

void SignalGenerator_API::setStairWaveformPhase(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->stairphase = static_cast<int>(list.at(i));
	}

	gen->stairPhase->setValue(gen->getCurrentData()->stairphase);
}

QList<double> SignalGenerator_API::getWaveformPhase() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->phase);
	}

	return list;
}

void SignalGenerator_API::setWaveformPhase(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->phase = list.at(i);
	}

	gen->phase->setValue(gen->getCurrentData()->phase);
}


QList<double> SignalGenerator_API::getWaveformDuty() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->dutycycle);
	}

	return list;
}

void SignalGenerator_API::setWaveformDuty(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->dutycycle = list.at(i);
	}

	gen->dutycycle->setValue(gen->getCurrentData()->dutycycle);
}

QList<int> SignalGenerator_API::getNoiseType() const
{
	QList<int> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);
		list.append(ptr->noiseType);
	}

	return list;
}

void SignalGenerator_API::setNoiseType(const QList<int>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->noiseType = qvariant_cast<gr::analog::noise_type_t>(list.at(i));

		if (i == gen->currentChannel) {
			gen->updateRightMenuForChn(i);
		}
	}
}


QList<double> SignalGenerator_API::getNoiseAmpl() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->noiseAmplitude);
	}

	return list;
}

void SignalGenerator_API::setNoiseAmpl(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);
		ptr->noiseAmplitude = list.at(i);
	}

	gen->noiseAmplitude->setValue(gen->getCurrentData()->noiseAmplitude);
	gen->resetZoom();
}


QList<double> SignalGenerator_API::getWaveformRise() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->rise);
	}

	return list;
}

void SignalGenerator_API::setWaveformRise(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->rise = list.at(i);
	}

	gen->riseTime->setValue(gen->getCurrentData()->rise);
}


QList<double> SignalGenerator_API::getWaveformFall() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->fall);
	}

	return list;
}

void SignalGenerator_API::setWaveformFall(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->fall = list.at(i);
	}

	gen->fallTime->setValue(gen->getCurrentData()->fall);
}


QList<double> SignalGenerator_API::getWaveformHoldHigh() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->holdh);
	}

	return list;
}

void SignalGenerator_API::setWaveformHoldHigh(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->holdh = list.at(i);
	}

	gen->holdHighTime->setValue(gen->getCurrentData()->holdh);
}


QList<double> SignalGenerator_API::getWaveformHoldLow() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->holdl);
	}

	return list;
}

void SignalGenerator_API::setWaveformHoldLow(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->holdl = list.at(i);
	}

	gen->holdLowTime->setValue(gen->getCurrentData()->holdl);
}



QList<double> SignalGenerator_API::getMathFreq() const
{
	QList<double> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->math_freq);
	}

	return list;
}

void SignalGenerator_API::setMathFreq(const QList<double>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->math_freq = list.at(i);
	}

	gen->mathFrequency->setValue(gen->getCurrentData()->math_freq);
}

QList<QString> SignalGenerator_API::getMathFunction() const
{
	QList<QString> list;

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		list.append(ptr->function);
	}

	return list;
}

void SignalGenerator_API::setMathFunction(const QList<QString>& list)
{
	if (list.size() != gen->channels.size()) {
		return;
	}

	for (int i = 0; i < gen->channels.size(); i++) {
		auto ptr = gen->getData(gen->channels[i]);

		ptr->function = list.at(i);
	}

	if (gen->getCurrentData()->type == SIGNAL_TYPE_MATH) {
		gen->ui->mathWidget->setFunction(
		        gen->getCurrentData()->function);
			gen->resetZoom();
	}

}

QList<QString> SignalGenerator_API::getBufferFilePath() const{
    QList<QString> list;

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        if(ptr->file!="" && ptr->file_type!=FORMAT_NO_FILE){
            list.append(ptr->file);
        }
        else{
            list.append("");
        }

    }

    return list;
}
void SignalGenerator_API::setBufferFilePath(const QList<QString>& list){
    if (list.size() != gen->channels.size()) {
        return;
    }

    auto currentChannel = gen->currentChannel;
    for (int i = 0; i < gen->channels.size(); i++) {
        if(list.at(i) != ""){
            gen->currentChannel = i;
            gen->loadFileFromPath(list.at(i));
        }
    }
    gen->currentChannel = currentChannel;
    gen->updateRightMenuForChn(gen->currentChannel);
}

QList<double> SignalGenerator_API::getBufferAmplitude() const{
    QList<double> list;

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        list.append(ptr->file_amplitude);
    }

    return list;
}

void SignalGenerator_API::setBufferAmplitude(const QList<double>& list){
    if (list.size() != gen->channels.size()) {
        return;
    }

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        ptr->file_amplitude = list.at(i);
	if(i == gen->currentChannel){
	    gen->resetZoom();
        }
    }

    gen->fileAmplitude->setValue(gen->getCurrentData()->file_amplitude);
}

QList<double> SignalGenerator_API::getBufferOffset() const{
    QList<double> list;

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        list.append(ptr->file_offset);
    }

    return list;
}

void SignalGenerator_API::setBufferOffset(const QList<double>& list){
    if (list.size() != gen->channels.size()) {
        return;
    }

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        ptr->file_offset = list.at(i);
	if(i == gen->currentChannel){
	    gen->resetZoom();
        }
    }
    gen->fileOffset->setValue(gen->getCurrentData()->file_offset);
}

QList<double> SignalGenerator_API::getBufferSampleRate() const{
    QList<double> list;

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        list.append(ptr->file_sr);
    }

    return list;
}

void SignalGenerator_API::setBufferSampleRate(const QList<double>& list){
    if (list.size() != gen->channels.size()) {
        return;
    }

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        ptr->file_sr = list.at(i);
	if(i == gen->currentChannel){
	    gen->resetZoom();
	}
    }
    gen->fileSampleRate->setValue(gen->getCurrentData()->file_sr);
}

QList<double> SignalGenerator_API::getBufferPhase() const{
    QList<double> list;

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        list.append(ptr->file_phase);
    }

    return list;
}

void SignalGenerator_API::setBufferPhase(const QList<double>& list){
    if (list.size() != gen->channels.size()) {
        return;
    }

    for (int i = 0; i < gen->channels.size(); i++) {
        auto ptr = gen->getData(gen->channels[i]);

        ptr->file_phase = list.at(i);
	if(i == gen->currentChannel){
	    gen->resetZoom();
        }
    }
    gen->filePhase->setValue(gen->getCurrentData()->file_phase);
}
}
