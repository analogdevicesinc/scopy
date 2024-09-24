#include "fftplotcomponentsettings.h"
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <QWidget>
#include <QLineEdit>
#include "fftplotcomponentchannel.h"

#include <gnuradio/fft/window.h>

using namespace scopy;
using namespace scopy::adc;

FFTPlotComponentSettings::FFTPlotComponentSettings(FFTPlotComponent *plt, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_plotComponent(plt)
	, m_autoscaleEnabled(false)
	, m_running(false)

{
	// This could be refactored in it's own class
	QVBoxLayout *v = new QVBoxLayout(this);
	v->setSpacing(0);
	v->setMargin(0);
	setLayout(v);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	MenuSectionCollapseWidget *plotMenu = new MenuSectionCollapseWidget(
		"SETTINGS", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QLabel *plotTitleLabel = new QLabel("Plot title");
	StyleHelper::MenuSmallLabel(plotTitleLabel);

	QLineEdit *plotTitle = new QLineEdit(m_plotComponent->name());
	StyleHelper::MenuLineEdit(plotTitle);
	connect(plotTitle, &QLineEdit::textChanged, this, [=](QString s) {
		m_plotComponent->setName(s);
		//	plotMenu->setTitle("PLOT - " + s);
	});

	MenuOnOffSwitch *labelsSwitch = new MenuOnOffSwitch("Show plot labels", plotMenu, false);
	connect(labelsSwitch->onOffswitch(), &QAbstractButton::toggled, m_plotComponent,
		&PlotComponent::showPlotLabels);

	MenuSectionCollapseWidget *yaxis = new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_NONE,
									 MenuCollapseSection::MHW_BASEWIDGET, parent);

	m_yCtrl = new MenuPlotAxisRangeControl(m_plotComponent->fftPlot()->yAxis(), this);
	m_yCtrl->minSpinbox()->setIncrementMode(MenuSpinbox::IS_FIXED);
	m_yCtrl->maxSpinbox()->setIncrementMode(MenuSpinbox::IS_FIXED);
	m_yCtrl->minSpinbox()->setUnit("dB");
	m_yCtrl->maxSpinbox()->setUnit("dB");
	m_yCtrl->minSpinbox()->setMinValue(-1000);
	m_yCtrl->maxSpinbox()->setMinValue(-1000);
	m_yCtrl->minSpinbox()->setMaxValue(1000);
	m_yCtrl->maxSpinbox()->setMaxValue(1000);
	m_yCtrl->minSpinbox()->setScaleRange(1, 1);
	m_yCtrl->maxSpinbox()->setScaleRange(1, 1);

	MenuOnOffSwitch *m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), plotMenu, false);
	m_autoscaler = new PlotAutoscaler(this);

	connect(m_autoscaler, &PlotAutoscaler::newMin, this, [=](double v) { m_yCtrl->setMin(v - 10); });
	// connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	m_plotComponent->fftPlot()->yAxis()->setUnits("dB");
	m_plotComponent->fftPlot()->yAxis()->setUnitsVisible(true);
	m_plotComponent->fftPlot()->yAxis()->getFormatter()->setTwoDecimalMode(false);

	m_yPwrOffset = new MenuSpinbox("Power Offset", 0, "dB", -300, 300, true, false, yaxis);
	m_yPwrOffset->setScaleRange(1, 1);
	m_yPwrOffset->setIncrementMode(MenuSpinbox::IS_FIXED);

	m_windowCb = new MenuCombo("Window", yaxis);

	m_windowCb->combo()->addItem("Hann", gr::fft::window::WIN_HANN);
	m_windowCb->combo()->addItem("Hanning", gr::fft::window::WIN_HANNING);
	m_windowCb->combo()->addItem("Blackman", gr::fft::window::WIN_BLACKMAN);
	m_windowCb->combo()->addItem("Rectangular", gr::fft::window::WIN_RECTANGULAR);
	m_windowCb->combo()->addItem("Flattop", gr::fft::window::WIN_FLATTOP);
	m_windowCb->combo()->addItem("Blackman-Harris", gr::fft::window::WIN_BLACKMAN_hARRIS);
	m_windowCb->combo()->addItem("Bartlett", gr::fft::window::WIN_BARTLETT);
	m_windowCb->combo()->setCurrentIndex(0);

	connect(m_windowCb->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		for(auto c : m_channels) {
			if(dynamic_cast<FFTChannel *>(c)) {
				FFTChannel *fc = dynamic_cast<FFTChannel *>(c);
				fc->setWindow(m_windowCb->combo()->itemData(idx).toInt());
			}
		}
	});

	m_windowChkb = new MenuOnOffSwitch("Window Correction", this);
	connect(m_windowChkb->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		for(auto c : m_channels) {
			if(dynamic_cast<FFTChannel *>(c)) {
				FFTChannel *fc = dynamic_cast<FFTChannel *>(c);
				fc->setWindowCorrection(b);
			}
		}
	});
	m_windowChkb->onOffswitch()->setChecked(true);

	m_curve = new MenuPlotChannelCurveStyleControl(plotMenu);

	m_deletePlot = new QPushButton("DELETE PLOT");
	StyleHelper::BlueButton(m_deletePlot);
	connect(m_deletePlot, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestDeletePlot(); });

	yaxis->contentLayout()->addWidget(m_autoscaleBtn);
	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxis->contentLayout()->addWidget(m_yPwrOffset);
	yaxis->contentLayout()->addWidget(m_windowCb);
	yaxis->contentLayout()->addWidget(m_windowChkb);
	yaxis->contentLayout()->setSpacing(10);

	plotMenu->contentLayout()->addWidget(plotTitleLabel);
	plotMenu->contentLayout()->addWidget(plotTitle);
	plotMenu->contentLayout()->addWidget(labelsSwitch);
	plotMenu->contentLayout()->addWidget(m_curve);
	plotMenu->contentLayout()->setSpacing(10);

	v->setSpacing(10);
	v->addWidget(yaxis);
	v->addWidget(plotMenu);
	v->addWidget(m_deletePlot);
	v->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_yCtrl->setVisible(true);

	m_autoscaleBtn->onOffswitch()->setChecked(false);
	m_yCtrl->setMin(-140);
	m_yCtrl->setMax(20);
	labelsSwitch->onOffswitch()->setChecked(true);
	labelsSwitch->onOffswitch()->setChecked(false);

	m_deletePlotHover = new QPushButton("", nullptr);
	m_deletePlotHover->setMaximumSize(16, 16);
	m_deletePlotHover->setIcon(QIcon(":/gui/icons/orange_close.svg"));

	connect(m_deletePlotHover, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestDeletePlot(); });

	m_settingsPlotHover = new QPushButton("", nullptr);
	m_settingsPlotHover->setMaximumSize(16, 16);
	m_settingsPlotHover->setIcon(QIcon(":/gui/icons/scopy-default/icons/preferences.svg"));

	connect(m_settingsPlotHover, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestSettings(); });

	m_plotComponent->fftPlot()->plotButtonManager()->add(m_deletePlotHover);
	m_plotComponent->fftPlot()->plotButtonManager()->add(m_settingsPlotHover);
}

void FFTPlotComponentSettings::showDeleteButtons(bool b)
{
	m_plotComponent->fftPlot()->plotButtonManager()->setVisible(b);
	m_deletePlot->setVisible(b);
	m_settingsPlotHover->setVisible(b);
	m_deletePlotHover->setVisible(b);
}

FFTPlotComponentSettings::~FFTPlotComponentSettings() {}

void FFTPlotComponentSettings::toggleAutoScale()
{
	if(m_running && m_autoscaleEnabled) {
		m_autoscaler->start();
	} else {
		m_autoscaler->stop();
	}
}

void FFTPlotComponentSettings::addChannel(ChannelComponent *c)
{
	// https://stackoverflow.com/questions/44501171/qvariant-with-custom-class-pointer-does-not-return-same-address

	auto fftPlotComponentChannel = dynamic_cast<FFTPlotComponentChannel *>(c->plotChannelCmpt());
	m_curve->addChannels(fftPlotComponentChannel->plotChannel());

	m_autoscaler->addChannels(fftPlotComponentChannel->m_fftPlotCh);
	if(dynamic_cast<FFTChannel *>(c)) {
		FFTChannel *fc = dynamic_cast<FFTChannel *>(c);
		connections[c] << connect(m_yPwrOffset, &MenuSpinbox::valueChanged, c,
					  [=](double val) { fc->setPowerOffset(val); });
		fc->setPowerOffset(m_yPwrOffset->value());
		fc->setWindow(m_windowCb->combo()->currentData().toInt());
	}
	m_channels.append(c);
}

void FFTPlotComponentSettings::removeChannel(ChannelComponent *c)
{
	m_channels.removeAll(c);

	auto fftPlotComponentChannel = dynamic_cast<FFTPlotComponentChannel *>(c->plotChannelCmpt());
	m_curve->removeChannels(fftPlotComponentChannel->plotChannel());

	m_autoscaler->addChannels(fftPlotComponentChannel->m_fftPlotCh);

	for(const QMetaObject::Connection &c : qAsConst(connections[c])) {
		QObject::disconnect(c);
	}
	connections.remove(c);
}

void FFTPlotComponentSettings::onStart()
{
	m_running = true;
	toggleAutoScale();
}

void FFTPlotComponentSettings::onStop()
{
	m_running = false;
	toggleAutoScale();
}
