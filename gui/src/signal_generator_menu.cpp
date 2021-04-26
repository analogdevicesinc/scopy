#include "ui_signal_generator_menu.h"

#include <scopy/gui/signal_generator_menu.hpp>
#include <scopy/gui/spinbox_a.hpp>

using namespace scopy::gui;

SignalGeneratorMenu::SignalGeneratorMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SignalGeneratorMenu)
{
	m_ui->setupUi(this);

	initUi();
}

SignalGeneratorMenu::~SignalGeneratorMenu() { delete m_ui; }

void SignalGeneratorMenu::initUi()
{
	m_ui->widgetFileSubsSep->setLabel("FILE");
	m_ui->widgetFileSubsSep->setButtonVisible(false);

	m_ui->widgetNoiseSubsSep->setLabel("NOISE");
	m_ui->widgetNoiseSubsSep->setButtonVisible(true);
	m_ui->widgetNoiseSubsSep->setButtonChecked(true);

	m_ui->widgetOtherSubsSep->setLabel("OTHER");
	m_ui->widgetOtherSubsSep->setButtonVisible(true);
	m_ui->widgetOtherSubsSep->setButtonChecked(true);

	m_ui->widgetTimingSubsSep->setLabel("TIMING");
	m_ui->widgetTimingSubsSep->setButtonVisible(false);

	m_ui->widgetScalingSubsSep->setLabel("SCALING");
	m_ui->widgetScalingSubsSep->setButtonVisible(false);

	/* Create waveform control widgets */
	m_phase = new PhaseSpinButton({{tr("deg"), 1},
				       {tr("π rad"), 180},
				       {tr("ns"), 1e-9},
				       {tr("μs"), 1e-6},
				       {tr("ms"), 1e-3},
				       {tr("s"), 1e0}},
				      tr("Phase"), 0, 360, true, true, this);

	m_amplitude = new ScaleSpinButton({{tr("μVolts p-p"), 1e-6}, {tr("mVolts p-p"), 1e-3}, {tr("Volts p-p"), 1e0}},
					  tr("Amplitude"), 0.000001, 10, true, true, this);

	m_offset = new PositionSpinButton({{tr("μVolts"), 1e-6}, {tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}},
					  tr("Offset"), -5, 5, true, true, this);

	m_frequency = new ScaleSpinButton({{tr("mHz"), 1e-3}, {tr("Hz"), 1e0}, {tr("kHz"), 1e3}, {tr("MHz"), 1e6}},
					  tr("Frequency"), 0.001, 0.0, true, false, this);

	/* Create stairstep waveform control widgets*/
	m_stepsUp = new PositionSpinButton(
		{
			{"steps", 1e0},
		},
		tr("Rising"), 1, 1024, true, false, this);

	m_stepsDown = new PositionSpinButton(
		{
			{"steps", 1e0},
		},
		tr("Falling"), 1, 1024, true, false, this);

	m_stairPhase = new PositionSpinButton({{"samples", 1e0}}, tr("Phase"), 0, 1024, true, false, this);
	m_stairPhase->setFineModeAvailable(false);

	/* Create trapezoidal waveform control widgets */
	m_riseTime = new ScaleSpinButton({{tr("ns"), 1e-9}, {tr("μs"), 1e-6}, {tr("ms"), 1e-3}, {tr("s"), 1e0}},
					 tr("Rise Time"), 0, 10, true, false, this);

	m_fallTime = new ScaleSpinButton({{tr("ns"), 1e-9}, {tr("μs"), 1e-6}, {tr("ms"), 1e-3}, {tr("s"), 1e0}},
					 tr("Fall Time"), 0, 10, true, false, this);

	m_holdHighTime = new ScaleSpinButton({{tr("ns"), 1e-9}, {tr("μs"), 1e-6}, {tr("ms"), 1e-3}, {tr("s"), 1e0}},
					     tr("High Time"), 0, 10, true, false, this);

	m_holdLowTime = new ScaleSpinButton({{tr("ns"), 1e-9}, {tr("μs"), 1e-6}, {tr("ms"), 1e-3}, {tr("s"), 1e0}},
					    tr("Low Time"), 0, 10, true, false, this);

	/* Create file control widgets */

	m_filePhase = new PositionSpinButton({{"samples", 1e0}}, tr("Phase"), 0.0, 360.0, true, false, this);
	m_filePhase->setFineModeAvailable(false);

	m_fileOffset = new PositionSpinButton({{tr("μVolts"), 1e-6}, {tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}},
					      tr("Offset"), -5, 5, true, true, this);

	m_fileSampleRate = new ScaleSpinButton({{"msps", 1e-3}, {"sps", 1e0}, {"ksps", 1e3}, {"Msps", 1e6}},
					       tr("SampleRate"), 0.001, 0.0, true, false, this);
	m_fileSampleRate->setIntegerDivider(75000000);

	m_fileAmplitude = new ScaleSpinButton({{tr("μVolts"), 1e-6}, {tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}},
					      tr("Amplitude"), 0.000001, 10, true, true, this);

	m_mathSampleRate = new ScaleSpinButton({{"msps", 1e-3}, {"sps", 1e0}, {"ksps", 1e3}, {"Msps", 1e6}},
					       tr("SampleRate"), 0.001, 75000000.0, true, false, this);

	m_mathSampleRate->setIntegerDivider(75000000);

	m_mathRecordLength = new ScaleSpinButton({{"ns", 1e-9}, {"μs", 1e-6}, {"ms", 1e-3}, {"s", 1}},
						 tr("Record Length"), 1e-9, 100.0, true, false, this, {1, 2, 5});

	m_noiseAmplitude = new ScaleSpinButton({{tr("μVolts"), 1e-6}, {tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}},
					       tr("Amplitude"), 0.000001, 10, true, true, this);

	m_constantValue = new PositionSpinButton({{tr("mVolts"), 1e-3}, {tr("Volts"), 1e0}}, tr("Value"), -5, 5, true,
						 true, this);

	m_dutycycle = new PositionSpinButton({{"%", 1e0}}, tr("Duty Cycle"), -5, 100, true, false, this);

	m_ui->gridLayoutWaveform->addWidget(m_amplitude, 0, 0, 1, 1);
	m_ui->gridLayoutWaveform->addWidget(m_offset, 0, 1, 1, 1);
	m_ui->gridLayoutWaveform->addWidget(m_frequency, 1, 0, 1, 1);
	m_ui->gridLayoutWaveform->addWidget(m_phase, 1, 1, 1, 1);
	m_ui->gridLayoutWaveform->addWidget(m_dutycycle, 2, 1, 1, 1);

	m_ui->gridLayout_2->addWidget(m_riseTime, 0, 0, 1, 1);
	m_ui->gridLayout_2->addWidget(m_holdHighTime, 0, 1, 1, 1);
	m_ui->gridLayout_2->addWidget(m_fallTime, 1, 0, 1, 1);
	m_ui->gridLayout_2->addWidget(m_holdLowTime, 1, 1, 1, 1);

	m_ui->gridLayoutWaveform->addWidget(m_stepsUp, 2, 0, 1, 1);
	m_ui->gridLayoutWaveform->addWidget(m_stepsDown, 2, 1, 1, 1);
	m_ui->gridLayoutWaveform->addWidget(m_stairPhase, 1, 1, 1, 1);

	m_ui->gridLayoutWaveform_2->addWidget(m_fileAmplitude, 0, 0, 1, 1);
	m_ui->gridLayoutWaveform_2->addWidget(m_fileOffset, 0, 1, 1, 1);
	m_ui->gridLayoutWaveform_2->addWidget(m_fileSampleRate, 1, 0, 1, 1);
	m_ui->gridLayoutWaveform_2->addWidget(m_filePhase, 1, 1, 1, 1);

	m_ui->verticalLayout_3->insertWidget(0, m_mathRecordLength);
	m_ui->verticalLayout_3->insertWidget(1, m_mathSampleRate);

	m_ui->hLayout_5->insertWidget(1, m_noiseAmplitude);
	m_ui->gridLayout->addWidget(m_constantValue, 0, 0, 1, 1);

	/* Max amplitude by default */
	m_amplitude->setValue(m_amplitude->maxValue());

	/* Set max frequency according to max sample rate */
	m_fileSampleRate->setMinValue(0.1);
	m_frequency->setMaxValue(maxFrequency);
	m_fileSampleRate->setMaxValue(m_sampleRate);

	m_mathSampleRate->setMinValue(1);
	m_mathSampleRate->setMaxValue(m_sampleRate);
	m_mathSampleRate->setValue(1000000);

	/* (lowest freq * 100 * 1000) frequency by default */
	m_frequency->setValue(m_frequency->minValue() * 1000 * 1000.0);
	m_fileSampleRate->setValue(m_fileSampleRate->minValue() * 100 * 1000.0);
	m_fileAmplitude->setValue(1);
	m_fileOffset->setValue(0);
	m_filePhase->setValue(0);

	m_fileAmplitude->setDisabled(true);
	m_filePhase->setDisabled(true);
	m_fileOffset->setDisabled(true);
	m_fileSampleRate->setDisabled(true);

	m_fallTime->setMinValue(0.00000001);
	m_riseTime->setMinValue(0.00000001);
	m_holdHighTime->setMinValue(0.00000001);
	m_holdLowTime->setMinValue(0.00000001);

	m_stepsUp->setMinValue(1);
	m_stepsDown->setMinValue(1);
	m_stairPhase->setMinValue(0);

	m_fallTime->setValue(0.25);
	m_riseTime->setValue(0.25);
	m_holdHighTime->setValue(0.25);
	m_holdLowTime->setValue(0.25);

	m_stepsUp->setValue(5);
	m_stepsDown->setValue(5);
	m_stepsUp->setVisible(false);
	m_stepsDown->setVisible(false);
	m_stairPhase->setValue(0);
	m_stairPhase->setVisible(false);

	m_dutycycle->setValue(50);
	m_dutycycle->setVisible(false);
	m_ui->widgetTrapezParams->setVisible(false);
	m_mathRecordLength->setValue(m_mathRecordLength->minValue() * 100 * 1000.0);

	m_ui->cmbBoxNoiseType->setCurrentIndex(0);
	m_noiseAmplitude->setMinValue(1e-06);
	m_noiseAmplitude->setValue(m_noiseAmplitude->minValue());

	connect(m_ui->widgetNoiseSubsSep->getButton(), &QPushButton::clicked,
		[=](bool check) { m_ui->widgetNoise->setVisible(check); });

	connect(m_ui->widgetOtherSubsSep->getButton(), &QPushButton::clicked,
		[=](bool check) { m_ui->widgetAppearance->setVisible(check); });
}
