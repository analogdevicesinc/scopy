#include "logic_analyzer.h"

#include "ui_logic_analyzer.h"
#include "ui_cursors_settings.h"
#include "oscilloscope_plot.hpp"

#include "logicanalyzer/logicdatacurve.h"

#include "dynamicWidget.hpp"

#include <QDebug>

using namespace adiscope;
using namespace adiscope::logic;

LogicAnalyzer::LogicAnalyzer(iio_context *ctx, adiscope::Filter *filt,
			     adiscope::ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, adiscope::ToolLauncher *parent,
			     bool offline_mode_):
	Tool(ctx, toolMenuItem, nullptr, "Logic Analyzer", parent),
	ui(new Ui::LogicAnalyzer),
	m_plot(this, 16, 10),
	m_bufferPreviewer(new DigitalBufferPreviewer(40, this)),
	m_sampleRateButton(new ScaleSpinButton({
					{"Hz", 1E0},
					{"kHz", 1E+3},
					{"MHz", 1E+6}
					}, tr("Sample Rate"), 1,
					10e7,
					true, false, this, {1, 2, 5})),
	m_bufferSize(new ScaleSpinButton({
					{"samples", 1E0},
					{"k samples", 1E+3},
					{"M samples", 1E+6},
					{"G samples", 1E+9},
					}, tr("Samples"), 1,
					10e8,
					true, false, this, {1, 2, 5})),
	m_m2kContext(m2kOpen(ctx, "")),
	m_buffer(nullptr),
	m_horizOffset(0.0),
	m_timeTriggerOffset(0.0),
	m_resetHorizAxisOffset(true)

{
	// setup ui
	setupUi();

	// setup signals slots
	connectSignalsAndSlots();

	m_plot.setLeftVertAxesCount(1);

	// TODO: query libm2k for number of channels
	// TODO: Add channels on the plot

	m_buffer = new uint16_t[1000000];
	for (int i = 0; i < 1000000; ++i) {
		m_buffer[i] = rand() % (255 * 255);
	}

	double sampleRate = 100000; // 100kHz
	uint64_t bufferSize = 1000000;



	m_plot.setHorizUnitsPerDiv(1.0 / sampleRate * bufferSize / 16.0);

	M2kDigital *dig = m_m2kContext->getDigital();
	for (uint8_t i = 0; i < 3; ++i) {
		QCheckBox *channelBox = new QCheckBox("DIO " + QString::number(i));
		if (i < 8) {
			ui->channelEnumeratorLayout->addWidget(channelBox, i % 8, 0);
		} else {
			ui->channelEnumeratorLayout->addWidget(channelBox, i % 8, 1);
		}

		channelBox->setChecked(true);

		// 1 for each channel
		// m_plot.addGenericPlotCurve()
		LogicDataCurve *curve = new LogicDataCurve(nullptr, i, this);
		curve->setTraceHeight(1);
		m_plot.addDigitalPlotCurve(curve, true);
		curve->dataAvailable(0, 1000000);

		curve->setPlotConfiguration(sampleRate, 1000000, 0.0);

		connect(channelBox, &QCheckBox::toggled, [=](bool toggled){
			m_plot.enableDigitalPlotCurve(i, toggled);
			m_plot.setOffsetWidgetVisible(i, toggled);
			m_plot.replot();
		});
	}

	// Add propper zoomer
	m_plot.addZoomer(0);

	m_plot.setZoomerParams(true, 20);

	m_plot.zoomBaseUpdate();

	connect(&m_plot, &CapturePlot::timeTriggerValueChanged,
		this, &LogicAnalyzer::onTimeTriggerValueChanged);

	m_plot.enableXaxisLabels();

	m_plot.setTimeTriggerInterval(-500000, 500000);

	initBufferScrolling();

}

LogicAnalyzer::~LogicAnalyzer()
{
	delete cr_ui;
	delete ui;
}

uint16_t *LogicAnalyzer::getData()
{
	return m_buffer;
}

void LogicAnalyzer::on_btnChannelSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void LogicAnalyzer::on_btnCursors_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void LogicAnalyzer::on_btnTrigger_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void LogicAnalyzer::on_cursorsBox_toggled(bool on)
{
	m_plot.setCursorReadoutsVisible(on);
	m_plot.setVertCursorsEnabled(on);
}

void LogicAnalyzer::on_btnSettings_clicked(bool checked)
{
	CustomPushButton *btn = nullptr;

	if (checked && !m_menuOrder.isEmpty()) {
		btn = m_menuOrder.back();
		m_menuOrder.pop_back();
	} else {
		btn = static_cast<CustomPushButton *>(
			ui->settings_group->checkedButton());
	}

	btn->setChecked(checked);
}

void LogicAnalyzer::on_btnGeneralSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
	if(checked)
		ui->btnSettings->setChecked(!checked);
}

void LogicAnalyzer::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (m_menuButtonActions.size()) {
		auto pair = m_menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void LogicAnalyzer::onTimeTriggerValueChanged(double value)
{
	m_plot.cancelZoom();
	m_plot.zoomBaseUpdate();

	qDebug() << "timeTriggermoved: " << value;
	m_plot.setHorizOffset(value);
	m_plot.replot();

	if (m_resetHorizAxisOffset) {
		m_horizOffset = value;
	}

	updateBufferPreviewer();
}

void LogicAnalyzer::setupUi()
{
	ui->setupUi(this);

	// Hide the run button
	ui->runSingleWidget->enableRunButton(false);

	int gsettings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnGeneralSettings->setProperty("id", QVariant(-gsettings_panel));

	int measure_panel = ui->stackedWidget->insertWidget(-1, new QWidget());
	ui->btnChannelSettings->setProperty("id", QVariant(-measure_panel));

	/* Cursors Settings */
	ui->btnCursors->setProperty("id", QVariant(-1));

	/* Trigger Settings */
	int triggers_panel = ui->stackedWidget->insertWidget(-1, new QWidget());
	ui->btnTrigger->setProperty("id", QVariant(-triggers_panel));

	/* Channel Settings */
	int channelSettings_panel = ui->stackedWidget->indexOf(ui->channelSettings);
	ui->btnChannelSettings->setProperty("id", QVariant(-channelSettings_panel));

	// default trigger menu?
	m_menuOrder.push_back(ui->btnTrigger);

	// set default menu width to 0
	ui->rightMenu->setMaximumWidth(0);



	// Plot positioning and settings
	m_plot.disableLegend();

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
		QSizePolicy::Fixed, QSizePolicy::Fixed);

//	ui->gridLayoutPlot->addWidget(measurePanel, 0, 1, 1, 1);
	ui->gridLayoutPlot->addWidget(m_plot.topArea(), 0, 0, 1, 4);
	ui->gridLayoutPlot->addWidget(m_plot.topHandlesArea(), 1, 0, 1, 4);

	ui->gridLayoutPlot->addWidget(m_plot.leftHandlesArea(), 0, 0, 4, 1);
	ui->gridLayoutPlot->addWidget(m_plot.rightHandlesArea(), 0, 3, 4, 1);

	ui->gridLayoutPlot->addWidget(&m_plot, 2, 1, 1, 1);
//	ui->gridLayoutPlot->addWidget(&hist_plot, 3, 2, 1, 1);

	ui->gridLayoutPlot->addWidget(m_plot.bottomHandlesArea(), 3, 0, 1, 4);
	ui->gridLayoutPlot->addItem(plotSpacer, 4, 0, 1, 4);
//	ui->gridLayoutPlot->addWidget(statisticsPanel, 6, 1, 1, 1);

	m_plot.enableAxis(QwtPlot::yLeft, false);
	m_plot.enableAxis(QwtPlot::xBottom, false);

	m_plot.setUsingLeftAxisScales(false);
	m_plot.enableLabels(false);

	// Buffer previewer

	m_bufferPreviewer->setVerticalSpacing(6);
	m_bufferPreviewer->setMinimumHeight(20);
	m_bufferPreviewer->setMaximumHeight(20);
	m_bufferPreviewer->setMinimumWidth(375);
//	m_bufferPreviewer->setMaximumWidth(375);
	m_bufferPreviewer->setCursorPos(0.5);

	m_bufferPreviewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	ui->vLayoutBufferSlot->addWidget(m_bufferPreviewer);

	m_plot.canvas()->setStyleSheet("background-color: #272730");


	// Setup sweep settings menu

	ui->sweepSettingLayout->addWidget(m_sampleRateButton);
	ui->sweepSettingLayout->addWidget(m_bufferSize);

	// Setup trigger menu

	// Setup channel / decoder menu


	// Setup cursors menu

	cr_ui = new Ui::CursorsSettings;
	cr_ui->setupUi(ui->cursorsSettings);

	setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);

	auto cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);
	connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
		[=](CustomPlotPositionButton::ReadoutsPosition position){
		m_plot.moveCursorReadouts(position);
	});

	// Disable some options we don't need for this cursor settings panel
	cr_ui->btnNormalTrack->setVisible(false);
	cr_ui->label_3->setVisible(false);
	cr_ui->line_3->setVisible(false);
	cr_ui->vCursorsEnable->setVisible(false);
	cr_ui->btnLockVertical->setVisible(false);

	cr_ui->horizontalSlider->setMaximum(100);
	cr_ui->horizontalSlider->setMinimum(0);
	cr_ui->horizontalSlider->setSingleStep(1);
	cr_ui->horizontalSlider->setSliderPosition(0);


}

void LogicAnalyzer::connectSignalsAndSlots()
{
	// connect all the signals and slots here
	// between UI (View) and it's implementation (Controller)

	connect(ui->rightMenu, &MenuAnim::finished,
		this, &LogicAnalyzer::rightMenuFinished);


	// TODO: can be moved away from here into on_cursorsBox_toggled
	connect(ui->cursorsBox, &QCheckBox::toggled, [=](bool toggled){
		if (!toggled) {
			// make sure to deselect the cursors button if
			// the cursors are disabled
			ui->btnCursors->setChecked(false);

			// we also remove the button from the history
			// so that the last menu opened button on top
			// won't open the cursors menu when it is disabled
			m_menuOrder.removeOne(ui->btnCursors);
		}
	});

	connect(&m_plot, &CapturePlot::plotSizeChanged, [=](){
		m_bufferPreviewer->setFixedWidth(m_plot.canvas()->size().width());
	});

	// some conenctions for the cursors menu
	connect(cr_ui->hCursorsEnable, &CustomSwitch::toggled,
		&m_plot, &CapturePlot::setVertCursorsEnabled);
	connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
		&m_plot, &CapturePlot::setHorizCursorsLocked);

	connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
		cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
		m_plot.setCursorReadoutsTransparency(value);
	});

	connect(m_plot.getZoomer(), &OscPlotZoomer::zoomFinished, [=](bool isZoomOut){
		updateBufferPreviewer();
	});

}

void LogicAnalyzer::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	if (ui->rightMenu->animInProgress()) {
		m_menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void LogicAnalyzer::toggleRightMenu(CustomPushButton *btn, bool checked)
{

	qDebug() << "toggleRightMenu called!";

	int id = btn->property("id").toInt();

	if (id != -ui->stackedWidget->indexOf(ui->generalSettings)){
		if (!m_menuOrder.contains(btn)){
			m_menuOrder.push_back(btn);
		} else {
			m_menuOrder.removeOne(btn);
			m_menuOrder.push_back(btn);
		}
	}

	if (checked) {
		settingsPanelUpdate(id);
	}

	ui->rightMenu->toggleMenu(checked);
}

void LogicAnalyzer::settingsPanelUpdate(int id)
{
	if (id >= 0) {
		ui->stackedWidget->setCurrentIndex(0);
	} else {
		ui->stackedWidget->setCurrentIndex(-id);
	}

	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QSizePolicy::Policy policy = QSizePolicy::Ignored;

		if (i == ui->stackedWidget->currentIndex()) {
			policy = QSizePolicy::Expanding;
		}
		QWidget *widget = ui->stackedWidget->widget(i);
		widget->setSizePolicy(policy, policy);
	}
	ui->stackedWidget->adjustSize();
}

void LogicAnalyzer::updateBufferPreviewer()
{
	// Time interval within the plot canvas
	QwtInterval plotInterval = m_plot.axisInterval(QwtPlot::xBottom);

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, 0.0);
	long long triggerSamples = /*trigger_settings.triggerDelay();*/0;
	long long totalSamples = /*last_set_sample_count*/ 1000000;

	if (totalSamples > 0) {
		dataInterval.setMinValue(-5);
		dataInterval.setMaxValue(5);
	}

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) /
		fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) /
		fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();

	// Determine the cursor position
	QwtInterval containerInterval = (totalSamples > 0) ? dataInterval :
		fullInterval;
	double containerWidth = (totalSamples > 0) ? wWidth : 1;
	double containerPos = (totalSamples > 0) ? wPos : 0;
	double cPosInContainer = 1 - (containerInterval.maxValue() - 0) /
		containerInterval.width();
	double cPos = cPosInContainer * containerWidth + containerPos;

	// Update the widget
	m_bufferPreviewer->setWaveformWidth(wWidth);
	m_bufferPreviewer->setWaveformPos(wPos);
	m_bufferPreviewer->setHighlightWidth(hWidth);
	m_bufferPreviewer->setHighlightPos(hPos);
	m_bufferPreviewer->setCursorPos(cPos);
}

void LogicAnalyzer::initBufferScrolling()
{
	// TODO: remove extra signal for plot size changeee!!!!!!!

	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, [=](int value) {
		m_resetHorizAxisOffset = false;
		double moveTo = 0.0;
		auto interval = m_plot.axisInterval(QwtPlot::xBottom);
		double min = interval.minValue();
		double max = interval.maxValue();
		int width = m_bufferPreviewer->width();
		double xAxisWidth = max - min;

		moveTo = value * xAxisWidth / width;
		m_plot.setHorizOffset(moveTo + m_horizOffset);
		m_plot.replot();
		updateBufferPreviewer();
	});
	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, [=](){
		m_horizOffset = m_plot.HorizOffset();
		m_resetHorizAxisOffset = true;
	});
	connect(m_bufferPreviewer, &BufferPreviewer::bufferResetPosition, [=](){
		m_plot.setHorizOffset(m_timeTriggerOffset);
		m_plot.replot();
		updateBufferPreviewer();
		m_horizOffset = m_timeTriggerOffset;
	});
}
