#include <timeplotcomponentsettings.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
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
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);



	MenuSectionCollapseWidget *plotMenu =
		new MenuSectionCollapseWidget("SETTINGS", MenuCollapseSection::MHCW_NONE, parent);

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
		&TimePlotComponent::showPlotLabels);

	m_yCtrl = new MenuPlotAxisRangeControl(m_plotComponent->timePlot()->yAxis(), this);

	m_autoscaleBtn = new MenuOnOffSwitch(tr("AUTOSCALE"), plotMenu, false);

	m_autoscaler = new PlotAutoscaler(this);
	connect(m_autoscaler, &PlotAutoscaler::newMin, m_yCtrl, &MenuPlotAxisRangeControl::setMin);
	connect(m_autoscaler, &PlotAutoscaler::newMax, m_yCtrl, &MenuPlotAxisRangeControl::setMax);

	connect(m_yCtrl, &MenuPlotAxisRangeControl::intervalChanged, this, [=](double min, double max) {
		if(m_plotComponent->XYXChannel() && m_plotComponent->XYXChannel()->plotChannelCmpt()->m_singleYMode) {
			m_plotComponent->xyPlot()->xAxis()->setInterval(m_yCtrl->min(), m_yCtrl->max());
		}
		m_plotComponent->xyPlot()->yAxis()->setInterval(m_yCtrl->min(), m_yCtrl->max());
	});

	connect(m_autoscaleBtn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yCtrl->setEnabled(!b);
		m_autoscaleEnabled = b;
		toggleAutoScale();
	});

	MenuSectionCollapseWidget *yaxis =
		new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_NONE, parent);

	MenuSectionCollapseWidget *xySection =
		new MenuSectionCollapseWidget("XY PLOT", MenuCollapseSection::MHCW_ONOFF, parent);
	QAbstractButton *xySwitch = xySection->collapseSection()->header();

	m_xAxisSrc = new MenuCombo("XY - X Axis source");
	connect(m_xAxisSrc->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		QComboBox *cb = m_xAxisSrc->combo();
		ChannelComponent *c = static_cast<ChannelComponent *>(cb->itemData(idx).value<void *>());
		m_plotComponent->setXYXChannel(c);
	});

	m_xAxisShow = new MenuOnOffSwitch("XY - Plot X source", plotMenu, false);

	connect(xySwitch, &QAbstractButton::toggled, this, [=](bool b) {
		m_plotComponent->xyPlot()->setVisible(b);
		m_xAxisSrc->setVisible(b);
		m_xAxisShow->setVisible(b);
	});

	connect(m_xAxisShow->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { m_plotComponent->showXSourceOnXy(b); });

	m_yModeCb = new MenuCombo("YMODE", plotMenu);
	auto ycb = m_yModeCb->combo();
	ycb->addItem("ADC Counts", YMODE_COUNT);
	ycb->addItem("% Full Scale", YMODE_FS);

	connect(ycb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = ycb->itemData(idx).toInt();
		for(auto c : qAsConst(m_scaleProviders)) {
			c->setYMode(static_cast<YMode>(mode));
		}
		updateYAxis();
	});

	m_curve = new MenuPlotChannelCurveStyleControl(plotMenu);

	m_deletePlot = new QPushButton("DELETE PLOT");
	StyleHelper::BlueButton(m_deletePlot);
	connect(m_deletePlot, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestDeletePlot(); });

	yaxis->contentLayout()->addWidget(m_autoscaleBtn);
	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxis->contentLayout()->addWidget(m_yModeCb);

	plotMenu->contentLayout()->addWidget(plotTitleLabel);
	plotMenu->contentLayout()->addWidget(plotTitle);
	plotMenu->contentLayout()->addWidget(labelsSwitch);
	plotMenu->contentLayout()->addWidget(m_curve);
	plotMenu->contentLayout()->setSpacing(10);

	xySection->add(m_xAxisSrc);
	xySection->add(m_xAxisShow);

	v->setSpacing(10);
	v->addWidget(yaxis);
	v->addWidget(xySection);
	v->addWidget(plotMenu);
	v->addWidget(m_deletePlot);
	v->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_autoscaleBtn->setVisible(true);
	m_yCtrl->setVisible(true);
	m_xAxisSrc->setVisible(false);
	m_xAxisShow->setVisible(false);

	// init
	xySwitch->setChecked(true);
	m_yCtrl->setMin(-2048);
	m_yCtrl->setMax(2048);
	labelsSwitch->onOffswitch()->setChecked(true);
	labelsSwitch->onOffswitch()->setChecked(false);

	m_deletePlotHover = new QPushButton("", nullptr);
	m_deletePlotHover->setMaximumSize(16, 16);
	m_deletePlotHover->setIcon(QIcon(":/gui/icons/orange_close.svg"));

	HoverWidget *hv = new HoverWidget(m_deletePlotHover, m_plotComponent, m_plotComponent);
	hv->setStyleSheet("background-color: transparent; border: 0px;");
	hv->setContentPos(HP_TOPLEFT);
	hv->setAnchorPos(HP_BOTTOMRIGHT);
	hv->setVisible(true);
	hv->raise();

	connect(m_deletePlotHover, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestDeletePlot(); });

}

void TimePlotComponentSettings::showDeleteButtons(bool b)
{
	m_deletePlot->setVisible(b);
	m_deletePlotHover->setVisible(b);
}

TimePlotComponentSettings::~TimePlotComponentSettings() {}

void TimePlotComponentSettings::addChannel(ChannelComponent *c)
{
	// https://stackoverflow.com/questions/44501171/qvariant-with-custom-class-pointer-does-not-return-same-address
	m_xAxisSrc->combo()->addItem(c->name(), QVariant::fromValue(static_cast<void *>(c)));
	m_autoscaler->addChannels(c->plotChannelCmpt()->m_timePlotCh);
	ScaleProvider *sp = dynamic_cast<ScaleProvider *>(c);
	if(sp) {
		m_scaleProviders.append(sp);
		updateYModeCombo();
	}

	m_curve->addChannels(c->plotChannelCmpt()->m_timePlotCh);
	m_curve->addChannels(c->plotChannelCmpt()->m_xyPlotCh);

	m_channels.append(c);
}

void TimePlotComponentSettings::removeChannel(ChannelComponent *c)
{
	m_channels.removeAll(c);
	int comboId = m_xAxisSrc->combo()->findData(QVariant::fromValue(static_cast<void *>(c)));
	m_xAxisSrc->combo()->removeItem(comboId);
	m_autoscaler->removeChannels(c->plotChannelCmpt()->m_timePlotCh);
	ScaleProvider *sp = dynamic_cast<ScaleProvider *>(c);
	if(sp) {
		m_scaleProviders.removeAll(sp);
		updateYModeCombo();
	}
	m_curve->removeChannels(c->plotChannelCmpt()->m_timePlotCh);
	m_curve->removeChannels(c->plotChannelCmpt()->m_xyPlotCh);
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

void TimePlotComponentSettings::updateYModeCombo()
{
	bool scaleItemCbtmp = true;
	for(ScaleProvider *s : qAsConst(m_scaleProviders)) {
		if(s->scaleAvailable() == false) {
			scaleItemCbtmp = false;
			break;
		}
	}

	if(scaleItemCbtmp) {
		// need scale item
		int idx = m_yModeCb->combo()->findData(YMODE_SCALE);
		if(idx == -1) {
			m_yModeCb->combo()->addItem("Scale", YMODE_SCALE);
		}

	} else {
		// no need
		int idx = m_yModeCb->combo()->findData(YMODE_SCALE);
		if(idx) {
			m_yModeCb->combo()->removeItem(idx);
		}
	}
}

void TimePlotComponentSettings::updateYAxis()
{
	double max = -1000000.0;
	double min = 1000000.0;
	for(ScaleProvider *s : qAsConst(m_scaleProviders)) {
		if(s->yMax() > max) {
			max = s->yMax();
		}
		if(s->yMin() < min) {
			min = s->yMin();
		}
	}
	m_yCtrl->setMin(min);
	m_yCtrl->setMax(max);
}
