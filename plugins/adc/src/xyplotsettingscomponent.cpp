#include "xyplotsettingscomponent.h"
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <gui/stylehelper.h>

namespace scopy {
namespace adc {

XyPlotSettingsComponent::XyPlotSettingsComponent(PlotComponent *plot, QWidget *parent)
	: QWidget(parent)
	, m_syncMode(false)
	, m_singleYMode(false)
	, m_xChannel(nullptr)

{
	m_plot = plot->plot();
	auto *w = createMenu(this);
	QVBoxLayout *lay = new QVBoxLayout(parent);
	lay->addWidget(w);
	lay->setSpacing(0);
	lay->setMargin(0);
	setLayout(lay);

}

XyPlotSettingsComponent::~XyPlotSettingsComponent() {}

QWidget *XyPlotSettingsComponent::createMenu(QWidget *parent)
{
	QScrollArea *scroll = new QScrollArea(parent);
	scroll->setWidgetResizable(true);
	QWidget *w = new QWidget(scroll);
	scroll->setWidget(w);
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	m_pen = QPen(StyleHelper::getColor("ScopyBlue"));

	MenuHeaderWidget *header = new MenuHeaderWidget("XY-PLOT", m_pen, w);
	QWidget *xaxismenu = createXAxisMenu(w);
	QWidget *yaxismenu = createYAxisMenu(w);
	//	QWidget* curvemenu = createCurveMenu(w);

	lay->addWidget(header);
	lay->addWidget(xaxismenu);
	lay->addWidget(yaxismenu);
	//	lay->addWidget(curvemenu);

	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return scroll;
}

QWidget *XyPlotSettingsComponent::createYAxisMenu(QWidget *parent)
{
	MenuSectionWidget *yaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *yaxis = new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, yaxiscontainer);

	m_plot->yAxis()->setUnits("V");
	m_yctrl = new MenuPlotAxisRangeControl(m_plot->yAxis(), yaxis);
	m_singleYModeSw = new MenuOnOffSwitch("Single Y Mode", yaxis);
	m_autoscaleBtn = new QPushButton("Autoscale", yaxis);

	y_autoscaler = new PlotAutoscaler(false, this);
	connect(y_autoscaler, &PlotAutoscaler::newMin, m_yctrl, &MenuPlotAxisRangeControl::setMin);
	connect(y_autoscaler, &PlotAutoscaler::newMax, m_yctrl, &MenuPlotAxisRangeControl::setMax);
	StyleHelper::BlueButton(m_autoscaleBtn, "autoscale");

	connect(m_autoscaleBtn, &QPushButton::clicked, this, [=]() { y_autoscaler->autoscale(); });

	yaxis->contentLayout()->addWidget(m_singleYModeSw);
	yaxis->contentLayout()->addWidget(m_yctrl);
	yaxis->contentLayout()->addSpacerItem(new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed));
	yaxis->contentLayout()->addWidget(m_autoscaleBtn);
	yaxiscontainer->contentLayout()->addWidget(yaxis);

	connect(m_singleYModeSw->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_yctrl->setEnabled(b);
		m_autoscaleBtn->setEnabled(b);
		setSingleYMode(b);
	});

	return yaxiscontainer;
}

QWidget *XyPlotSettingsComponent::createXAxisMenu(QWidget *parent)
{
	MenuSectionWidget *xaxiscontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *xaxis = new MenuCollapseSection("X-AXIS", MenuCollapseSection::MHCW_NONE, xaxiscontainer);

	m_bufferSizeSpin = new ScaleSpinButton(
		{
		 {"samples", 1e0},
		 {"ksamples", 1e3},
		 {"Msamples", 1e6},
		 },
		"Buffer Size", 16, DBL_MAX, false, false, this);

	connect(m_bufferSizeSpin, &ScaleSpinButton::valueChanged, this, [=](double val) {
		setBufferSize((uint32_t)val);
	});

	xaxis->contentLayout()->addWidget(m_bufferSizeSpin);

	m_xctrl = new MenuPlotAxisRangeControl(m_plot->xAxis(), xaxis);
	xaxis->contentLayout()->addWidget(m_xctrl);

	x_autoscaler = new PlotAutoscaler(true, this);
	connect(x_autoscaler, &PlotAutoscaler::newMin, m_xctrl, &MenuPlotAxisRangeControl::setMin);
	connect(x_autoscaler, &PlotAutoscaler::newMax, m_xctrl, &MenuPlotAxisRangeControl::setMax);
	connect(m_autoscaleBtn, &QPushButton::clicked, this, [=]() { x_autoscaler->autoscale(); });

	m_xChannelCb = new MenuCombo("X-Channel", this);
	connect(m_xChannelCb->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx){
		ChannelComponent *ch;
		QString name = m_xChannelCb->combo()->itemData(idx).toString();
		for(ChannelComponent *c : m_channels) {
			if(c->name() == name) {
				ch = c;
			}
		}
		setXChannel(ch);
	});

	m_showLabels = new MenuOnOffSwitch("PLOT LABELS", xaxis);
	showPlotLabels(false);
	connect(m_showLabels->onOffswitch(), &QAbstractButton::toggled, this,
		&XyPlotSettingsComponent::showPlotLabels);

	xaxiscontainer->contentLayout()->setSpacing(10);
	xaxiscontainer->contentLayout()->addWidget(xaxis);
	xaxis->contentLayout()->addWidget(m_xChannelCb);
	xaxis->contentLayout()->addWidget(m_showLabels);

	xaxis->contentLayout()->setSpacing(10);

	return xaxiscontainer;
}

void XyPlotSettingsComponent::onInit() {
	m_bufferSizeSpin->setValue(32);
	m_showLabels->onOffswitch()->setChecked(false);
	m_yctrl->setEnabled(false);
	m_singleYModeSw->setEnabled(true);
	m_singleYModeSw->onOffswitch()->setChecked(false);
	m_autoscaleBtn->setEnabled(false);
}

void XyPlotSettingsComponent::showPlotLabels(bool b)
{
	m_plot->setShowXAxisLabels(b);
	m_plot->setShowYAxisLabels(b);
	m_plot->showAxisLabels();
}

bool XyPlotSettingsComponent::singleYMode() const { return m_singleYMode; }

void XyPlotSettingsComponent::setSingleYMode(bool newSingleYMode)
{
	if(m_singleYMode == newSingleYMode)
		return;
	m_singleYMode = newSingleYMode;
	Q_EMIT singleYModeChanged(newSingleYMode);
}


uint32_t XyPlotSettingsComponent::bufferSize() const { return m_bufferSize; }

void XyPlotSettingsComponent::setBufferSize(uint32_t newBufferSize)
{
	if(m_bufferSize == newBufferSize)
		return;
	m_bufferSize = newBufferSize;
	Q_EMIT bufferSizeChanged(newBufferSize);
}

void XyPlotSettingsComponent::onStart()
{
	Q_EMIT singleYModeChanged(m_singleYMode);
	if(!m_syncMode) {
		Q_EMIT bufferSizeChanged(m_bufferSize);
	}
}


void XyPlotSettingsComponent::addChannel(ChannelComponent *c) {
	m_channels.append(c);
	m_xChannelCb->combo()->addItem(c->name(), c->name());
	y_autoscaler->addChannels(c->plotCh());
}

void XyPlotSettingsComponent::removeChannel(ChannelComponent *c) {
	m_channels.removeAll(c);
	m_xChannelCb->combo()->removeItem(m_xChannelCb->combo()->findData(c->name()));
	y_autoscaler->removeChannels(c->plotCh());
}

ChannelComponent *XyPlotSettingsComponent::xChannel() const
{
	return m_xChannel;
}

void XyPlotSettingsComponent::setXChannel(ChannelComponent *newXChannel)
{
	if (m_xChannel == newXChannel)
		return;
	m_xChannel = newXChannel;
	Q_EMIT xChannelChanged(newXChannel);
}


} // namespace adc
} // namespace scopy
