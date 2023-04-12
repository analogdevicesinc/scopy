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

#include <QDebug>
#include <QFile>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QPushButton>
#include <QJsonDocument>
#include <QDirIterator>
#include <QFileDialog>
#include <QMap>

#include <errno.h>
#include "patterns.hpp"
#include "gui/annotationcurve.h"
#include "gui/annotationdecoder.h"
#include "gui/dynamicWidget.h"

#include <math.h>

using namespace std;
using namespace adiscope::m2k;

constexpr int PG_MAX_SAMPLERATE = 100000000; // 100MHz

namespace adiscope::m2k {

JSConsole::JSConsole(QObject *parent) :
	QObject(parent)
{
}

void JSConsole::log(QString msg)
{
	qDebug() << "jsConsole: "<< msg;
}

Pattern::Pattern()
{
	// qDebug()<<"PatternCreated";
	buffer = nullptr;
	periodic = true;
	channels = 1;
}

Pattern::~Pattern()
{
	//qDebug()<<"PatternDestroyed";
	delete_buffer();
}

string Pattern::get_name()
{
	return name;
}

void Pattern::set_name(const string &name_)
{
	name = name_;
}

string Pattern::get_description()
{
	return description;
}

void Pattern::set_description(const string &description_)
{
	description = description_;
}

void Pattern::init()
{

}

void Pattern::deinit()
{

}

bool Pattern::is_periodic()
{
	return periodic;
}

void Pattern::set_periodic(bool periodic_)
{
	periodic=periodic_;
}

short *Pattern::get_buffer()
{
	return buffer;
}

void Pattern::delete_buffer()
{
	if (buffer) {
		delete[] buffer;
	}

	buffer=nullptr;
}

uint8_t Pattern::pre_generate()
{
	return 0;
}

std::string Pattern::toString()
{
	return "";
}

bool Pattern::fromString(std::string from)
{
	return 0;
}

int Pattern::nrOfChannels() const
{
	return channels;
}

void Pattern::setNrOfChannels(int channels)
{
	this->channels = channels;
}

uint32_t Pattern::get_min_sampling_freq()
{
	return 1000; // minimum 1 kHz if not specified otherwise
}

uint32_t Pattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	return 0; // 0 samples required
}

uint32_t changeBit(uint32_t number,uint8_t n, bool x)
{
	number ^= (-x ^ number) & (1 << n);
	return number;
}

Pattern *Pattern_API::fromString(QString str)
{
	/*QJsonValue val;

	val = QJsonObject(str);
	if(val.isNull())
	{
		qDebug()<<"Invalid String";
		return PatternFactory::create(0);
	}*/

	QJsonObject obj;
	QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());


	if (!doc.isNull()) {
		if (doc.isObject()) {
			obj = doc.object();
		} else {
			qDebug() << "Document is not an object" << Qt::endl;
		}
	} else {
		qDebug() << "Invalid JSON...\n" << str << Qt::endl;
	}

	return fromJson(obj);
}

QString Pattern_API::toString(Pattern *p)
{
	QJsonValue val;
	val = toJson(p);
	QJsonDocument doc(val.toObject());
	QString ret(doc.toJson(QJsonDocument::Compact));
	return ret;
}

QJsonValue Pattern_API::toJson(Pattern *p)
{
	QJsonObject obj;
	ClockPattern *cp = dynamic_cast<ClockPattern *>(p);
	NumberPattern *np = dynamic_cast<NumberPattern *>(p);
	RandomPattern *rp = dynamic_cast<RandomPattern *>(p);
	BinaryCounterPattern *bcp = dynamic_cast<BinaryCounterPattern *>(p);
	GrayCounterPattern *gcp = dynamic_cast<GrayCounterPattern *>(p);
	UARTPattern *up = dynamic_cast<UARTPattern *>(p);
	SPIPattern *sp = dynamic_cast<SPIPattern *>(p);
	I2CPattern *ip = dynamic_cast<I2CPattern *>(p);
	ImportPattern *imp = dynamic_cast<ImportPattern *>(p);
	PulsePattern *pp = dynamic_cast<PulsePattern *>(p);

	QJsonObject params;

	if (cp) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["freq"]  = QJsonValue(cp->get_frequency());
		params["duty"]  = QJsonValue(cp->get_duty_cycle());
		params["phase"] = QJsonValue(cp->get_phase());
		obj["params"] = QJsonValue(params);
	} else if (np) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["nr"]  = QJsonValue((qint64)np->get_nr());
		obj["params"] = QJsonValue(params);
	} else if (rp) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["freq"]  = QJsonValue((qint64)rp->get_frequency());
		obj["params"] = QJsonValue(params);
	} else if (bcp) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["freq"]  = QJsonValue((qint64)bcp->get_frequency());
		obj["params"] = QJsonValue(params);
	} else if (gcp) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["freq"]  = QJsonValue((qint64)gcp->get_frequency());
		obj["params"] = QJsonValue(params);
	} else if (up) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["uart"] = QString::fromStdString(up->get_params());
		params["string"] = QString::fromStdString(up->get_string());
		obj["params"] = QJsonValue(params);
	} else if (sp) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["BPF"]=sp->getBytesPerFrame();
		params["IFS"]=sp->getWaitClocks();
		params["freq"]= QJsonValue((qint64)sp->getClkFrequency());
		params["CPHA"]=sp->getCPHA();
		params["CPOL"]=sp->getCPOL();
		params["CS"]=sp->getCSPol();
		params["MSB"]=sp->getMsbFirst();

		QJsonArray spi;

		for (auto val : sp->v) {
			spi.append(val);
		}

		params["v"]=spi;

		obj["params"] = QJsonValue(params);
	} else if (ip) {
		obj["name"] = QString::fromStdString(p->get_name());
		params["BPF"]=ip->getBytesPerFrame();
		params["IFS"]=ip->getInterFrameSpace();
		params["freq"]= QJsonValue((qint64)ip->getClkFrequency());
		params["MSB"]=ip->getMsbFirst();
		params["write"]=ip->getWrite();
		params["address"]=ip->getAddress();

		QJsonArray i2c;

		for (auto val : ip->v) {
			i2c.append(val);
		}

		params["v"]=i2c;

		obj["params"] = QJsonValue(params);
	} else if (imp) {
		obj["name"] = QString::fromStdString(imp->get_name());
		params["frequency"] =  QJsonValue((qint64)imp->get_frequency());
		params["file"] = imp->fileName;
		params["channel_mapping"] = QJsonValue((imp->channel_mapping));

		obj["params"] = QJsonValue(params);

	} else if (pp) {
		obj["name"] = QString::fromStdString(pp->get_name());
		params["sample_rate"] = pp->get_sample_rate();
		params["high"] = QJsonValue((qint32)pp->get_high_number_of_samples());
		params["low"] = QJsonValue((qint32)pp->get_low_number_of_samples());
		params["start"] = pp->get_start();
		params["cnt_init"] = QJsonValue((qint32)pp->get_counter_init());
		params["delay"] = QJsonValue((qint32)pp->get_delay());
		params["pulses"] = QJsonValue((qint32)pp->get_no_pulses());

		obj["params"] = QJsonValue(params);
	} else {
		obj["name"] = "none";
	}

	return QJsonValue(obj);;
}

Pattern *Pattern_API::fromJson(QJsonObject obj)
{
	//QJsonObject obj = j.toObject();
	Pattern *p = PatternFactory::create(obj["name"].toString());

	ClockPattern *cp = dynamic_cast<ClockPattern *>(p);
	NumberPattern *np = dynamic_cast<NumberPattern *>(p);
	RandomPattern *rp = dynamic_cast<RandomPattern *>(p);
	BinaryCounterPattern *bcp = dynamic_cast<BinaryCounterPattern *>(p);
	BinaryCounterPattern *gcp = dynamic_cast<GrayCounterPattern *>(p);
	UARTPattern *up = dynamic_cast<UARTPattern *>(p);
	SPIPattern *sp = dynamic_cast<SPIPattern *>(p);
	I2CPattern *ip = dynamic_cast<I2CPattern *>(p);
	ImportPattern *imp = dynamic_cast<ImportPattern *>(p);
	PulsePattern *pp = dynamic_cast<PulsePattern *>(p);

	QJsonObject params = obj["params"].toObject();

	if (cp) {
		cp->set_frequency(params["freq"].toDouble());
		cp->set_duty_cycle(params["duty"].toDouble());
		cp->set_phase(params["phase"].toInt());
	} else if (np) {
		np->set_nr(params["nr"].toInt());
	} else if (rp) {
		rp->set_frequency(params["freq"].toInt());
	} else if (bcp) {
		bcp->set_frequency(params["freq"].toInt());
	} else if (gcp) {
		gcp->set_frequency(params["freq"].toInt());
	} else if (up) {
		up->set_string(params["string"].toString().toStdString());
		up->set_params(params["uart"].toString().toStdString());
	} else if (sp) {
		sp->setBytesPerFrame(params["BPF"].toInt());
		sp->setClkFrequency(params["freq"].toInt());
		sp->setWaitClocks(params["IFS"].toInt());
		sp->setCPHA(params["CPHA"].toBool());
		sp->setCPOL(params["CPOL"].toBool());
		sp->setCSPol(params["CS"].toBool());
		sp->setMsbFirst(params["MSB"].toBool());

		auto paramsContainer = params["v"].toArray();
		for (const auto &val : qAsConst(paramsContainer)) {
			sp->v.push_back(val.toInt());
		}
	} else if (ip) {
		ip->setBytesPerFrame(params["BPF"].toInt());
		ip->setClkFrequency(params["freq"].toInt());
		ip->setInterFrameSpace(params["IFS"].toInt());
		ip->setAddress(params["address"].toInt());
		ip->setMsbFirst(params["MSB"].toBool());
		ip->setWrite(params["write"].toBool());

		auto paramsContainer = params["v"].toArray();
		for (const auto &val : qAsConst(paramsContainer)) {
			ip->v.push_back(val.toInt());
		}
	} else if (imp) {
		imp->fileName = params["file"].toString();
		imp->channel_mapping = params["channel_mapping"].toInt();
		imp->setFrequency(params["frequency"].toDouble());
	} else if (pp) {
		pp->set_counter_init(params["cnt_init"].toInt());
		pp->set_high_number_of_samples(params["high"].toInt());
		pp->set_low_number_of_samples(params["low"].toInt());
		pp->set_delay(params["delay"].toInt());
		pp->set_no_pulses(params["pulses"].toInt());
		pp->set_sample_rate(params["sample_rate"].toDouble());
		pp->set_start(params["start"].toBool());
	}

	return p;
}

PatternUI::PatternUI(QWidget *parent) : QWidget(parent)
{
	qDebug()<<"PatternUICreated";
}

PatternUI::~PatternUI()
{
	qDebug()<<"PatternUIDestroyed";
}

void PatternUI::build_ui(QWidget *parent,uint16_t number_of_channels) {}
void PatternUI::post_load_ui() {}
void PatternUI::parse_ui() {}
void PatternUI::destroy_ui() {}

uint32_t ClockPattern::get_min_sampling_freq()
{
	return frequency * std::lcm(duty_cycle_granularity,phase_granularity);
}

uint32_t ClockPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	// greatest common divider duty cycle and 1000;0;
	uint32_t period_number_of_samples = (uint32_t)sample_rate/frequency;
	return period_number_of_samples;
}

float ClockPattern::get_duty_cycle() const
{
	return duty_cycle;
}

void ClockPattern::set_duty_cycle(float value)
{
	if (value>100) {
		value = 100;
	}

	duty_cycle = value;
	auto max = 100;
	duty_cycle_granularity = 100/std::gcd((int)value, max);
}

float ClockPattern::get_frequency() const
{
	return frequency;
}

void ClockPattern::set_frequency(float value)
{
	static const int frequency_precision = 1000;
	frequency = round(value * frequency_precision)/ frequency_precision;
}

int ClockPattern::get_phase() const
{
	return phase;
}

void ClockPattern::set_phase(int value)
{
	phase = value;

	if (phase>360) {
		phase = phase%360;
	}

	if (phase<0) {
		phase = 360-(abs(phase)%360);
	}

	auto max=360;
	phase_granularity=360/std::gcd((int)phase,max);

}

ClockPattern::ClockPattern()
{
	set_name("Clock");
	set_description("Clock pattern");
	set_periodic(true);
	set_frequency(5000);
	set_duty_cycle(50);
	set_phase(0);
}

ClockPattern::~ClockPattern()
{

}


uint8_t ClockPattern::generate_pattern(uint32_t sample_rate,
				       uint32_t number_of_samples, uint16_t number_of_channels)
{
	float f_period_number_of_samples = (float)sample_rate/frequency;
	qDebug()<<"period_number_of_samples - "<<f_period_number_of_samples;
	float f_number_of_periods = number_of_samples / f_period_number_of_samples;
	qDebug()<<"number_of_periods - " << f_number_of_periods;
	float f_low_number_of_samples = (f_period_number_of_samples *
					 (100-duty_cycle)) / 100;
	qDebug()<<"low_number_of_samples - " << f_low_number_of_samples;
	float f_high_number_of_samples = f_period_number_of_samples -
					 f_low_number_of_samples;
	qDebug()<<"high_number_of_samples - " << f_high_number_of_samples;


	int period_number_of_samples = (int)round(f_period_number_of_samples);
	int low_number_of_samples = (int)round(f_low_number_of_samples);

	if (period_number_of_samples==0) {
		period_number_of_samples=1;
	}

	delete_buffer();
	buffer = new short[number_of_samples];
	int i=0;

	// phased samples
	int phased = (period_number_of_samples * phase/360);

	while (i<number_of_samples) {
		if ((i+phased) % ((int)period_number_of_samples) < low_number_of_samples) {
			buffer[i] = 0;
		} else {
			buffer[i] = 0xffff;
		}

		//buffer[i] = (number_of_samples % period_number_of_samples) ;
		i++;
	}

	return 0;
}

ClockPatternUI::ClockPatternUI(ClockPattern *pattern,
			       QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
{
	ui = new Ui::EmptyPatternUI();
	ui->setupUi(this);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	ui->verticalLayout->addWidget(frequencySpinButton);
	phaseSpinButton = new PhaseSpinButton({
		{"deg",1},
		{"π rad",180}
	},tr("Phase"),0,360,true,true,this);
	phaseSpinButton->setFineModeAvailable(false);
	ui->verticalLayout->addWidget(phaseSpinButton);
	dutySpinButton = new PositionSpinButton({{"%",1}},tr("Duty"),0,100,true,false,this);
	dutySpinButton->setFineModeAvailable(false);
	ui->verticalLayout->addWidget(dutySpinButton);
	requestedDuty = 50;
	requestedFrequency = 1000;
	requestedPhase = 0;
	setVisible(false);
}

ClockPatternUI::~ClockPatternUI()
{
	//qDebug()<<"ClockPatternUI destroyed";
	delete ui;
}

Pattern *ClockPatternUI::get_pattern()
{
	return pattern;
}

void ClockPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	requestedFrequency=pattern->get_frequency();
	requestedPhase=pattern->get_phase();
	requestedDuty=pattern->get_duty_cycle();
	frequencySpinButton->setValue(pattern->get_frequency());
	phaseSpinButton->setValue(pattern->get_phase());
	dutySpinButton->setValue(pattern->get_duty_cycle());

	connect(frequencySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));
	connect(phaseSpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));
	connect(dutySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));


}
void ClockPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

void ClockPatternUI::parse_ui()
{
	QObject *obj = sender();

	bool freqStepDown = false;
	bool phaseStepDown = true;
	bool phaseNoChange  = true;
	bool freqChange = false;
	double oldPhase = 0;


	if (obj==frequencySpinButton) {
		if (frequencySpinButton->value() < requestedFrequency) {
			freqStepDown=true;
		}

		requestedFrequency = frequencySpinButton->value();
		freqChange=true;
	}

	if (obj==phaseSpinButton) {
		oldPhase = requestedPhase;
		requestedPhase=phaseSpinButton->value();
		phaseNoChange = false;
		phaseStepDown = phaseSpinButton->isStepDown();
	}

	if (obj==dutySpinButton) {
		requestedDuty=dutySpinButton->value();
	}

	auto freq=requestedFrequency;//frequencySpinButton->value();

	long div;

	if(freqChange)
	{
		if (freqStepDown) {
			div=(long)ceil((double)PG_MAX_SAMPLERATE/freq);
		} else {
			div=(long)floor((double)PG_MAX_SAMPLERATE/freq);
		}

		freq=(PG_MAX_SAMPLERATE)/(float)div;
		requestedFrequency=freq;

		frequencySpinButton->blockSignals(true);
		frequencySpinButton->setValue(freq);
		frequencySpinButton->updateCompletionCircle(freq);
		frequencySpinButton->blockSignals(false);
	}
	pattern->set_frequency(freq);

	auto dutystep=100.0/ (PG_MAX_SAMPLERATE / pattern->get_frequency());
	dutystep=(dutystep>1 ? dutystep : 1);
	dutystep=floor(dutystep*100+0.5)/100.0;
	auto dutyval = requestedDuty;//dutySpinButton->value();
	dutyval=floor((dutyval/dutystep) +0.5)*dutystep;

	dutySpinButton->blockSignals(true);
	dutySpinButton->setStep(dutystep);
	dutySpinButton->setValue(dutyval);
	dutySpinButton->updateCompletionCircle(dutyval);
	dutySpinButton->blockSignals(false);
	pattern->set_duty_cycle(dutySpinButton->value());


	/*auto phaseStep=360.0/ (PG_MAX_SAMPLERATE / pattern->get_frequency());
	phaseStep=(phaseStep>1 ? phaseStep : 1);
	phaseStep=floor(phaseStep*360+0.5)/360.0;
	auto phaseVal = requestedPhase;
	if (phaseNoChange)
	{
		phaseVal = (floor(phaseVal/phaseStep))*phaseStep;
	}
	else
	{
		if (phaseStepDown) {
			phaseVal = floor((phaseVal/phaseStep)-0.5)*phaseStep;
		} else {
			phaseVal = round((phaseVal/phaseStep)+0.5)*phaseStep;
		}

		phaseVal = abs(phaseVal);

		if (phaseVal == oldPhase) {
			if (phaseVal == 0) phaseVal = 360;
			phaseStepDown ? (phaseVal -= phaseStep)
				      : (phaseVal += phaseStep);
		}
	}*/

	auto phaseStep=360.0/ (PG_MAX_SAMPLERATE / pattern->get_frequency());
	phaseStep=(phaseStep>1 ? phaseStep : 1);
	phaseStep=floor(phaseStep*360+0.5)/360.0;
	auto phaseVal = requestedPhase;
	if(fmod(requestedPhase, phaseStep)  != 0)
	{
		if(phaseStep > 45)
		{
			if(phaseStepDown)
				phaseVal = floor((phaseVal/phaseStep)-0.5)*phaseStep;
			else
				phaseVal = ceil((phaseVal/phaseStep)+0.5)*phaseStep;
		}
		else
		{
			phaseVal = floor((phaseVal/phaseStep)+0.5)*phaseStep;
		}

		requestedPhase = phaseVal;
		phaseSpinButton->blockSignals(true);
		phaseSpinButton->setStep(phaseStep);
		phaseSpinButton->setValue(phaseVal);
		if (phaseVal >= 360) phaseVal = (int)phaseVal % 360;
		}
	phaseSpinButton->updateCompletionCircle(phaseVal);
	phaseSpinButton->blockSignals(false);
	pattern->set_phase(phaseSpinButton->value());
	Q_EMIT patternParamsChanged();

}


uint16_t NumberPattern::get_nr() const
{
	return nr;
}

void NumberPattern::set_nr(const uint16_t& value)
{
	nr = value;
}

NumberPattern::NumberPattern() : nr(0)
{
	set_name(NumberPatternName);
	set_description(NumberPatternDescription);
	set_periodic(false);
}
uint8_t NumberPattern::generate_pattern(uint32_t sample_rate,
					uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();
	buffer = new short[number_of_samples];

	for (size_t i=0; i<number_of_samples; i++) {
		buffer[i] = nr;
	}

	return 0;
}


NumberPatternUI::NumberPatternUI(NumberPattern *pattern,
				 QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent), max(0)
{
	qDebug()<<"NumberPatternUI created";
	ui = new Ui::NumberPatternUI();
	ui->setupUi(this);
	setVisible(false);
}

NumberPatternUI::~NumberPatternUI()
{
	qDebug()<<"NumberPatternUI destroyed";
	delete ui;
}

Pattern *NumberPatternUI::get_pattern()
{
	return pattern;
}



void NumberPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	max = (1<<number_of_channels)-1;
	qDebug()<<max;
	//ui->numberLineEdit->setValidator(new QIntValidator(0, max, this));
	ui->numberLineEdit->setText(QString::number(pattern->get_nr()));
	connect(ui->numberLineEdit,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));
}
void NumberPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

void NumberPatternUI::parse_ui()
{
	bool ok =0;

	auto val = ui->numberLineEdit->text().toInt(&ok,10);

	if (!ok) {
		qDebug()<<"Cannot set frequency, not an int";
	}

	max = (1 << pattern->nrOfChannels()) - 1;
	if (val <= max && ok) {
		setDynamicProperty(ui->numberLineEdit, "valid", true);
		setDynamicProperty(ui->numberLineEdit, "invalid", false);
	} else {
		setDynamicProperty(ui->numberLineEdit, "invalid", true);
		setDynamicProperty(ui->numberLineEdit, "valid", false);
	}

	pattern->set_nr(val);

	Q_EMIT patternParamsChanged();
}

RandomPattern::RandomPattern()
{
	set_name(RandomPatternName);
	set_description(RandomPatternDescription);
	set_periodic(false);
	set_frequency(5000);
}

RandomPattern::~RandomPattern()
{
}


uint32_t RandomPattern::get_min_sampling_freq()
{

	return frequency;
}

uint32_t RandomPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	uint32_t period_number_of_samples = (uint32_t)sample_rate/frequency;
	return period_number_of_samples*10;
}

uint32_t RandomPattern::get_frequency() const
{
	return frequency;
}

void RandomPattern::set_frequency(const uint32_t& value)
{
	frequency = value;
}

uint8_t RandomPattern::generate_pattern(uint32_t sample_rate,
					uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();
	buffer = new short[number_of_samples];
	auto samples_per_count = (int)round(((float)sample_rate/(float)frequency));
	size_t j=0;

	while (j<number_of_samples) {
		uint16_t random_value = rand() % (1<<number_of_channels);

		for (auto k=0; k<samples_per_count; k++,j++) {
			if (j>=number_of_samples) {
				break;
			}

			buffer[j] = random_value;
		}

	}

	return 0;
}

RandomPatternUI::RandomPatternUI(RandomPattern *pattern,
				 QWidget *parent): pattern(pattern),parent_(parent)
{
	//qDebug()<<"RandomPatternUI created";
	ui = new Ui::EmptyPatternUI();
	ui->setupUi(this);
	setVisible(false);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	ui->verticalLayout->addWidget(frequencySpinButton);
}
RandomPatternUI::~RandomPatternUI()
{
	//qDebug()<<"NumberPatternUI destroyed";
	delete ui;
}


Pattern *RandomPatternUI::get_pattern()
{
	return pattern;
}


void RandomPatternUI::parse_ui()
{
	pattern->set_frequency(frequencySpinButton->value());
	Q_EMIT patternParamsChanged();
}

void RandomPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	frequencySpinButton->setValue(pattern->get_frequency());
	connect(frequencySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));

}

void RandomPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}



uint32_t BinaryCounterPattern::get_min_sampling_freq()
{
	return frequency;
}

uint32_t BinaryCounterPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	// greatest common divider duty cycle and 1000;0;
	return ((float)sample_rate/(float)frequency) * (1<<number_of_channels);
}


uint32_t BinaryCounterPattern::get_frequency() const
{
	return frequency;
}

void BinaryCounterPattern::set_frequency(const uint32_t& value)
{
	frequency = value;
}

uint16_t BinaryCounterPattern::get_start_value() const
{
	return start_value;
}

void BinaryCounterPattern::set_start_value(const uint16_t& value)
{
	start_value = value;
}

uint16_t BinaryCounterPattern::get_end_value() const
{
	return end_value;
}

void BinaryCounterPattern::set_end_value(const uint16_t& value)
{
	end_value = value;
}

uint16_t BinaryCounterPattern::get_increment() const
{
	return increment;
}

void BinaryCounterPattern::set_increment(const uint16_t& value)
{
	increment = value;
}

uint16_t BinaryCounterPattern::get_init_value() const
{
	return init_value;
}

void BinaryCounterPattern::set_init_value(const uint16_t& value)
{
	init_value = value;
}

BinaryCounterPattern::BinaryCounterPattern()
{
	set_name(BinaryCounterPatternName);
	set_description(BinaryCounterPatternDescription);
	set_periodic(true);
	set_frequency(5000);
	start_value = 0;
	end_value = 1;
	increment = 1;
	init_value = 0;
}

BinaryCounterPattern::~BinaryCounterPattern()
{

}


uint8_t BinaryCounterPattern::generate_pattern(uint32_t sample_rate,
		uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();
	buffer = new short[number_of_samples];
	auto samples_per_count = (int)round(((float)sample_rate/(float)frequency));
	//auto i=init_value;
	auto i = 0;
	auto increment = 1;
	auto start_value = 0;
	auto end_value = (1<<number_of_channels)-1;
	size_t j=0;

	while (j<number_of_samples) {
		for (auto k=0; k<samples_per_count; k++,j++) {
			if (j>=number_of_samples) {
				break;
			}

			buffer[j] = i;
		}

		if (i<end_value) {
			i=i+increment;
		} else {
			i=start_value;
		}
	}

	return 0;
}

BinaryCounterPatternUI::BinaryCounterPatternUI(BinaryCounterPattern *pattern,
		QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
{
	//qDebug()<<"BinaryCounterPatternUI Created";
	ui = new Ui::EmptyPatternUI();
	ui->setupUi(this);
	setVisible(false);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	ui->verticalLayout->addWidget(frequencySpinButton);

	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	for (const GSList *sl = decoderList; sl; sl = sl->next) {
	    srd_decoder *dec = (struct srd_decoder *)sl->data;
	    if (QString::fromUtf8(dec->id) == "parallel") {
		m_decoder = std::make_shared<adiscope::logic::Decoder>(dec);
	    }
	}

	g_slist_free(decoderList);

	connect(this, &BinaryCounterPatternUI::patternParamsChanged, [=](){
//		m_decoder->set_option();
		qDebug() << "Update decoder params parallel!";
	});
}

BinaryCounterPatternUI::~BinaryCounterPatternUI()
{
	qDebug()<<"BinaryCounterPatternUI Destroyed";
	delete ui;
}

void BinaryCounterPatternUI::build_ui(QWidget *parent,
				      uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	frequencySpinButton->setValue(pattern->get_frequency());
	connect(frequencySpinButton,SIGNAL(valueChanged(double)), this,
		SLOT(parse_ui()));
}

void BinaryCounterPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

GenericLogicPlotCurve *BinaryCounterPatternUI::getAnnotationCurve()
{
	return m_annotationCurve;
}

std::shared_ptr<adiscope::logic::Decoder> BinaryCounterPatternUI::getDecoder()
{
	return m_decoder;
}

void BinaryCounterPatternUI::setAnnotationCurve(GenericLogicPlotCurve *curve)
{
	m_annotationCurve = curve;
}

QVector<int> BinaryCounterPatternUI::getChannelsToAssign()
{
	// we don't want to assign the first channel 0 (clock) of the
	// parallel decoder. Only the data lines.
	return {1, 2, 3, 4, 5, 6, 7, 8};
}

Pattern *BinaryCounterPatternUI::get_pattern()
{
	return pattern;
}


void BinaryCounterPatternUI::parse_ui()
{
	/*bool ok = false;*/
	pattern->set_frequency(frequencySpinButton->value());



	/*pattern->set_init_value(ui->inittval_LE->text().toULong(&ok));
	if(!ok) qDebug()<<"Cannot set_init_value, not a uint32";

	pattern->set_start_value(ui->startEdit->text().toUInt(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
	pattern->set_end_value(ui->endEdit->text().toUInt(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
	pattern->set_increment(ui->incrementEdit->text().toUInt(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a uint16";*/
	Q_EMIT patternParamsChanged();
}

GrayCounterPattern::GrayCounterPattern()
{
	set_name(GrayCounterPatternName);
	set_description(GrayCounterPatternDescription);
	set_periodic(true);
}

uint8_t GrayCounterPattern::generate_pattern(uint32_t sample_rate,
		uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();
	buffer = new short[number_of_samples];
	auto samples_per_count = (int)round(((float)sample_rate/(float)frequency));
	init_value = 0;
	end_value =(1<< (number_of_channels))-1;
	increment = 1;
	start_value = 0;
	auto i=init_value;
	size_t j=0;

	while (j<number_of_samples) {
		for (auto k=0; k<samples_per_count; k++,j++) {
			if (j>=number_of_samples) {
				break;
			}

			buffer[j] = i ^ (i >> 1);
		}

		if (i<end_value) {
			i=i+increment;
		} else {
			i=start_value;
		}
	}

	return 0;
}

GrayCounterPatternUI::GrayCounterPatternUI(GrayCounterPattern *pattern,
		QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
{
	ui = new Ui::EmptyPatternUI();
	ui->setupUi(this);
	setVisible(false);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	ui->verticalLayout->addWidget(frequencySpinButton);
}
GrayCounterPatternUI::~GrayCounterPatternUI()
{
	qDebug()<<"BinaryCounterPatternUI Destroyed";
	delete ui;
}


Pattern *GrayCounterPatternUI::get_pattern()
{
	return pattern;
}

void GrayCounterPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	frequencySpinButton->setValue(pattern->get_frequency());
	connect(frequencySpinButton,SIGNAL(valueChanged(double)), this,
		SLOT(parse_ui()));

}
void GrayCounterPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
	//delete ui;
}

void GrayCounterPatternUI::parse_ui()
{
	/*bool ok = false;*/
	pattern->set_frequency(frequencySpinButton->value());



	/*pattern->set_init_value(ui->inittval_LE->text().toULong(&ok));
	if(!ok) qDebug()<<"Cannot set_init_value, not a uint32";

	pattern->set_start_value(ui->startEdit->text().toUInt(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
	pattern->set_end_value(ui->endEdit->text().toUInt(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a uint16";
	pattern->set_increment(ui->incrementEdit->text().toUInt(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a uint16";*/
	Q_EMIT patternParamsChanged();
}


UARTPattern::UARTPattern()
{
	parity = SP_PARITY_NONE;
	stop_bits = 1;
	baud_rate = 9600;
	data_bits = 8;
	msb_first=false;
	set_periodic(false);
	set_name(UARTPatternName);
	set_description(UARTPatternDescription);

}


unsigned int UARTPattern::get_baud_rate()
{
	return baud_rate;
}

unsigned int UARTPattern::get_data_bits()
{
	return data_bits;
}

unsigned int UARTPattern::get_stop_bits()
{
	return stop_bits;
}

enum UARTPattern::sp_parity UARTPattern::get_parity()
{
	return parity;
}

void UARTPattern::set_string(const std::string &str_)
{
	str = str_;
}

std::string UARTPattern::get_string()
{
	return str;
}

std::string UARTPattern::get_params()
{
	return params;
}

int UARTPattern::set_params(std::string params_)
{
	// https://github.com/analogdevicesinc/libiio/blob/master/serial.c#L426
	params = params_;
	const char *params = params_.c_str();
	char *end;

	baud_rate = strtoul(params, &end, 10);

	if (params == end) {
		return -EINVAL;
	}

	auto uart_format = strchr(params,'/');

	if (uart_format == NULL) { /* / not found, use default settings*/
		data_bits = 8;
		parity = SP_PARITY_NONE;
		stop_bits = 1;
		return 0;
	}

	uart_format++;

	data_bits = strtoul(uart_format, &end, 10);

	if (params == end) {
		return -EINVAL;
	}

	char lowercase_parity = tolower(*end);

	switch (lowercase_parity) {
	case 'n':
		parity = SP_PARITY_NONE;
		break;

	case 'o':
		parity = SP_PARITY_ODD;
		break;

	case 'e':
		parity = SP_PARITY_EVEN;
		break;

	case 'm':
		parity = SP_PARITY_MARK;
		break;

	case 's':
		parity = SP_PARITY_SPACE;
		break;

	default:
		return -EINVAL;
	}

	end++;
	uart_format = end;
	stop_bits = strtoul(uart_format, &end, 10);

	if (params == end) {
		return -EINVAL;
	}

	return 0;
}

void UARTPattern::set_msb_first(bool msb_first_)
{
	msb_first = msb_first_;
}

uint16_t UARTPattern::encapsulateUartFrame(char chr, uint16_t *bits_per_frame)
{
	uint16_t ret = 0xffff;
	bool parity_bit_available = false;
	uint16_t parity_bit_value = 1;
	auto chr_to_test = chr;

	switch (parity) {
	case SP_PARITY_NONE:
		parity_bit_available = false;
		break;

	case SP_PARITY_ODD:
		parity_bit_value = 1;
		parity_bit_available = true;

		for (size_t i=0; i<data_bits; i++) {
			parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
			chr_to_test = chr_to_test>>1;
		}

		break;

	case SP_PARITY_EVEN:
		parity_bit_value = 0;
		parity_bit_available = true;

		for (size_t i=0; i<data_bits; i++) {
			parity_bit_value = parity_bit_value ^ (chr_to_test & 0x01);
			chr_to_test = chr_to_test>>1;
		}

		break;

	case SP_PARITY_MARK:
		parity_bit_available = true;
		parity_bit_value = 1;
		break;

	case SP_PARITY_SPACE:
		parity_bit_available = true;
		parity_bit_value = 0;
		break;

	case SP_PARITY_INVALID:
		qDebug() << "Invalid parity setting detected";
	}

	if (!msb_first) {
		ret = chr;
		ret = ret << 1; // start bit
		uint16_t stop_bit_values;

		/*      if(parity_bit_available)
		{
		    stop_bit_values = ((1<<stop_bits+1)-1) & parity_bit_value; // parity bit value is cleared
		}
		else
		    stop_bit_values = ((1<<stop_bits)-1); // if parity bit not availabe, stop bits will not be incremented
		*/
		stop_bit_values = ((1<<(stop_bits+parity_bit_available))-1) & ((
					  parity_bit_value) ? (0xffff) : (0xfffe)); // todo: Simplify this
		ret = ret | stop_bit_values << (data_bits+1);


	} else {
		ret = (~(1<<data_bits)); // start bit
		ret = ret & chr;

		if (parity_bit_available) {
			ret = (ret << 1) | parity_bit_value;
		}

		for (size_t i=0; i<stop_bits; i++) {
			ret = (ret << 1) | 0x01;
		}
	}

	(*bits_per_frame) = data_bits + 1 + stop_bits + parity_bit_available;

	return ret;

}

uint32_t UARTPattern::get_min_sampling_freq()
{
	return baud_rate*2;
}

uint32_t UARTPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	uint16_t number_of_frames = str.length();
	uint32_t samples_per_bit = sample_rate/baud_rate;
	uint16_t bits_per_frame;
	encapsulateUartFrame(*(str.c_str()), &bits_per_frame);
	uint32_t samples_per_frame = samples_per_bit * bits_per_frame;
	return samples_per_frame*(number_of_frames + 1/* padding */);
}

uint8_t UARTPattern::generate_pattern(uint32_t sample_rate,
				      uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();
	uint32_t samples_per_bit = sample_rate/baud_rate;
	qDebug()<< "samples_per_bit - "<<(float)sample_rate/(float)baud_rate;
	uint16_t bits_per_frame;
	encapsulateUartFrame(*(str.c_str()), &bits_per_frame);
	uint32_t samples_per_frame = samples_per_bit * bits_per_frame;

	buffer = new short[number_of_samples]; // no need to recreate buffer
	memset(buffer, 0xff, (number_of_samples)*sizeof(short));

	short *buf_ptr = buffer;
	short *buf_ptr_end = buffer + number_of_samples;
	const char *str_ptr = str.c_str();
	size_t i;

	for (i=0; i<samples_per_frame/2 && buf_ptr < buf_ptr_end; i++, buf_ptr++) { // pad with half a frame
		*buf_ptr = 1;
	}

	for (i=0; i<str.length(); i++,str_ptr++) {
		auto frame_to_send = encapsulateUartFrame(*str_ptr, &bits_per_frame);

		for (size_t j=0; j<bits_per_frame; j++) {
			short bit_to_send;

			if (!msb_first) {
				bit_to_send = (frame_to_send & 0x01);
				frame_to_send = frame_to_send >> 1;
			} else {
				bit_to_send = ((frame_to_send & (1<<(bits_per_frame-1))) ? 1 :
					       0);  // set bit here
				frame_to_send = frame_to_send << 1;
			}

			for (size_t k=0; k<samples_per_bit && buf_ptr < buf_ptr_end; k++,buf_ptr++) {
				*buf_ptr =  bit_to_send;// set bit here
			}
		}
	}

	for (i=0; i<samples_per_frame/2 && buf_ptr < buf_ptr_end; i++, buf_ptr++) { // pad with half a frame
		*buf_ptr = 1;
	}

	for (; buf_ptr!=(short *)(buffer+number_of_samples) && buf_ptr < buf_ptr_end; buf_ptr++) {
		*buf_ptr = 1;
	}

	return 0;
}


UARTPatternUI::UARTPatternUI(UARTPattern *pattern,
			     QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
{
	qDebug()<<"UARTPatternUI created";
	ui = new Ui::UARTPatternUI();
	ui->setupUi(this);
	setVisible(false);

	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	for (const GSList *sl = decoderList; sl; sl = sl->next) {
	    srd_decoder *dec = (struct srd_decoder *)sl->data;
	    if (QString::fromUtf8(dec->id) == "uart") {
		m_decoder = std::make_shared<adiscope::logic::Decoder>(dec);
	    }
	}

	g_slist_free(decoderList);

	connect(this, &UARTPatternUI::patternParamsChanged, [=](){
//		m_decoder->set_option();
		qDebug() << "Update decoder params uart!";

		m_decoder->set_option("baudrate",
				      g_variant_new_int64(pattern->get_baud_rate()));
		m_decoder->set_option("data_bits",
				      g_variant_new_int64(8));

		auto par = pattern->get_parity();

		GVariant *parityStr = g_variant_new_string("none");

		switch (par) {

		case UARTPattern::SP_PARITY_ODD:
			parityStr = g_variant_new_string("odd");
			break;

		case UARTPattern::SP_PARITY_EVEN:
			parityStr = g_variant_new_string("even");
			break;

		case UARTPattern::SP_PARITY_MARK:
			parityStr = g_variant_new_string("mark");
			break;

		case UARTPattern::SP_PARITY_SPACE:
			parityStr = g_variant_new_string("space");
			break;

		case UARTPattern::SP_PARITY_NONE:
		default:
			parityStr = g_variant_new_string("none");
			break;

		}

		m_decoder->set_option("parity", parityStr);
		m_decoder->set_option("stop_bits",
				      g_variant_new_double(pattern->get_stop_bits()));
		m_decoder->set_option("format", g_variant_new_string("ascii"));

		dynamic_cast<AnnotationCurve*>(m_annotationCurve)->getAnnotationDecoder()->startDecode();
	});
}

UARTPatternUI::~UARTPatternUI()
{
	qDebug()<<"UARTPatternUI destroyed";
	delete ui;
}

void UARTPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	ui->CB_baud->setCurrentText(QString::number(pattern->get_baud_rate()));
	ui->CB_Parity->setCurrentIndex(pattern->get_parity());
	ui->CB_Stop->setCurrentText(QString::number(pattern->get_stop_bits()));
	ui->LE_Data->setText(QString::fromStdString(pattern->get_string()));
	connect(ui->CB_baud,SIGNAL(activated(QString)),this,SLOT(parse_ui()));
	connect(ui->CB_Parity,SIGNAL(activated(QString)),this,SLOT(parse_ui()));
	connect(ui->CB_Stop,SIGNAL(activated(QString)),this,SLOT(parse_ui()));
	connect(ui->LE_Data,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));
	parse_ui();


}
void UARTPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
	//    delete ui;
}

GenericLogicPlotCurve *UARTPatternUI::getAnnotationCurve()
{
	return m_annotationCurve;
}

std::shared_ptr<adiscope::logic::Decoder> UARTPatternUI::getDecoder()
{
	return m_decoder;
}

void UARTPatternUI::setAnnotationCurve(GenericLogicPlotCurve *curve)
{
	m_annotationCurve = curve;
}

Pattern *UARTPatternUI::get_pattern()
{
	return pattern;
}


void UARTPatternUI::parse_ui()
{

	auto oldStr = ui->LE_paramsOut->text();
	auto newStr = ui->CB_baud->currentText() + "/8"
		      +ui->CB_Parity->currentText()[0] + ui->CB_Stop->currentText();
	ui->LE_paramsOut->setText(newStr);
	qDebug()<<ui->LE_paramsOut->text();
	pattern->set_params(ui->LE_paramsOut->text().toStdString());
	qDebug()<<ui->LE_Data->text();
	pattern->set_string(ui->LE_Data->text().toStdString());

	Q_EMIT patternParamsChanged();

	if (oldStr != newStr) {
		Q_EMIT decoderChanged();
	}

}



uint8_t I2CPattern::getAddress() const
{
	return address;
}

void I2CPattern::setAddress(const uint8_t& value)
{
	address = value;
}

bool I2CPattern::getWrite() const
{
	return read;
}

void I2CPattern::setWrite(bool value)
{
	read = value;
}

bool I2CPattern::getMsbFirst() const
{
	return msbFirst;
}

void I2CPattern::setMsbFirst(bool value)
{
	msbFirst = value;
}

uint8_t I2CPattern::getInterFrameSpace() const
{
	return interFrameSpace;
}

void I2CPattern::setInterFrameSpace(const uint8_t& value)
{
	interFrameSpace = value;
}

uint32_t I2CPattern::getClkFrequency() const
{
	return clkFrequency;
}

void I2CPattern::setClkFrequency(const uint32_t& value)
{
	clkFrequency = value;
}

uint8_t I2CPattern::getBytesPerFrame() const
{
	return bytesPerFrame;
}

void I2CPattern::setBytesPerFrame(const uint8_t& value)
{
	bytesPerFrame = value;
}

bool I2CPattern::getTenbit() const
{
	return tenbit;
}

void I2CPattern::setTenbit(bool value)
{
	tenbit = value;
}

I2CPattern::I2CPattern()
{
	clkFrequency=5000;
	msbFirst=true;
	address=0x72;
	samples_per_bit=1;
	buf_ptr=nullptr;
	interFrameSpace=3;
	bytesPerFrame=2;
	read=false;
	tenbit=false;
	set_periodic(false);
	set_name(I2CPatternName);
	set_description(I2CPatternDescription);
}

uint32_t I2CPattern::get_min_sampling_freq()
{
	return clkFrequency * (4);
}

uint32_t I2CPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	auto samples_per_bit = 1*(sample_rate/clkFrequency);
	auto IFS=interFrameSpace*samples_per_bit;

	// size = samples/bit * (IFS+start(2), address(7), ack(1), (data(8) + ack(1))*data_len, stop(2)+IFS)
	uint32_t samples=samples_per_bit * (interFrameSpace+2+7+1+(8+1)*v.size()+2+interFrameSpace);
	return samples;
}


void I2CPattern::sample_bit(bool bit)
{
	// SDA Transitions must occur when SCL is LOW (unless starting or stopping)
	// Set SDA whilst SCL is low from previous bit
	for (size_t i=0; i<samples_per_bit/4 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,bit);
		*buf_ptr = changeBit(*buf_ptr,SCL,0);
	}

	// Sample SDA by clocking SCL
	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,bit);
		*buf_ptr = changeBit(*buf_ptr,SCL,1);
	}

	// Leave SCL low for next bit
	for (size_t i=0; i<samples_per_bit/4 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,bit);
		*buf_ptr = changeBit(*buf_ptr,SCL,0);
	}
}

void I2CPattern::sample_start_bit()
{
	// Explicitly set start condition, consume 2 bits
	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,1);
		*buf_ptr = changeBit(*buf_ptr,SCL,1);
	}

	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,0);
		*buf_ptr = changeBit(*buf_ptr,SCL,1);
	}

	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,0);
		*buf_ptr = changeBit(*buf_ptr,SCL,0);
	}

	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,0);
		*buf_ptr = changeBit(*buf_ptr,SCL,0);
	}
}

void I2CPattern::sample_address()
{
	auto tmpaddress = address;

	for (auto i=0; i<7; i++) {
		sample_bit((tmpaddress&0x40)>>6);
		tmpaddress<<=1;
	}

	sample_bit(read);

}

void I2CPattern::sample_send_ack()
{
	sample_bit(0);
}

void I2CPattern::sample_await_ack()
{
	sample_bit(1);
}

void I2CPattern::sample_payload()
{
	for (std::deque<uint8_t>::iterator it = v.begin(); it != v.end();
	     ++it) {
		uint8_t val;

		if (read) {
			val = 0xff;
		} else {
			val = *it;
		}

		for (auto j=0; j<8; j++) {
			bool bit;

			if (msbFirst) {
				bit = (val & 0x80) >> 7;
				val = val << 1;
			} else {
				bit = (val & 0x01);
				val = val >> 1;
			}

			sample_bit(bit);
		}

		if (read) {
			// Last bit on a read should be NACKed
			if (it == v.end()-1)	{
				sample_bit(1);
			} else {
				sample_send_ack();
			}
		} else {
			sample_await_ack();
		}
	}
}

void I2CPattern::sample_stop()
{
	// Explicitly set stop condition, consume 2 bits
	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,0);
		*buf_ptr = changeBit(*buf_ptr,SCL,0);
	}

	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,0);
		*buf_ptr = changeBit(*buf_ptr,SCL,1);
	}

	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,1);
		*buf_ptr = changeBit(*buf_ptr,SCL,1);
	}

	for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
		*buf_ptr = changeBit(*buf_ptr,SDA,1);
		*buf_ptr = changeBit(*buf_ptr,SCL,1);
	}
}

uint8_t I2CPattern::generate_pattern(uint32_t sample_rate,
				     uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();

	buffer = new short[number_of_samples]; // no need to recreate buffer
	buf_ptr = buffer;
	buf_ptr_end = buffer + number_of_samples;
	memset(buffer, (0xff), (number_of_samples)*sizeof(short));

	samples_per_bit = 1*(sample_rate/clkFrequency);
	buf_ptr+=interFrameSpace*samples_per_bit;


	sample_start_bit();
	sample_address();
	sample_await_ack();
	sample_payload();
	sample_stop();
	return 0;
}



I2CPatternUI::I2CPatternUI(I2CPattern *pattern,
			   QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
      , m_annotationCurve(nullptr)
      , m_decoder(nullptr)
{
	qDebug()<<"UARTPatternUI created";
	ui = new Ui::I2CPatternUI();
	ui->setupUi(this);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	ui->verticalLayout->insertWidget(0,frequencySpinButton);
	setVisible(false);

	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	for (const GSList *sl = decoderList; sl; sl = sl->next) {
	    srd_decoder *dec = (struct srd_decoder *)sl->data;
	    if (QString::fromUtf8(dec->id) == "i2c") {
		m_decoder = std::make_shared<adiscope::logic::Decoder>(dec);
	    }
	}

	g_slist_free(decoderList);

	connect(this, &I2CPatternUI::patternParamsChanged, [=](){
//		m_decoder->set_option();
		qDebug() << "Update decoder params!";
	});
}

I2CPatternUI::~I2CPatternUI()
{
	qDebug()<<"UARTPatternUI destroyed";
	delete ui;
}

void I2CPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	frequencySpinButton->setValue(pattern->getClkFrequency());
	ui->PB_MSB->setChecked(pattern->getMsbFirst());
	ui->LE_IFS->setText(QString::number(pattern->getInterFrameSpace()));
	ui->LE_address->setText(QString::number(pattern->getAddress(),16));
	ui->PB_readWrite->setChecked(pattern->getWrite());
	ui->PB_MSB->setVisible(false);
	ui->label_8->setVisible(false);

	//ui->LE_BPF->setText(QString::number(pattern->getBytesPerFrame()));
	QString buf;

	for (auto val:pattern->v) {
		buf.append(QString::number(val,16));
		buf.append(" ");
	}

	ui->LE_toSend->setText(buf);

	//ui->LE_toSend->setText(QString::fromStdString(pattern->get_string()));
	connect(frequencySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));
	connect(ui->PB_MSB,SIGNAL(clicked()),this,SLOT(parse_ui()));
	connect(ui->PB_readWrite,SIGNAL(clicked()),this,SLOT(parse_ui()));
	connect(ui->LE_IFS,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));
	connect(ui->LE_address,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));
	connect(ui->LE_toSend,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));


	parse_ui();


}

void I2CPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
	//    delete ui;
}

GenericLogicPlotCurve *I2CPatternUI::getAnnotationCurve()
{
	return m_annotationCurve;
}

std::shared_ptr<adiscope::logic::Decoder> I2CPatternUI::getDecoder()
{
	return m_decoder;
}

void I2CPatternUI::setAnnotationCurve(GenericLogicPlotCurve *curve)
{
	m_annotationCurve = curve;
}

Pattern *I2CPatternUI::get_pattern()
{
	return pattern;
}


void I2CPatternUI::parse_ui()
{
	bool ok;
	pattern->setClkFrequency(frequencySpinButton->value());

	auto address = ui->LE_address->text().toInt(&ok,16);

	if (ok && address>=0 && address<=0x7f) {
		pattern->setAddress(address);
	}

	auto IFS = ui->LE_IFS->text().toInt(&ok);

	if (ok && IFS>0) {
		pattern->setInterFrameSpace(IFS);
	}

	pattern->setMsbFirst(ui->PB_MSB->isChecked());
	pattern->setWrite(ui->PB_readWrite->isChecked());
	QStringList strList = ui->LE_toSend->text().split(' ',Qt::SkipEmptyParts);
	pattern->v.clear();

	bool fail = false;

	std::vector<uint8_t> b;
	std::reverse(strList.begin(),strList.end());

	for (const QString &str: qAsConst(strList)) {
		uint64_t val;
		bool ok;
		b.clear();
		val = str.toULongLong(&ok,16);

		if (ok) {
			do {
				auto u8val = val & 0xff;
				val = val >> 8;
				b.push_back(u8val);
			} while (val);

			for (auto u8val : b) {
				pattern->v.push_front(u8val);
			}

		} else {
			fail = true;
			/* add str*/
		}
	}

	if (fail) {
		setDynamicProperty(ui->LE_toSend, "invalid", true);
		setDynamicProperty(ui->LE_toSend, "valid", false);
	} else {
		setDynamicProperty(ui->LE_toSend, "valid", true);
		setDynamicProperty(ui->LE_toSend, "invalid", false);
	}


	Q_EMIT patternParamsChanged();

}



bool SPIPattern::getMsbFirst() const
{
	return msbFirst;
}

void SPIPattern::setMsbFirst(bool value)
{
	msbFirst = value;
}

SPIPattern::SPIPattern()
{
	CPOL=false;
	CPHA = false;
	CSPOL = false;
	clkFrequency=5000;
	bytesPerFrame=2;
	waitClocks=3;
	msbFirst = true;

	set_periodic(false);
	set_name(SPIPatternName);
	set_description(SPIPatternDescription);
}

uint32_t SPIPattern::get_min_sampling_freq()
{
	return clkFrequency * (2);
}

uint32_t SPIPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{

	auto samples_per_bit = 2*(sample_rate/clkFrequency);
	auto IFS=waitClocks*samples_per_bit;

	// number of bytes * samples per bit * 9 (8 bits + 1 account for extra padding) +
	// 2 * interframespace(beginning and end) + extra frame space between each frame
	return (v.size()*samples_per_bit*9) + 2*IFS + IFS *(v.size()/bytesPerFrame);

}


uint8_t SPIPattern::getWaitClocks() const
{
	return waitClocks;
}

void SPIPattern::setWaitClocks(const uint8_t& value)
{
	waitClocks = value;
}

uint8_t SPIPattern::getBytesPerFrame() const
{
	return bytesPerFrame;
}

void SPIPattern::setBytesPerFrame(const uint8_t& value)
{
	bytesPerFrame = value;
}

uint8_t SPIPattern::generate_pattern(uint32_t sample_rate,
				     uint32_t number_of_samples, uint16_t number_of_channels)
{
	delete_buffer();

	buffer = new short[number_of_samples]; // no need to recreate buffer

	auto clkActiveBit = 0;
	auto outputBit = 1;
	auto csBit = 2;

	if (CSPOL) {
		memset(buffer, (CPOL) ? 0xfffb : 0xfffa, (number_of_samples)*sizeof(short));
	} else {
		memset(buffer, (CPOL) ? 0xffff : 0xfffe, (number_of_samples)*sizeof(short));
	}

	short *buf_ptr = buffer;
	short *buf_ptr_end = buffer + number_of_samples;

	auto samples_per_bit = 2 * (sample_rate/clkFrequency);
	buf_ptr += waitClocks * samples_per_bit;
	auto frameBytesLeft = bytesPerFrame;
	bool start_new_frame = 1;

	for (std::deque<uint8_t>::iterator it = v.begin(); it != v.end();
	     ++it) {
		uint8_t val = *it;
		bool oldbit = 0;
		bool bit;

		if(CPHA && start_new_frame)
		{
			for (auto i=samples_per_bit/2; i<samples_per_bit && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
				*buf_ptr = changeBit(*buf_ptr,csBit,CSPOL);
				*buf_ptr = changeBit(*buf_ptr,clkActiveBit,!CPOL);
				*buf_ptr = changeBit(*buf_ptr,outputBit,oldbit);
			}
		}

		for (auto j=0; j<8; j++) {

			if (msbFirst) {
				bit = (val & 0x80) >> 7;
				val = val << 1;
			} else {
				bit = (val & 0x01);
				val = val >> 1;
			}

			for (size_t i=0; i<samples_per_bit/2 && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
				*buf_ptr = changeBit(*buf_ptr,csBit,CSPOL);
				*buf_ptr = changeBit(*buf_ptr,clkActiveBit,CPOL);

				if (!CPHA) {
					*buf_ptr = changeBit(*buf_ptr,outputBit,oldbit);
				} else {
					*buf_ptr = changeBit(*buf_ptr,outputBit,bit);
				}
			}

			for (auto i=samples_per_bit/2; i<samples_per_bit && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
				*buf_ptr = changeBit(*buf_ptr,csBit,CSPOL);
				*buf_ptr = changeBit(*buf_ptr,clkActiveBit,!CPOL);
				*buf_ptr = changeBit(*buf_ptr,outputBit,bit);
			}

			oldbit = bit;
		}

		frameBytesLeft--;

		if (frameBytesLeft == 0) {
			if(!CPHA)
			{
				for (auto i=samples_per_bit/2; i<samples_per_bit && buf_ptr < buf_ptr_end; i++,buf_ptr++) {
					*buf_ptr = changeBit(*buf_ptr,csBit,CSPOL);
					*buf_ptr = changeBit(*buf_ptr,clkActiveBit,!CPOL);
					*buf_ptr = changeBit(*buf_ptr,outputBit,bit);
				}
			}
			buf_ptr += waitClocks * samples_per_bit;
			frameBytesLeft = bytesPerFrame;
			start_new_frame = 1;
		}
		else
		{
			start_new_frame = 0;
		}
	}

	return 0;
}


bool SPIPattern::getCPOL() const
{
	return CPOL;
}

void SPIPattern::setCPOL(bool value)
{
	CPOL = value;
}

bool SPIPattern::getCPHA() const
{
	return CPHA;
}

void SPIPattern::setCPHA(bool value)
{
	CPHA = value;
}

uint32_t SPIPattern::getClkFrequency() const
{
	return clkFrequency;
}

void SPIPattern::setClkFrequency(const uint32_t& value)
{
	clkFrequency = value;
}

bool SPIPattern::getCSPol() const
{
	return CSPOL;
}

void SPIPattern::setCSPol(bool value)
{
	CSPOL = value;
}



SPIPatternUI::SPIPatternUI(SPIPattern *pattern,
			   QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
{
	qDebug()<<"UARTPatternUI created";
	ui = new Ui::SPIPatternUI();
	ui->setupUi(this);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	ui->verticalLayout->insertWidget(0,frequencySpinButton);
	setVisible(false);

	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	for (const GSList *sl = decoderList; sl; sl = sl->next) {
	    srd_decoder *dec = (struct srd_decoder *)sl->data;
	    if (QString::fromUtf8(dec->id) == "spi") {
		m_decoder = std::make_shared<adiscope::logic::Decoder>(dec);
	    }
	}

	g_slist_free(decoderList);


	connect(this, &SPIPatternUI::patternParamsChanged, [=](){
//		m_decoder->set_option();
		qDebug() << "Spi decoder params!";

		GVariant *cspolstr, *bitorderstr;

		if (pattern->getCSPol()) {
			cspolstr = g_variant_new_string("active-high");
		} else {
			cspolstr = g_variant_new_string("active-low");
		}

		if (pattern->getMsbFirst()) {
			bitorderstr = g_variant_new_string("msb-first");
		} else {
			bitorderstr = g_variant_new_string("lsb-first");
		}

		m_decoder->set_option("cs_polarity", cspolstr);
		m_decoder->set_option("bitorder", bitorderstr);
		m_decoder->set_option("cpol", g_variant_new_int64(pattern->getCPOL()));
		m_decoder->set_option("cpha", g_variant_new_int64(pattern->getCPHA()));
	});

}

SPIPatternUI::~SPIPatternUI()
{
	qDebug()<<"UARTPatternUI destroyed";
	delete ui;
}

void SPIPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	frequencySpinButton->setValue(pattern->getClkFrequency());
	ui->PB_CPHA->setChecked(pattern->getCPHA());
	ui->PB_CPOL->setChecked(pattern->getCPOL());
	ui->PB_CS->setChecked(pattern->getCSPol());
	ui->PB_MSB->setChecked(pattern->getMsbFirst());
	ui->LE_IFS->setText(QString::number(pattern->getWaitClocks()));
	ui->LE_BPF->setText(QString::number(pattern->getBytesPerFrame()));
	QString buf;

	for (auto val:pattern->v) {
		buf.append(QString::number(val,16));
		buf.append(" ");
	}

	ui->LE_toSend->setText(buf);

	//ui->LE_toSend->setText(QString::fromStdString(pattern->get_string()));
	connect(frequencySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));

	connect(ui->PB_CPHA,SIGNAL(clicked()),this,SLOT(parse_ui()));
	connect(ui->PB_CPOL,SIGNAL(clicked()),this,SLOT(parse_ui()));
	connect(ui->PB_CS,SIGNAL(clicked()),this,SLOT(parse_ui()));
	connect(ui->PB_MSB,SIGNAL(clicked()),this,SLOT(parse_ui()));

	connect(ui->LE_BPF,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));
	connect(ui->LE_IFS,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));


	connect(ui->LE_toSend,SIGNAL(textChanged(QString)),this,SLOT(parse_ui()));
	parse_ui();


}
void SPIPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
	//    delete ui;
}

GenericLogicPlotCurve *SPIPatternUI::getAnnotationCurve()
{
	return m_annotationCurve;
}

std::shared_ptr<adiscope::logic::Decoder> SPIPatternUI::getDecoder()
{
	return m_decoder;
}

void SPIPatternUI::setAnnotationCurve(GenericLogicPlotCurve *curve)
{
	m_annotationCurve = curve;
}

QVector<int> SPIPatternUI::getChannelsToAssign()
{
	// For the SPI decoder we only want to assign channels
	return {0, 2, 3};
}

Pattern *SPIPatternUI::get_pattern()
{
	return pattern;
}


void SPIPatternUI::parse_ui()
{
	bool ok;
	pattern->setClkFrequency(frequencySpinButton->value());

	auto BPF = ui->LE_BPF->text().toInt(&ok);

	auto oldbpf = pattern->getBytesPerFrame();
	auto oldcpha = pattern->getCPHA();
	auto oldcpol = pattern->getCPOL();
	auto oldcspol = pattern->getCSPol();
	auto oldmsb = pattern->getMsbFirst();

	if (ok && BPF>0) {
		pattern->setBytesPerFrame(ui->LE_BPF->text().toInt());
	}

	auto IFS = ui->LE_IFS->text().toInt(&ok);

	if (ok && IFS>0) {
		pattern->setWaitClocks(IFS);
	}



	pattern->setCPHA(ui->PB_CPHA->isChecked());
	pattern->setCPOL(ui->PB_CPOL->isChecked());
	pattern->setCSPol(ui->PB_CS->isChecked());
	pattern->setMsbFirst(ui->PB_MSB->isChecked());
	QStringList strList = ui->LE_toSend->text().split(' ',Qt::SkipEmptyParts);
	pattern->v.clear();
	bool fail = false;

	std::vector<uint8_t> b;
	std::reverse(strList.begin(),strList.end());

	for (const QString &str: qAsConst(strList)) {
		uint64_t val;
		bool ok;
		b.clear();
		val = str.toULongLong(&ok,16);

		if (ok) {
			do {
				auto u8val = val & 0xff;
				val = val >> 8;
				b.push_back(u8val);
			} while (val);

			for (auto u8val : b) {
				pattern->v.push_front(u8val);
			}

		} else {
			fail = true;
			/* add str*/
		}
	}

	if (fail) {
		setDynamicProperty(ui->LE_toSend, "invalid", true);
		setDynamicProperty(ui->LE_toSend, "valid", false);
	} else {
		setDynamicProperty(ui->LE_toSend, "valid", true);
		setDynamicProperty(ui->LE_toSend, "invalid", false);
	}


	Q_EMIT patternParamsChanged();

	if (oldbpf!=pattern->getBytesPerFrame() || oldcpha!=pattern->getCPHA()
	    || oldcpol!=pattern->getCPOL() || oldcspol != pattern->getCSPol()
	    || oldmsb!=pattern->getMsbFirst()) {
		Q_EMIT decoderChanged();
	}
}


JSPattern::JSPattern(QJsonObject obj_) : obj(obj_)
{
	qDebug()<<"JSPattern created";
	set_name(obj["name"].toString().toStdString());
	console = new JSConsole();
	qEngine = nullptr;
	sample_rate = 1;
	number_of_samples = 1;
	number_of_channels = 1;
	ui_form = nullptr;
}

void JSPattern::init()
{

	if (qEngine!=nullptr) {
		qEngine->collectGarbage();
		delete qEngine;
	}

	qEngine = new QJSEngine();
}

void JSPattern::deinit()
{
	if (qEngine!=nullptr) {
		qEngine->collectGarbage();
		delete qEngine;
		qEngine = nullptr;
	}
}

uint32_t JSPattern::get_min_sampling_freq()
{
	QJSValue result = qEngine->evaluate("get_min_sampling_freq()");

	if (result.isNumber()) {
		return result.toUInt();
	} else if (result.isString()) {
		qDebug() << "Error - return value of get_min_sampling_freq() is a string - " <<
			 result.toString();
	} else {
		qDebug() << "Error - get_min_sampling_freq - "<< result.toString();
	}

	return 0;
}

uint32_t JSPattern::get_required_nr_of_samples()
{
	QJSValue result = qEngine->evaluate("get_required_nr_of_samples()");

	if (result.isNumber()) {
		return result.toUInt();
	} else if (result.isString()) {
		qDebug() <<
			 "Error - return value of get_required_nr_of_samples() is a string - " <<
			 result.toString();
	} else {
		qDebug() << "Error - get_required_nr_of_samples - "<< result.toString();
	}

	return 0;
}

bool JSPattern::is_periodic()
{
	QJSValue result = qEngine->evaluate("is_periodic()");

	if (result.isBool()) {
		return result.toBool();
	} else if (result.isString()) {
		qDebug() << "Error - return value of is_periodic() is a string - " <<
			 result.toString();
	} else {
		qDebug() << "Error - is_periodic - "<< result.toString();
	}

	return 0;
}

uint8_t JSPattern::pre_generate()
{
	QString fileName(obj["filepath"].toString() +
			 obj["generate_script"].toString());
	qDebug()<<fileName;
	QFile scriptFile(fileName);
	scriptFile.open(QIODevice::ReadOnly);
	QTextStream stream(&scriptFile);
	QString contents = stream.readAll();
	scriptFile.close();

	qEngine->evaluate("function is_periodic(){ status_window.print(\"is_periodic() not found\")}");
	qEngine->evaluate("function get_required_nr_of_samples(){ status_window.print(\"get_required_nr_of_samples() not found\")}");
	qEngine->evaluate("function get_min_sampling_freq(){ status_window.print(\"get_min_sampling_freq() not found\")}");
	qEngine->evaluate("function generate(){ status_window.print(\"generate() not found\")}");

	// if file does not exist, stream will be empty
	handle_result(qEngine->evaluate(contents, fileName),"Eval generatescript");


	return 0;
}

bool JSPattern::handle_result(QJSValue result,QString str)
{
	if (result.isError()) {
		qDebug()
				<< "Uncaught exception at line"
				<< result.property("lineNumber").toInt()
				<< ":" << result.toString();
		return -2;
	} else {
		qDebug()<<str<<" - Success";
		return 0;
	}

}

uint8_t JSPattern::generate_pattern(uint32_t sample_rate,
				    uint32_t number_of_samples, uint16_t number_of_channels)
{

	this->sample_rate = sample_rate;
	this->number_of_channels = number_of_channels;
	this->number_of_samples = number_of_samples;
	handle_result(qEngine->evaluate("generate()"),"Eval generate");
	commitBuffer(qEngine->evaluate("pg.buffer"),qEngine->evaluate("pg.buffersize"));
	return 0;
}

quint32 JSPattern::get_nr_of_samples()
{
	return number_of_samples;
}

quint32 JSPattern::get_nr_of_channels()
{
	return number_of_channels;
}

quint32 JSPattern::get_sample_rate()
{
	return sample_rate;
}

void JSPattern::JSErrorDialog(QString errorMessage)
{
	qDebug()<<"JSErrorDialog: "<<errorMessage;
}

void JSPattern::commitBuffer(QJSValue jsBufferValue, QJSValue jsBufferSize)
{
	if (!jsBufferValue.isArray()) {
		qDebug()<<"Not an array";
		return;
	}

	if (!jsBufferSize.isNumber()) {
		qDebug()<<"Not a valid size";
		return;
	}

	delete_buffer();
	buffer = new short[jsBufferSize.toInt()];

	for (auto i=0; i<jsBufferSize.toInt(); i++) {
		if (!jsBufferValue.property(i).isError()) {
			auto val = jsBufferValue.property(i).toInt();
			buffer[i] = val;
		} else {
			buffer[i] = 0;
		}
	}
}

JSPatternUIScript_API::JSPatternUIScript_API(QObject *parent,
		JSPatternUI *pat) : QObject(parent),pattern(pat)
{}

void JSPatternUIScript_API::parse_ui()
{
	pattern->parse_ui();
}

JSPatternUI::JSPatternUI(JSPattern *pat,QJsonObject obj_,
			 QWidget *parent) : pattern(pat), parent_(parent), jspat_api(nullptr), PatternUI(parent)
{
	qDebug()<<"JSPatternUI created";
	loader = nullptr;
	pattern->ui_form = nullptr;
	ui = new Ui::GenericJSPatternUI();
	ui->setupUi(this);
	setVisible(false);
	textedit = new JSPatternUIStatusWindow(ui->jsstatus);
	//ui->jsstatus->setVisible(false);

}
JSPatternUI::~JSPatternUI()
{
	qDebug()<<"JSPatternUI destroyed";

}
void JSPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	qDebug()<<"JSPatternUI built";
	jspat_api = new JSPatternUIScript_API(this,this);
	parent_ = parent;
	parent->layout()->addWidget(this);
	QFile file(pattern->obj["filepath"].toString() +
		   pattern->obj["ui_form"].toString());
	file.open(QFile::ReadOnly);

	if (file.exists() && file.isReadable()) {
		loader = new QUiLoader;
		pattern->ui_form = loader->load(&file, ui->ui_form);
		file.close();
		form_name = pattern->ui_form->objectName();

	} else {
		qDebug() << "file does not exist";
	}


}
void JSPatternUI::destroy_ui()
{
	if (pattern->ui_form) {
		delete pattern->ui_form;
	}

	if (loader) {
		delete loader;
	}

	parent_->layout()->removeWidget(this);
	//delete ui_form;
	//   delete ui;
}

void JSPatternUI::find_all_children(QObject *parent, QJSValue property)
{

	if (parent->children().count() == 0) {
		return;
	}

	for (auto child: parent->children()) {
		if (child->objectName()!="") {
			QJSValue jschild = pattern->qEngine->newQObject(child);
			property.setProperty(child->objectName(),jschild);
			QQmlEngine::setObjectOwnership(child, QQmlEngine::CppOwnership);

			find_all_children(child, property.property(child->objectName()));
		}
	}
}

void JSPatternUI::post_load_ui()
{
	QString fileName(pattern->obj["filepath"].toString() +
			 pattern->obj["ui_script"].toString());
	qDebug()<<fileName;
	QFile scriptFile(fileName);
	scriptFile.open(QIODevice::ReadOnly);
	QTextStream stream(&scriptFile);
	QString contents = stream.readAll();
	scriptFile.close();

	JSPattern *pg = pattern;
	JSPatternUIStatusWindow *status_window = textedit;
	QJSValue pgObj =  pattern->qEngine->newQObject(pg);
	QJSValue consoleObj =  pattern->qEngine->newQObject(pattern->console);
	QJSValue statusTextObj =  pattern->qEngine->newQObject(status_window);
	QJSValue parseuiobj = pattern->qEngine->newQObject(jspat_api);
	QJSValue uiObj =
		pattern->qEngine->newQObject(/*ui_form*/ui->ui_form->findChild<QWidget *>
				(form_name));

	pattern->qEngine->globalObject().setProperty("pg",pgObj);
	QQmlEngine::setObjectOwnership(/*(QObject*)*/pg, QQmlEngine::CppOwnership);
	pattern->qEngine->globalObject().setProperty("console", consoleObj);
	QQmlEngine::setObjectOwnership(/*(QObject*)*/pattern->console,
			QQmlEngine::CppOwnership);
	pattern->qEngine->globalObject().setProperty("ui", uiObj);
	QQmlEngine::setObjectOwnership(/*(QObject*)*/pattern->ui_form,
			QQmlEngine::CppOwnership);

	pattern->qEngine->globalObject().setProperty("script", parseuiobj);
	QQmlEngine::setObjectOwnership(/*(QObject*)*/this, QQmlEngine::CppOwnership);

	pattern->qEngine->globalObject().setProperty("status_window", statusTextObj);
	QQmlEngine::setObjectOwnership(/*(QObject*)*/status_window,
			QQmlEngine::CppOwnership);

	pattern->qEngine->evaluate("ui_elements = [];");

	find_all_children(ui->ui_form->findChild<QObject *>(form_name),
			  pattern->qEngine->globalObject().property("ui_elements"));

	pattern->qEngine->evaluate("pg.buffer = [];").toString();
	pattern->qEngine->evaluate("pg.buffersize = 0;").toString();


	pattern->qEngine->evaluate("function post_load_ui(){ status_window.print(\"post_load_ui() not found\")}");
	pattern->qEngine->evaluate("function parse_ui(){ status_window.print(\"parse_ui() not found\")}");
	handle_result(pattern->qEngine->evaluate(contents, fileName),"eval ui_script");
	handle_result(pattern->qEngine->evaluate("post_load_ui()"),"post_load_ui");
}

void JSPatternUI::parse_ui()
{
	handle_result(pattern->qEngine->evaluate("parse_ui_callback()"),"parse_ui");
	Q_EMIT patternParamsChanged();
}

Pattern *JSPatternUI::get_pattern()
{
	return pattern;
}

bool JSPatternUI::handle_result(QJSValue result,QString str)
{
	pattern->handle_result(result,str);

	if (result.isError()) {
		/*    qDebug()
			    << "Uncaught exception at line"
			    << result.property("lineNumber").toInt()
			    << ":" << result.toString();
		return -2;*/
		textedit->print((QString)"Uncaught exception at line" +
				result.property("lineNumber").toString() + ":" + result.toString());
		return -2;

	} else {
		textedit->print(str + " - Success");
		return 0;
	}


}

JSPatternUIStatusWindow::JSPatternUIStatusWindow(QTextEdit *textedit)
{
	con = textedit;
}
void JSPatternUIStatusWindow::clear()
{
	con->clear();
}

void JSPatternUIStatusWindow::print(QString str)
{
	con->append(str);
}


#if 0

uint32_t LFSRPattern::get_lfsr_period() const
{
	return lfsr_period;
}

uint32_t LFSRPattern::get_lfsr_poly() const
{
	return lfsr_poly;
}

void LFSRPattern::set_lfsr_poly(const uint32_t& value)
{
	lfsr_poly = value;
}

uint16_t LFSRPattern::get_start_state() const
{
	return start_state;
}

void LFSRPattern::set_start_state(const uint16_t& value)
{
	start_state = value;
}

LFSRPattern::LFSRPattern()
{
	lfsr_poly = 0x01;
	start_state = 0x01;
	lfsr_period = 0;
	set_name("LFSR");
	set_description("Linear Feedback Shift Register sequence generated using Galois method");
	set_periodic(false);
}

uint8_t LFSRPattern::generate_pattern()
{
	// https://en.wikipedia.org/wiki/Linear-feedback_shift_register
	uint16_t lfsr = start_state;
	int i=0;
	delete_buffer();
	buffer = new short[number_of_samples];

	do {
		unsigned lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
		lfsr >>= 1;                /* Shift register */

		if (lsb) {                 /* If the output bit is 1, apply toggle mask. */
			lfsr ^= lfsr_poly;
		}

		buffer[i] = lfsr;
		i++;
	} while (i < number_of_samples);

	return 0;
}

uint32_t LFSRPattern::compute_period()
{
	uint16_t lfsr = start_state;
	unsigned period = 0;

	do {
		unsigned lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
		lfsr >>= 1;                /* Shift register */

		if (lsb) {                 /* If the output bit is 1, apply toggle mask. */
			lfsr ^= lfsr_poly;
		}

		++period;
	} while (lfsr != start_state);

	lfsr_period = period;
	return period;
}

LFSRPatternUI::LFSRPatternUI(QWidget *parent) : PatternUI(parent)
{
	qDebug()<<"LFSRPatternUI created";
	ui = new Ui::LFSRPatternUI();
	ui->setupUi(this);
	setVisible(false);
}

LFSRPatternUI::~LFSRPatternUI()
{
	qDebug()<<"LFSRPatternUI destroyed";
}
void LFSRPatternUI::build_ui(QWidget *parent)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
}
void LFSRPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
	//    delete ui;
}

void LFSRPatternUI::parse_ui()
{
	bool ok=0;
	set_lfsr_poly(ui->genPoly->text().toULong(&ok,16));

	if (!ok) {
		qDebug()<<"LFSR Poly cannot be converted to int";
	}

	set_start_state(ui->startState->text().toULong(&ok,16));

	ui->polyPeriod->setText(QString::number(compute_period()));
}

/*void LFSRPatternUI::on_setLFSRParameters_clicked()
{
}*/



bool ConstantPattern::get_constant() const
{
	return constant;
}

void ConstantPattern::set_constant(bool value)
{
	constant = value;
}

ConstantPattern::ConstantPattern()
{
	set_name(ConstantPatternName);
	set_description(ConstantPatternDescription);
	set_periodic(false);
}
uint8_t ConstantPattern::generate_pattern()
{
	delete_buffer();
	buffer = new short[number_of_samples];

	for (auto i=0; i<number_of_samples; i++) {
		if (constant) {
			buffer[i] = 0xffff;
		} else {
			buffer[i] = 0x0000;
		}
	}

	return 0;
}

ConstantPatternUI::ConstantPatternUI(QWidget *parent) : PatternUI(parent)
{
	qDebug()<<"ConstantPatternUI created";
	ui = new Ui::ConstantPatternUI();
	ui->setupUi(this);
	setVisible(false);
}

ConstantPatternUI::~ConstantPatternUI()
{
	qDebug()<<"ConstantPatternUI destroyed";
}
void ConstantPatternUI::build_ui(QWidget *parent)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
}
void ConstantPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

void ConstantPatternUI::parse_ui()
{
	/* bool ok =0;
	set_frequency(ui->frequencyEdit->text().toFloat(&ok));
	if(!ok) qDebug()<<"Cannot set frequency, not a float";*/
	bool temp = ui->constantComboBox->currentText().toInt();
	set_constant(temp);

}

PulsePattern::PulsePattern()
{
	set_name(PulsePatternName);
	set_description(PulsePatternDescription);
	set_periodic(true);
}

uint8_t PulsePattern::generate_pattern()
{
	delete_buffer();
	buffer = new short[number_of_samples];

	float period_number_of_samples = high_number_of_samples+low_number_of_samples;
	qDebug()<<"period_number_of_samples - "<<period_number_of_samples;
	float number_of_periods = number_of_samples / period_number_of_samples;
	qDebug()<<"number_of_periods - " << number_of_periods;

	delete_buffer();
	buffer = new short[number_of_samples];
	int i=0;

	auto cnt = counter_init;
	auto div_cnt = divider_init;

	// TODO: Compute divider and divider_cnt based on actual sample rate - from 80MHz
	while (i<number_of_samples) {
		if (div_cnt<divider) {
			div_cnt++;
		} else {
			div_cnt = 0;
			cnt++;
		}

		if (cnt<low_number_of_samples) {
			buffer[i] = 0x0000;
		} else if (cnt>=low_number_of_samples
			   && cnt<(low_number_of_samples+high_number_of_samples)) {
			buffer[i] = 0xffff;
		} else {
			cnt = 0;
		}

		i++;

	}
}

uint32_t PulsePattern::get_min_sampling_freq()
{
	return 1;
}

uint32_t PulsePattern::get_required_nr_of_samples()
{
	return 1;
}

bool PulsePattern::get_start()
{
	return start;
}
uint32_t PulsePattern::get_low_number_of_samples()
{
	return low_number_of_samples;
}
uint32_t PulsePattern::get_high_number_of_samples()
{
	return high_number_of_samples;
}
uint32_t PulsePattern::get_counter_init()
{
	return counter_init;
}
uint16_t PulsePattern::get_divider()
{
	return divider;
}
uint16_t PulsePattern::get_divider_init()
{
	return divider_init;
}

void PulsePattern::set_start(bool val)
{
	start=val;
}
void PulsePattern::set_low_number_of_samples(uint32_t val)
{
	low_number_of_samples=val;
}
void PulsePattern::set_high_number_of_samples(uint32_t val)
{
	high_number_of_samples=val;
}
void PulsePattern::set_counter_init(uint32_t val)
{
	counter_init=val;
}
void PulsePattern::set_divider(uint16_t val)
{
	divider=val;
}
void PulsePattern::set_divider_init(uint16_t val)
{
	divider_init=val;
}

PulsePatternUI::PulsePatternUI(QWidget *parent)
{
	qDebug()<<"PulsePatternUI created";
	ui = new Ui::PulsePatternUI();
	ui->setupUi(this);
	setVisible(false);
}
PulsePatternUI::~PulsePatternUI()
{
	qDebug()<<"PulsePatternUI destroyed";
}

void PulsePatternUI::parse_ui()
{
	bool ok =0;
	set_start(ui->start_CB->currentText().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set start, not an int";
	}

	set_counter_init(ui->counterInit_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set counter_init, not an int";
	}

	set_low_number_of_samples(ui->low_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set_low_number_of_samples, not an int";
	}

	set_high_number_of_samples(ui->high_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set_high_number_of_samples, not an int";
	}

	set_divider(ui->divider_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set_divider, not an int";
	}

	if (get_divider()<0) {
		set_divider(1);
	}

	set_divider_init(ui->dividerInit_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set_divider_init, not an int";
	}

}

void PulsePatternUI::build_ui(QWidget *parent)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
}

void PulsePatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

JohnsonCounterPattern::JohnsonCounterPattern()
{
	set_name(JohnsonCounterPatternName);
	set_description(JohnsonCounterPatternDescription);
	set_periodic(true);
}

uint32_t JohnsonCounterPattern::get_min_sampling_freq()
{
	return frequency;
}

uint32_t JohnsonCounterPattern::get_required_nr_of_samples()
{
	// greatest common divider duty cycle and 1000;0;
	return ((float)sample_rate/(float)frequency) * (2*number_of_channels);
}

uint32_t JohnsonCounterPattern::get_frequency() const
{
	return frequency;
}

void JohnsonCounterPattern::set_frequency(const uint32_t& value)
{
	frequency = value;
}

uint8_t JohnsonCounterPattern::generate_pattern()
{
	delete_buffer();
	buffer = new short[number_of_samples];
	auto samples_per_count = ((float)sample_rate/(float)frequency);
	auto i=0;
	auto j=0;

	while (j<number_of_samples) {
		for (auto k=0; k<samples_per_count; k++,j++) {
			if (j>=number_of_samples) {
				break;
			}

			buffer[j] = i;
		}

		i=(i>>1) | (((~i)&0x01) << (number_of_channels-1));

	}

	return 0;
}

JohnsonCounterPatternUI::JohnsonCounterPatternUI(QWidget *parent)
{
	qDebug()<<"JohnsonCounterPatternUI created";
	ui = new Ui::FrequencyPatternUI();
	ui->setupUi(this);
	setVisible(false);
}
JohnsonCounterPatternUI::~JohnsonCounterPatternUI()
{
	qDebug()<<"JohnsonCounterPatternUI destroyed";
}

void JohnsonCounterPatternUI::parse_ui()
{
	bool ok =0;
	set_frequency(ui->frequencyLineEdit->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set frequency, not an int";
	}
}

void JohnsonCounterPatternUI::build_ui(QWidget *parent)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
}

void JohnsonCounterPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

uint32_t WalkingPattern::get_frequency() const
{
	return frequency;
}

void WalkingPattern::set_frequency(const uint32_t& value)
{
	frequency = value;
}

uint16_t WalkingPattern::get_length() const
{
	return length;
}

void WalkingPattern::set_length(const uint16_t& value)
{
	length = value;
}

bool WalkingPattern::get_right() const
{
	return right;
}

void WalkingPattern::set_right(bool value)
{
	right = value;
}

bool WalkingPattern::get_level() const
{
	return level;
}

void WalkingPattern::set_level(bool value)
{
	level = value;
}

WalkingPattern::WalkingPattern()
{
	set_name(WalkingCounterPatternName);
	set_description(WalkingCounterPatternDescription);
	set_periodic(1);
}

uint32_t WalkingPattern::get_min_sampling_freq()
{
	return frequency;
}

uint32_t WalkingPattern::get_required_nr_of_samples()
{
	// greatest common divider duty cycle and 1000;0;
	return ((float)sample_rate/(float)frequency) * (number_of_channels);
}

uint8_t WalkingPattern::generate_pattern()
{
	delete_buffer();
	buffer = new short[number_of_samples];
	auto samples_per_count = ((float)sample_rate/(float)frequency);
	uint16_t i;
	i = (1<<length) - 1;

	if (!level) {
		i = ~i;
	}

	auto j=0;

	while (j<number_of_samples) {

		for (auto k=0; k<samples_per_count; k++,j++) {
			if (j>=number_of_samples) {
				break;
			}

			buffer[j] = i;
		}

		if (right) {
			auto dummy = i&0x01;
			i=(i>>1);

			if (dummy) {
				i = i | (1 << (number_of_channels-1));
			} else {
				i = i & ~(1<<(number_of_channels-1));
			}
		} else {
			i=(i<<1) | ((i&(1<<number_of_channels)) >> number_of_channels);
		}

	}

	return 0;
}

WalkingPatternUI::WalkingPatternUI(QWidget *parent)
{
	qDebug()<<"WalkingCounterPatternUI created";
	ui = new Ui::WalkingPatternUI();
	ui->setupUi(this);
	setVisible(false);
}
WalkingPatternUI::~WalkingPatternUI()
{
	qDebug()<<"WalkingCounterPatternUI destroyed";
}

void WalkingPatternUI::parse_ui()
{
	bool ok =0;
	set_frequency(ui->frequency_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set frequency, not an int";
	}

	set_length(ui->length_LE->text().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set length_LE, not an int";
	}

	set_level(ui->level_CB->currentText().toInt(&ok,10));

	if (!ok) {
		qDebug()<<"Cannot set frequency, not an int";
	}

	set_right("Right"==ui->direction_CB->currentText());

	if (!ok) {
		qDebug()<<"Cannot set frequency, not an int";
	}
}

void WalkingPatternUI::build_ui(QWidget *parent)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
}

void WalkingPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

#endif

uint32_t ImportPattern::get_min_sampling_freq()
{
	return frequency;
}

uint32_t ImportPattern::get_required_nr_of_samples(uint32_t sample_rate,
		uint32_t number_of_channels)
{
	// greatest common divider duty cycle and 1000;0;
	uint32_t period_number_of_samples = ((uint32_t)sample_rate/frequency) * data.size();
	return period_number_of_samples;
}


float ImportPattern::get_frequency() const
{
	return frequency;
}

void ImportPattern::set_frequency(float value)
{
	frequency = value;
}


float ImportPattern::getFrequency() const
{
	return frequency;
}

void ImportPattern::setFrequency(float value)
{
	frequency = value;
}

bool ImportPattern::useNativeDialog() const
{
	return nativeDialog;
}

void ImportPattern::setNativeDialog(bool nativeDialog)
{
	this->nativeDialog = nativeDialog;
}

ImportPattern::ImportPattern():
	fileName(""),
	channel_mapping(0),
	nativeDialog(true)
{
	set_name("Import");
	set_description("Import pattern");
	set_periodic(false);
	set_frequency(5000);
}

ImportPattern::~ImportPattern()
{

}

uint8_t ImportPattern::generate_pattern(uint32_t sample_rate,
				       uint32_t number_of_samples, uint16_t number_of_channels)
{
	float f_period_number_of_samples = (float)sample_rate/frequency;
	qDebug()<<"period_number_of_samples - "<<f_period_number_of_samples;

	int period_number_of_samples = (int)round(f_period_number_of_samples);
	if (period_number_of_samples==0) {
		period_number_of_samples=1;
	}

	delete_buffer();
	buffer = new short[number_of_samples];


	for (int i = 0; i < data.size(); ++i) {
		for (int j = 0; j < period_number_of_samples; ++j) {
			buffer[i * period_number_of_samples + j] = data[i];
		}
	}
	int i = data.size() * period_number_of_samples;
	for (; i < number_of_samples; ++i) {
		buffer[i] = 0xffff;
	}

	return 0;
}

ImportPatternUI::ImportPatternUI(ImportPattern *pattern,
			       QWidget *parent) : PatternUI(parent), pattern(pattern)
{
	ui = new Ui::EmptyPatternUI();
	ui->setupUi(this);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Frequency"), 1e0, PG_MAX_SAMPLERATE/2,true,false,this, {1,2.5,5});
	import_settings = new ImportSettings(this);
	fileLineEdit = new QLineEdit(this);
	openFileBtn = new QPushButton(tr("Open file"), this);
	importBtn = new QPushButton(tr("Import selected channels"), this);
	fileLineEdit->setDisabled(true);
	fileLineEdit->setText(tr("No file selected"));
	import_settings->setDisabled(true);
	importBtn->setDisabled(true);
	ui->verticalLayout->setSpacing(10);
	ui->verticalLayout->addWidget(fileLineEdit);
	ui->verticalLayout->addWidget(openFileBtn);
	ui->verticalLayout->addWidget(import_settings);
	ui->verticalLayout->addWidget(frequencySpinButton);
	ui->verticalLayout->addWidget(importBtn);
	this->setStylesheet();
	setVisible(false);
}

void ImportPatternUI::setStylesheet()
{
	QString style = "QPushButton{"
			"width: 175px;"
			"height: 30px;"
			"border-radius: 4px;"
			"background-color: #4a64ff;"
			"font-size: 14px;"
			"font-weight: normal;"
			"font-style: normal;"
			"text-align: center;"
			"color: #ffffff;"
		      "}"
			"QPushButton:disabled {"
			"background-color: gray;"
			"color:white;"
			"}"
		      "QPushButton:hover"
		      "{"
			"background-color: #4a34ff;"
		      "}";
	openFileBtn->setStyleSheet(style);
	importBtn->setStyleSheet(style);
}

ImportPatternUI::~ImportPatternUI()
{
	//qDebug()<<"ClockPatternUI destroyed";
	delete ui;
}

Pattern *ImportPatternUI::get_pattern()
{
	return pattern;
}

void ImportPatternUI::reloadFileData()
{
	try {
		loadFileData(this->fileName);
		parse_ui();
	} catch (FileManagerException &e) {
		fileLineEdit->setText(QString(e.what()));
		fileLineEdit->setToolTip("");
		importBtn->setDisabled(true);
	}
}

void ImportPatternUI::loadFileData(QString fileName)
{
		FileManager fm("Pattern Generator");
		fm.open(fileName, FileManager::IMPORT);
		data.clear();
		data = fm.read();
		this->fileName = fileName;
		pattern->fileName = fileName;
}


void ImportPatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	requestedFrequency=pattern->get_frequency();
	frequencySpinButton->setValue(pattern->get_frequency());

	try {
		loadFileData(fileName);
		import_settings->clear();
		for (int i = 0; i < data[0].size(); ++i) {
			import_settings->addChannel(i, "CH" + QString::number(i));
		}

		fileLineEdit->setText(fileName);
		fileLineEdit->setToolTip(fileName);
		import_settings->setDisabled(false);
		importBtn->setDisabled(false);

		unsigned short mask = pattern->channel_mapping;

		QMap<int, bool> config;
		for (int i = 0; i < data[0].size(); ++i) {
			config[i] = (bool) (mask & (1 << i));
		}

		import_settings->setExportConfig(config);

		parse_ui();

	} catch (FileManagerException &e) {
		fileLineEdit->setText(QString(e.what()));
		fileLineEdit->setToolTip("");
		importBtn->setDisabled(true);
	}

	connect(openFileBtn, &QPushButton::clicked, [=](){
		QString fileName = QFileDialog::getOpenFileName(this,
		    tr("Export"), "", tr("Comma-separated values files (*.csv);;"
					       "Tab-delimited values files (*.txt)"),
		    nullptr, (pattern->useNativeDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
		try {
			loadFileData(fileName);

			import_settings->clear();
			for (int i = 0; i < data[0].size(); ++i) {
				import_settings->addChannel(i, "CH" + QString::number(i));
			}

			fileLineEdit->setText(fileName);
			fileLineEdit->setToolTip(fileName);
			import_settings->setDisabled(false);
			importBtn->setDisabled(false);

		} catch (FileManagerException &e) {
			fileLineEdit->setText(QString(e.what()));
			fileLineEdit->setToolTip("");
			importBtn->setDisabled(true);
		}

	});

	connect(frequencySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));
	connect(importBtn, &QPushButton::clicked, this, &ImportPatternUI::reloadFileData);

}
void ImportPatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

void ImportPatternUI::parse_ui()
{
	QObject *obj = sender();
	bool freqStepDown = false;

	if (obj==frequencySpinButton) {
		if (frequencySpinButton->value() < requestedFrequency) {
			freqStepDown=true;
		}
		requestedFrequency = frequencySpinButton->value();
	}

	auto freq=requestedFrequency;//frequencySpinButton->value();
	long div;
	if (freqStepDown) {
		div=(long)ceil((double)PG_MAX_SAMPLERATE/freq);
	} else {
		div=(long)floor((double)PG_MAX_SAMPLERATE/freq);
	}

	unsigned short mask = 0;
	auto keys = import_settings->getExportConfig().keys();
	for (int key : qAsConst(keys)) {
		mask = mask | (import_settings->getExportConfig()[key] << key);
	}

	pattern->channel_mapping = mask;
	pattern->data.clear();
	for (int i = 0; i < data.size(); ++i) {
		int k = 0;
		unsigned short line_mask = 0;
		for (int j = 0 ; j < data[i].size(); ++j) {
			if (mask & (1 << j) ) {
				line_mask |= ((int)data[i][j] << k);
				k++;
			}
		}
		pattern->data.push_back(line_mask);
	}

	freq=(PG_MAX_SAMPLERATE)/(float)div;
	requestedFrequency=freq;
	frequencySpinButton->blockSignals(true);
	frequencySpinButton->setValue(freq);
	frequencySpinButton->blockSignals(false);
	pattern->set_frequency(freq);
	pattern->fileName = fileName;



	Q_EMIT patternParamsChanged();

}



int PatternFactory::static_ui_limit = 0;
QStringList PatternFactory::ui_list = {};
QStringList PatternFactory::description_list = {};
QJsonObject PatternFactory::patterns = {};


void PatternFactory::init()
{
	QJsonObject pattern_object;

	ui_list.clear();

	ui_list.append(ClockPatternName);
	description_list.append(ClockPatternDescription);
	ui_list.append(NumberPatternName);
	description_list.append(NumberPatternDescription);
	ui_list.append(RandomPatternName);
	description_list.append(RandomPatternDescription);
	ui_list.append(BinaryCounterPatternName);
	description_list.append(BinaryCounterPatternDescription);
	ui_list.append(PulsePatternName);
	description_list.append(PulsePatternDescription);
	ui_list.append(UARTPatternName);
	description_list.append(UARTPatternDescription);
	ui_list.append(SPIPatternName);
	description_list.append(SPIPatternDescription);
	ui_list.append(I2CPatternName);
	description_list.append(I2CPatternDescription);
	ui_list.append(GrayCounterPatternName);
	description_list.append(GrayCounterPatternDescription);
	ui_list.append(ImportPatternName);
	description_list.append(ImportPatternDescription);
	/*
	ui_list.append(ConstantPatternName);
	description_list.append(ConstantPatternDescription);
	ui_list.append(NumberPatternName);
	description_list.append(NumberPatternDescription);

	ui_list.append(BinaryCounterPatternName);
	description_list.append(BinaryCounterPatternDescription);
	ui_list.append(GrayCounterPatternName);
	description_list.append(GrayCounterPatternDescription);
	ui_list.append(JohnsonCounterPatternName);
	description_list.append(JohnsonCounterPatternDescription);
	ui_list.append(WalkingCounterPatternName);
	description_list.append(WalkingCounterPatternDescription);
	*/
	static_ui_limit = ui_list.count();

	QString searchPattern = "generator.json";
	QDirIterator it("patterngenerator", QStringList() << searchPattern, QDir::Files,
			QDirIterator::Subdirectories);
	int i = 0;

	while (it.hasNext()) {
		QFile file;
		QString filename = it.next();

		file.setFileName(filename);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QJsonDocument d = QJsonDocument::fromJson(file.readAll());
		file.close();
		QJsonObject obj(d.object());

		filename.chop(searchPattern.length());
		obj.insert("filepath",filename);

		if (obj["enabled"] == true) {
			ui_list.append(obj["name"].toString());
			description_list.append(obj["description"].toString());
			pattern_object.insert(QString::number(i),QJsonValue(obj));
			i++;
		}

	}

	patterns = pattern_object;
	qDebug()<<patterns;
}


Pattern *PatternFactory::create(QString name)
{
	int i=0;

	for (const auto &str : qAsConst(ui_list)) {
		if (name==str) {
			return create(i);
		}

		i++;
	}

	return create(0);
}

Pattern *PatternFactory::create(int index)
{
	switch (index) {
	case ClockPatternId:
		return new ClockPattern();

	case RandomPatternId:
		return new RandomPattern();

	case BinaryCounterId:
		return new BinaryCounterPattern();

	case PulsePatternId:
		return new PulsePattern();

	case UARTPatternId:
		return new UARTPattern();

	case NumberPatternId:
		return new NumberPattern();

	case GrayCounterId:
		return new GrayCounterPattern();

	case SPIPatternId:
		return new SPIPattern();

	case I2CPatternId:
		return new I2CPattern();

	case ImportPatternId:
		return new ImportPattern();

	default:
		if (index>=static_ui_limit) {
			return new JSPattern(patterns[QString::number(index
						      -static_ui_limit)].toObject());
		} else {
			return nullptr;
		}

		/* case 0: return new ConstantPattern();
		 case 1: return new NumberPattern();

		 case 3: return new PulsePattern();

		 case 5: return new BinaryCounterPattern();
		 case 6: return new GrayCounterPattern();
		 case 7: return new JohnsonCounterPattern();
		 case 8: return new WalkingPattern();
		 default:
		     if(index>=static_ui_limit)
		     {
			 return new JSPattern(patterns[QString::number(static_ui_limit-index)].toObject());
		     }
		     else
		     {
			 return nullptr;
		     }
		     */
	}
}

PatternUI *PatternFactory::create_ui(Pattern *pattern, QWidget *parent)
{
	int i=0;
	QString name = QString::fromStdString(pattern->get_name());

	for (const auto &str : qAsConst(ui_list)) {
		if (name==str) {
			return create_ui(pattern,i,parent);
		}

		i++;
	}

	return create_ui(pattern,0,parent);
}

PatternUI *PatternFactory::create_ui(Pattern *pattern, int index,
				     QWidget *parent)
{
	switch (index) {
	case ClockPatternId:
		return new ClockPatternUI(dynamic_cast<ClockPattern *>(pattern),parent);

	case RandomPatternId:
		return new RandomPatternUI(dynamic_cast<RandomPattern *>(pattern),parent);

	case BinaryCounterId:
		return new BinaryCounterPatternUI(dynamic_cast<BinaryCounterPattern *>(pattern),
						  parent);

	case UARTPatternId:
		return new UARTPatternUI(dynamic_cast<UARTPattern *>(pattern),
					 parent);

	case PulsePatternId:
		return new PulsePatternUI(dynamic_cast<PulsePattern* >(pattern), parent);

	case NumberPatternId:
		return new NumberPatternUI(dynamic_cast<NumberPattern *>(pattern),
					   parent);

	case GrayCounterId:
		return new GrayCounterPatternUI(dynamic_cast<GrayCounterPattern *>(pattern),
						parent);

	case SPIPatternId:
		return new SPIPatternUI(dynamic_cast<SPIPattern *>(pattern),
					parent);

	case I2CPatternId:
		return new I2CPatternUI(dynamic_cast<I2CPattern *>(pattern),
					parent);
	case ImportPatternId:
		return new ImportPatternUI(dynamic_cast<ImportPattern *>(pattern),
					parent);


	default:
		if (index>=static_ui_limit) {
			auto jspat = dynamic_cast<JSPattern *>(pattern);
			if(jspat)
				return new JSPatternUI(jspat,
						       patterns[QString::number(static_ui_limit-index)].toObject(), parent);
			else
				return nullptr;
		} else {
			return nullptr;
		}

		/*
		case 0: return new ConstantPatternUI(parent);
		case 3: return new PulsePatternUI(parent);

		case 5: return new BinaryCounterPatternUI(parent);
		case 6: return new GrayCounterPatternUI(parent);
		case 7: return new JohnsonCounterPatternUI(parent);
		case 8: return new WalkingPatternUI(parent);

		*/
	}
}



QStringList PatternFactory::get_ui_list()
{
	return ui_list;
}

QStringList PatternFactory::get_description_list()
{
	return description_list;
}


double PulsePattern::get_sample_rate() const
{
	return sample_rate;
}

void PulsePattern::set_sample_rate(double value)
{
	sample_rate = value;
}

PulsePattern::PulsePattern() : Pattern()
{
	set_name(PulsePatternName);
	set_description(PulsePatternDescription);
	set_periodic(true);
	set_sample_rate(1000000);
	set_high_number_of_samples(10);
	set_low_number_of_samples(10);
	set_counter_init(0);
	set_delay(0);
	set_no_pulses(1);
	set_start(0);

}

PulsePattern::~PulsePattern()
{

}

uint8_t PulsePattern::generate_pattern(uint32_t sample_rate, uint32_t number_of_samples, uint16_t number_of_channels)
{

	float period_number_of_samples = delay + (no_pulses * (high_number_of_samples + low_number_of_samples));
	qDebug() << "period_number_of_samples - " << period_number_of_samples;
	float number_of_periods = number_of_samples / period_number_of_samples;
	qDebug() << "number_of_periods - " << number_of_periods;

	delete_buffer();
	buffer = new short[number_of_samples];

	uint16_t buffer_val = (start) ? 0xffff : 0x0000;

	short *buffer_ptr = buffer;
	short *buffer_stop = buffer + number_of_samples;

	// can happen when specifiyng 0 delay, 0 high, 0 low samples
	if(period_number_of_samples == 0) {
		while(buffer_ptr < buffer_stop){
			*buffer_ptr = buffer_val;
			buffer_ptr++;
		}
		return 0;
	}

	while (buffer_ptr < buffer_stop)
	{
		for(size_t j = 0; j < delay; j++, buffer_ptr++)
		{
			*buffer_ptr = buffer_val;
		}
		for(size_t j = 0; j < no_pulses; j++)
		{
			auto cnt = counter_init % (low_number_of_samples + high_number_of_samples);
			for(size_t k = 0; k < low_number_of_samples + high_number_of_samples; k++, buffer_ptr++) {
				if(cnt >= high_number_of_samples + low_number_of_samples)
					cnt=0;
				if(cnt < low_number_of_samples) {
					*buffer_ptr = 0x0000;
					cnt++;
				}
				else if (cnt >= low_number_of_samples &&
						 cnt < (low_number_of_samples + high_number_of_samples))
				{
					*buffer_ptr = 0xffff;
					cnt++;
				}
			}
		}
	}
	return 0;
}

uint32_t PulsePattern::get_min_sampling_freq()
{
	return sample_rate;
}

uint32_t PulsePattern::get_required_nr_of_samples(uint32_t  sample_rate,
												  uint32_t number_of_channels)
{
	return delay+(no_pulses*(high_number_of_samples+low_number_of_samples));
}

bool PulsePattern::get_start()
{
	return start;
}

uint32_t PulsePattern::get_low_number_of_samples()
{
	return low_number_of_samples;
}

uint32_t PulsePattern::get_high_number_of_samples()
{
	return high_number_of_samples;
}

uint32_t PulsePattern::get_counter_init()
{
	return counter_init;
}

uint32_t PulsePattern::get_delay()
{
		return delay;
}

uint32_t PulsePattern::get_no_pulses()
{
	return no_pulses;
}

void PulsePattern::set_start(bool val)
{
	start=val;
}

void PulsePattern::set_low_number_of_samples(uint32_t val)
{
	low_number_of_samples=val;
}

void PulsePattern::set_high_number_of_samples(uint32_t val)
{
	high_number_of_samples=val;
}

void PulsePattern::set_counter_init(uint32_t val)
{
	counter_init=val;
}

void PulsePattern::set_delay(uint32_t val)
{
	delay=val;
}

void PulsePattern::set_no_pulses(uint32_t val)
{
	no_pulses=val;
}

PulsePatternUI::PulsePatternUI(PulsePattern *pattern,
							   QWidget *parent) : PatternUI(parent), pattern(pattern), parent_(parent)
{
	qDebug()<<"PulsePatternUI created";
	ui = new Ui::PulsePatternUI();
	ui->setupUi(this);
	frequencySpinButton = new ScaleSpinButton({
		{"Hz", 1E0},
		{"kHz", 1E+3},
		{"MHz", 1E+6}
	}, tr("Sample Rate"), 1e0, PG_MAX_SAMPLERATE,true,false,this, {1,2.5,5});
	ui->verticalLayout_2->addWidget(frequencySpinButton);

	setVisible(false);
}

PulsePatternUI::~PulsePatternUI()
{
	qDebug()<<"PulsePatternUI destroyed";
}

Pattern *PulsePatternUI::get_pattern()
{
	return pattern;
}

void PulsePatternUI::parse_ui()
{
	bool ok =0;
	int val;

	val = ui->start_CB->currentText().toInt(&ok,10);
	pattern->set_start(val);

	val = ui->counterInit_LE->text().toInt(&ok,10);
	val = (val < 0) ? 0 : val;
	pattern->set_counter_init(val);

	val = ui->low_LE->text().toInt(&ok,10);
	val = (val < 0) ? 0 : val;
	pattern->set_low_number_of_samples(val);

	val = ui->high_LE->text().toInt(&ok,10);
	val = (val < 0) ? 0 : val;
	pattern->set_high_number_of_samples(val);


	val = ui->delay_LE->text().toInt(&ok,10);
	val = (val < 0) ? 0 : val;
	pattern->set_delay(val);

	val = ui->noPulses_LE->text().toInt(&ok,10);
	val = (val < 0) ? 0 : val;
	pattern->set_no_pulses(val);

	pattern->set_sample_rate(frequencySpinButton->value());

	Q_EMIT patternParamsChanged();

}

void PulsePatternUI::build_ui(QWidget *parent,uint16_t number_of_channels)
{
	parent_ = parent;
	parent->layout()->addWidget(this);
	ui->counterInit_LE->setText(QString::number(pattern->get_counter_init()));
	ui->delay_LE->setText(QString::number(pattern->get_delay()));
	ui->noPulses_LE->setText(QString::number(pattern->get_no_pulses()));
	ui->high_LE->setText(QString::number(pattern->get_high_number_of_samples()));
	ui->low_LE->setText(QString::number(pattern->get_low_number_of_samples()));
	ui->start_CB->setCurrentText(QString::number(pattern->get_start()));
	frequencySpinButton->setValue(pattern->get_sample_rate());

	connect(ui->counterInit_LE,SIGNAL(textChanged(QString)), this , SLOT(parse_ui()));
	connect(ui->delay_LE,SIGNAL(textChanged(QString)), this , SLOT(parse_ui()));
	connect(ui->noPulses_LE,SIGNAL(textChanged(QString)), this , SLOT(parse_ui()));
	connect(ui->high_LE,SIGNAL(textChanged(QString)), this , SLOT(parse_ui()));
	connect(ui->low_LE,SIGNAL(textChanged(QString)), this , SLOT(parse_ui()));
	connect(ui->start_CB,SIGNAL(activated(QString)), this , SLOT(parse_ui()));
	connect(frequencySpinButton,SIGNAL(valueChanged(double)),this,SLOT(parse_ui()));
	parse_ui();
}

void PulsePatternUI::destroy_ui()
{
	parent_->layout()->removeWidget(this);
}

} // namespace adiscope

