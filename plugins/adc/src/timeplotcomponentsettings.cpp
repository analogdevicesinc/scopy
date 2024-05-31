#include <timeplotcomponentsettings.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <QWidget>
#include <QLineEdit>
#include <timeplotcomponentchannel.h>

using namespace scopy;
using namespace scopy::adc;

TimePlotComponentSettings::TimePlotComponentSettings(TimePlotComponent *plt, QWidget *parent)
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
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	MenuSectionWidget *w = new MenuSectionWidget(this);
	v->addWidget(w);
	MenuCollapseSection *plotMenu =
		new MenuCollapseSection("PLOT - " + plt->name(), MenuCollapseSection::MHCW_NONE, w);

	QLabel *plotTitleLabel = new QLabel("Plot title");
	StyleHelper::MenuSmallLabel(plotTitleLabel);

	QLineEdit *plotTitle = new QLineEdit(m_plotComponent->name());
	StyleHelper::MenuLineEdit(plotTitle);
	connect(plotTitle, &QLineEdit::textChanged, this, [=](QString s) {
		m_plotComponent->setName(s);
		plotMenu->setTitle("PLOT - " + s);
	});

	MenuOnOffSwitch *labelsSwitch = new MenuOnOffSwitch("Show plot labels", plotMenu, false);
	connect(labelsSwitch->onOffswitch(), &QAbstractButton::toggled, m_plotComponent,
		&TimePlotComponent::showPlotLabels);

	MenuOnOffSwitch *singleYMode = new MenuOnOffSwitch("SINGLE Y MODE", plotMenu, true);
	MenuPlotAxisRangeControl *m_yCtrl = new MenuPlotAxisRangeControl(m_plotComponent->timePlot()->yAxis(), this);

	m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), plotMenu, false);

	m_autoscaler = new PlotAutoscaler(this);
	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		if(m_plotComponent->singleYMode()) {
			m_plotComponent->xyPlot()->xAxis()->setInterval(m_yCtrl->min(), m_yCtrl->max());
			m_plotComponent->xyPlot()->yAxis()->setInterval(m_yCtrl->min(), m_yCtrl->max());
		}
	});

	connect(singleYMode->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_plotComponent->setSingleYMode(b);
		m_yCtrl->setVisible(b);
		m_autoscaleBtn->setVisible(b);

		if(m_plotComponent->singleYMode()) {
			m_plotComponent->xyPlot()->xAxis()->setInterval(m_yCtrl->min(), m_yCtrl->max());
			m_plotComponent->xyPlot()->yAxis()->setInterval(m_yCtrl->min(), m_yCtrl->max());
		}
	});

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	MenuOnOffSwitch *xySwitch = new MenuOnOffSwitch("XY PLOT", plotMenu, true);

	m_xAxisSrc = new MenuCombo("XY - X Axis source");
	connect(m_xAxisSrc->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		QComboBox *cb = m_xAxisSrc->combo();
		ChannelComponent *c = static_cast<ChannelComponent *>(cb->itemData(idx).value<void *>());
		m_plotComponent->setXYXChannel(c);
	});

	m_xAxisShow = new MenuOnOffSwitch("XY - Plot X source", plotMenu, false);

	connect(xySwitch->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_plotComponent->xyPlot()->setVisible(b);
		m_xAxisSrc->setVisible(b);
		m_xAxisShow->setVisible(b);
	});

	connect(m_xAxisShow->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { m_plotComponent->showXSourceOnXy(b); });

	w->contentLayout()->addWidget(plotMenu);

	plotMenu->contentLayout()->addWidget(singleYMode);
	plotMenu->contentLayout()->addWidget(m_autoscaleBtn);
	plotMenu->contentLayout()->addWidget(m_yCtrl);

	plotMenu->contentLayout()->addWidget(xySwitch);
	plotMenu->contentLayout()->addWidget(m_xAxisSrc);
	plotMenu->contentLayout()->addWidget(m_xAxisShow);
	plotMenu->contentLayout()->addWidget(plotTitleLabel);
	plotMenu->contentLayout()->addWidget(plotTitle);
	plotMenu->contentLayout()->addWidget(labelsSwitch);
	plotMenu->contentLayout()->setSpacing(10);

	m_autoscaleBtn->setVisible(false);
	m_yCtrl->setVisible(false);
	m_xAxisSrc->setVisible(false);
	m_xAxisShow->setVisible(false);

	// init
	xySwitch->onOffswitch()->setChecked(true);
	singleYMode->onOffswitch()->setChecked(true);
	m_yCtrl->setMin(-2048);
	m_yCtrl->setMax(2048);
	labelsSwitch->onOffswitch()->setChecked(true);
	labelsSwitch->onOffswitch()->setChecked(false);
}

TimePlotComponentSettings::~TimePlotComponentSettings() {}

void TimePlotComponentSettings::addChannel(ChannelComponent *c)
{
	// https://stackoverflow.com/questions/44501171/qvariant-with-custom-class-pointer-does-not-return-same-address
	m_xAxisSrc->combo()->addItem(c->name(), QVariant::fromValue(static_cast<void *>(c)));
	m_autoscaler->addChannels(c->plotChannelCmpt()->m_timePlotCh);
	m_channels.append(c);
}

void TimePlotComponentSettings::removeChannel(ChannelComponent *c)
{
	m_channels.removeAll(c);
	int comboId = m_xAxisSrc->combo()->findData(QVariant::fromValue(static_cast<void *>(c)));
	m_xAxisSrc->combo()->removeItem(comboId);
	m_autoscaler->removeChannels(c->plotChannelCmpt()->m_timePlotCh);
}

void TimePlotComponentSettings::onInit() {}

void TimePlotComponentSettings::onDeinit() {}

void TimePlotComponentSettings::onStart()
{
	m_running = true;
	toggleAutoScale();
}

void TimePlotComponentSettings::onStop()
{
	m_running = false;
	toggleAutoScale();
}

void TimePlotComponentSettings::toggleAutoScale()
{
	if(m_running && m_autoscaleEnabled) {
		m_autoscaler->start();
	} else {
		m_autoscaler->stop();
	}
}
