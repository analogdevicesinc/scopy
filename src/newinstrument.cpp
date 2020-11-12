#include "newinstrument.hpp"
#include <QDebug>

#include "ui_cursors_settings.h"
#include "dynamicWidget.hpp"

using namespace adiscope;

#define NR_SAMPLES 12800

NewInstrument::NewInstrument(struct iio_context *ctx, Filter *filt,
			     ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, ToolLauncher *parent):
  Tool(ctx, toolMenuItem, nullptr, "NewInstrument",
       parent),
  m_ui(new Ui::NewInstrument),
  m_plot(this, false, 16, 10),
  m_channels(2),
  zoom_level(0)
{
	m_ui->setupUi(this);

	initAcquisition();

	m_plot.registerSink("Channel", m_channels, 2);
	m_plot.disableLegend();

	m_plot.addZoomer(0);
	m_plot.addZoomer(1);

	m_plot.setYaxisUnit("");

	//this is for later, when it will be possible to select and change the channel
	connect(this, SIGNAL(selectedChannelChanged(int)),
		&m_plot, SLOT(setZoomerVertAxis(int)));

	connect(m_plot.getZoomer(), &OscPlotZoomer::zoomIn, [=](){
		zoom_level++;
		m_plot.setTimeBaseZoomed(true);
	});
	connect(m_plot.getZoomer(), &OscPlotZoomer::zoomOut, [=](){
		if (zoom_level != 0) zoom_level--;
		if (zoom_level == 0)
			m_plot.setTimeBaseZoomed(false);
	});


	/* Cursors Settings */
	const int cursor_settings = m_ui->stackedWidget->indexOf(m_ui->cursorsSettings);
	m_ui->btnCursors->setProperty("id", QVariant(cursor_settings));

//	/* Trigger Settings */
//	int triggers_panel = m_ui->stackedWidget->insertWidget(-1, &trigger_settings);
//	m_ui->btnTrigger->setProperty("id", QVariant(-triggers_panel));


	//m_plot.setDisplayScale(50);

	/* Plot layout */
	m_ui->gridLayoutPlot->addWidget(m_plot.topArea(), 0, 0, 1, 3);
	m_ui->gridLayoutPlot->addWidget(m_plot.topHandlesArea(), 1, 0, 1, 3);

	m_ui->gridLayoutPlot->addWidget(m_plot.leftHandlesArea(), 0, 0, 4, 1);
	m_ui->gridLayoutPlot->addWidget(m_plot.rightHandlesArea(), 0, 2, 4, 1);

	m_ui->gridLayoutPlot->addWidget(&m_plot, 2, 1, 1, 1);

	m_ui->gridLayoutPlot->addWidget(m_plot.bottomHandlesArea(), 3, 0, 1, 3);


	/* Default plot settings */
	m_plot.setSampleRate(1, 1, "");
	m_plot.setActiveVertAxis(0);

	m_plot.enableAxis(QwtPlot::yLeft, true);
	m_plot.enableAxis(QwtPlot::xBottom, true);
	m_plot.setUsingLeftAxisScales(false);


	m_plot.setVertUnitsPerDiv(2048 * 2.0 / 10.0);
	m_plot.setHorizUnitsPerDiv((double) NR_SAMPLES /
				  ((double) 1.0 * 16.0));
	m_plot.setHorizOffset((double) NR_SAMPLES /
			     ((double) 1.0 * 2.0));

	m_plot.setLineStyle(0,Qt::DotLine);
	m_plot.setLineStyle(1,Qt::DotLine);
	m_plot.zoomBaseUpdate();
	m_plot.replot();


	if (!wheelEventGuard)
		wheelEventGuard = new MouseWheelWidgetGuard(m_ui->mainWidget);
	wheelEventGuard->installEventRecursively(m_ui->mainWidget);

	m_ui->rightMenu->setMaximumWidth(0);

	m_running = true;

	connect(m_ui->btnGetData, &QPushButton::clicked, [=](bool check) {
			generateData();

	});

	connect(m_ui->runSingleWidget, &RunSingleWidget::toggled,
		[=](bool checked){
		auto btn = dynamic_cast<CustomPushButton *>(run_button);
		btn->setChecked(checked);
		startStop(checked);
		if(checked) {
			m_workerThread = std::thread([&]{
				while(m_running) {
					generateData();
				}
			});
		}
		else
		{
			m_workerThread.join();
		}

	});

	cursor_panel_init();

}

void NewInstrument::cursor_panel_init()
{
	cr_ui = new Ui::CursorsSettings;
	cr_ui->setupUi(m_ui->cursorsSettings);
	setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);
	setDynamicProperty(cr_ui->btnLockVertical, "use_icon", true);


	connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
		&m_plot, &CapturePlot::setHorizCursorsLocked);
	connect(cr_ui->btnLockVertical, &QPushButton::toggled,
		&m_plot, &CapturePlot::setVertCursorsLocked);

	cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);

	connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)),
		&m_plot, SLOT(setVertCursorsEnabled(bool)));
	connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)),
		&m_plot, SLOT(setHorizCursorsEnabled(bool)));

//	connect(cr_ui->btnNormalTrack, &QPushButton::toggled,
//		this, &NewInstrument::toggleCursorsMode);

	cr_ui->horizontalSlider->setMaximum(100);
	cr_ui->horizontalSlider->setMinimum(0);
	cr_ui->horizontalSlider->setSingleStep(1);
	connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
		cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
		m_plot.setCursorReadoutsTransparency(value);
	});
	cr_ui->horizontalSlider->setSliderPosition(0);

	connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
		[=](CustomPlotPositionButton::ReadoutsPosition position){
		m_plot.moveCursorReadouts(position);
	});
}

void NewInstrument::on_boxCursors_toggled(bool on)
{
	m_plot.setHorizCursorsEnabled(
			on ? cr_ui->vCursorsEnable->isChecked() : false);
	m_plot.setVertCursorsEnabled(
			on ? cr_ui->hCursorsEnable->isChecked() : false);

	m_plot.trackModeEnabled(on ? cr_ui->btnNormalTrack->isChecked() : true);

	if (on) {
		m_plot.setCursorReadoutsVisible(on);
	} else {
		if (m_ui->btnCursors->isChecked())
			m_ui->btnCursors->setChecked(false);

		//menuOrder.removeOne(m_ui->btnCursors);
	}
}

void NewInstrument::on_btnCursors_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void NewInstrument::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	if (m_ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void NewInstrument::toggleRightMenu(CustomPushButton *btn, bool checked)
{
	int id = btn->property("id").toInt();
	if (checked) {
		m_ui->stackedWidget->setCurrentIndex(id);
	}
	m_ui->rightMenu->toggleMenu(checked);
}

void NewInstrument::startStop(bool pressed)
{
	if (pressed) {
		start();
		qDebug() << "STRAAARRTTTTTTT";
	} else {
		stop();
		qDebug ()<< "STooooppppp";
	}

}

void NewInstrument::start()
{
	m_running = true;

}

void NewInstrument::stop()
{
	m_running = false;

}

void NewInstrument::initAcquisition()
{
	m_m2k = libm2k::context::m2kOpen(ctx, "");
	m_m2k_analogin = m_m2k->getAnalogIn();

	m_m2k_analogin->enableChannel(0, true);
	m_m2k_analogin->enableChannel(1,true);
	m_m2k_analogin->setKernelBuffersCount(1);

	m_m2k_analogin->setRange((libm2k::analog::ANALOG_IN_CHANNEL)0,libm2k::analog::PLUS_MINUS_2_5V);
	m_m2k_analogin->setRange((libm2k::analog::ANALOG_IN_CHANNEL)1,libm2k::analog::PLUS_MINUS_2_5V);

	m_m2k_analogin->setSampleRate(1000000);
}

void NewInstrument::generateData()
{
	auto samples = m_m2k_analogin->getSamplesRaw(NR_SAMPLES);

	std::vector<double*> m_samples;

	for(int i = 0 ; i < samples.size(); i++)
	{
		double * values = new double[samples[i].size()];
		std::copy(samples[i].begin(), samples[i].end(), values);
		m_samples.push_back(values);
	}

	std::vector< std::vector<gr::tag_t> > tags;
	m_plot.plotNewData("Channel", m_samples, NR_SAMPLES, 0, tags);
}

NewInstrument::~NewInstrument()
{
	m_ui->runSingleWidget->toggle(false);
	setDynamicProperty(runButton(), "disabled", false);
	delete cr_ui;
	delete m_ui;
}
