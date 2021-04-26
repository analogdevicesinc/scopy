#include "ui_network_analyzer_channel_menu.h"

#include <scopy/gui/network_analyzer_channel_menu.hpp>

using namespace scopy::gui;

NetworkAnalyzerChannelMenu::NetworkAnalyzerChannelMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::NetworkAnalyzerChannelMenu)
	, m_startStopRange(new StartStopRangeWidget(1.0, 25e06))
{
	m_ui->setupUi(this);

	initUi();
}

NetworkAnalyzerChannelMenu::~NetworkAnalyzerChannelMenu()
{
	delete m_sampleStackedWidget;
	delete m_startStopRange;
	delete m_samplesCount;
	delete m_samplesPerDecadeCount;
	delete m_samplesStepSize;
	delete m_amplitude;
	delete m_offset;
	delete m_magMax;
	delete m_magMin;
	delete m_phaseMax;
	delete m_phaseMin;
	delete m_pushDelay;
	delete m_captureDelay;
	delete m_ui;
}

void NetworkAnalyzerChannelMenu::initUi()
{
	// Initialize labels
	m_ui->widgetMenuHeader->setLabel("Channel Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4A64FF"));

	m_ui->widgetSweepSubsSep->setLabel("SWEEP");
	m_ui->widgetSweepSubsSep->setButtonVisible(false);

	m_ui->widgetDisplaySubsSep->setLabel("DISPLAY");
	m_ui->widgetDisplaySubsSep->setButtonVisible(false);

	m_ui->widgetResponseSubsSep->setLabel("RESPONSE");
	m_ui->widgetResponseSubsSep->setButtonVisible(false);

	m_ui->widgetReferenceSubsSep->setLabel("REFRENCE");
	m_ui->widgetReferenceSubsSep->setButtonVisible(false);

	m_ui->widgetBufferPreviewerSubsSep->setLabel("BUFFER PREVIEWER");
	m_ui->widgetBufferPreviewerSubsSep->setButtonVisible(false);

	// Add the startStopRange widget to the ui
	m_ui->sweepRangeLayout->addWidget(m_startStopRange);

	// Create and add controls
	m_sampleStackedWidget = new QStackedWidget(this);
	m_samplesCount = new ScaleSpinButton(
		{
			{"samples", 1e0},
		},
		tr("Samples count"), 10, 10000, false, false, this);
	m_samplesCount->setValue(1000);

	m_samplesPerDecadeCount = new ScaleSpinButton(
		{
			{"samples", 1e0},
		},
		tr("Samps/decade"), 1, 10000, false, false, this);
	m_samplesPerDecadeCount->setValue(1000);

	m_samplesStepSize = new ScaleSpinButton({{"Hz", 1e0}, {"kHz", 1e3}, {"MHz", 1e6}}, tr("Step"), 1.0, 25e06,
						false, false, this, {1, 2.5, 5, 7.5});

	m_sampleStackedWidget->addWidget(m_samplesStepSize);
	m_sampleStackedWidget->addWidget(m_samplesPerDecadeCount);
	m_startStopRange->insertWidgetIntoLayout(m_sampleStackedWidget, 2, 0);

	m_amplitude = new ScaleSpinButton({{"μVolts", 1e-6}, {"mVolts", 1e-3}, {"Volts", 1e0}}, tr("Amplitude"), 1e-6,
					  1e1, false, false, this);
	m_amplitude->setValue(1);

	m_offset = new PositionSpinButton({{"μVolts", 1e-6}, {"mVolts", 1e-3}, {"Volts", 1e0}}, tr("Offset"), -5, 5,
					  false, false, this);

	m_offset->setValue(0);

	m_magMax = new PositionSpinButton({{"dB", 1e0}}, tr("Max. Magnitude"), -120, 120, false, false, this);
	m_magMax->setValue(20);

	m_magMin = new PositionSpinButton({{"dB", 1e0}}, tr("Min. Magnitude"), -120, 120, false, false, this);
	m_magMin->setValue(-80);

	m_phaseMax = new PositionSpinButton({{"°", 1e0}}, tr("Max. Phase"), -360, 360, false, false, this);
	m_phaseMax->setValue(180);

	m_phaseMin = new PositionSpinButton({{"°", 1e0}}, tr("Min. Phase"), -360, 360, false, false, this);
	m_phaseMin->setValue(-180);

	m_pushDelay =
		new PositionSpinButton({{"ms", 1e0}, {"s", 1e3}}, tr("Settling time"), 0, 2000, false, false, this);
	m_pushDelay->setValue(0);
	m_pushDelay->setStep(10);
	m_pushDelay->setToolTip(tr("Before Buffer"));

	m_captureDelay =
		new PositionSpinButton({{"ms", 1e0}, {"s", 1e3}}, tr("Settling time"), 0, 2000, false, false, this);
	m_captureDelay->setValue(0);
	m_captureDelay->setStep(10);
	m_captureDelay->setToolTip(tr("After Buffer"));

	m_ui->vLayoutPushDelay->addWidget(m_pushDelay);
	m_ui->vLayoutCaptureDelay->addWidget(m_captureDelay);
	m_startStopRange->insertWidgetIntoLayout(m_samplesCount, 2, 1);
	m_ui->vLayoutAmplitude->addWidget(m_amplitude);
	m_ui->vLayoutOffset->addWidget(m_offset);
	m_ui->vLayoutMagMax->addWidget(m_magMax);
	m_ui->vLayoutMagMin->addWidget(m_magMin);
	m_ui->vLayoutPhaseMax->addWidget(m_phaseMax);
	m_ui->vLayoutPhaseMin->addWidget(m_phaseMin);

	m_sampleStackedWidget->setCurrentIndex(m_ui->btnIsLog->isChecked());

	setMinimumDistanceBetween(m_magMin, m_magMax, 1);
	setMinimumDistanceBetween(m_phaseMin, m_phaseMax, 1);

	// Set Delta button state
	m_ui->btnDelta->setDisabled(m_ui->btnIsLog->isChecked());

	// Connect Linear/Log btn to Delta btn
	connect(m_ui->btnIsLog, &QPushButton::toggled, [=](bool checked) {
		m_ui->btnDelta->setDisabled(checked);

		if (checked) {
			m_wasChecked = m_ui->btnDelta->isChecked();
			m_ui->btnDelta->setChecked(false);
		} else {
			m_ui->btnDelta->setChecked(m_wasChecked);
		}
	});
}

void NetworkAnalyzerChannelMenu::setMinimumDistanceBetween(SpinBoxA* min, SpinBoxA* max, double distance)
{

	connect(max, &SpinBoxA::valueChanged, [=](double value) {
		min->setMaxValue(value - distance);
		//		min->setValue(min->value());
	});
	connect(min, &SpinBoxA::valueChanged, [=](double value) {
		max->setMinValue(value + distance);
		//		max->setValue(max->value());
	});
}
