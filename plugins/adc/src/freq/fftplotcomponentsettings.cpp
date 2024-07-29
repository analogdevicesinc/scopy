#include "fftplotcomponentsettings.h"
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <QWidget>
#include <QLineEdit>
#include "fftplotcomponentchannel.h"

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
		&FFTPlotComponent::showPlotLabels);

	MenuSectionCollapseWidget *yaxis =
		new MenuSectionCollapseWidget("Y-AXIS", MenuCollapseSection::MHCW_NONE, parent);

	m_yCtrl = new MenuPlotAxisRangeControl(m_plotComponent->fftPlot()->yAxis(), this);
	m_yPwrOffset = new PositionSpinButton(
		{ {"db", 1e0} },
		"Power Offset", -200, 200, false, false, yaxis);

	m_curve = new MenuPlotChannelCurveStyleControl(plotMenu);

	m_deletePlot = new QPushButton("DELETE PLOT");
	StyleHelper::BlueButton(m_deletePlot);
	connect(m_deletePlot, &QAbstractButton::clicked, this, [=]() { Q_EMIT requestDeletePlot(); });

	yaxis->contentLayout()->addWidget(m_yCtrl);
	yaxis->contentLayout()->addWidget(m_yPwrOffset);

	plotMenu->contentLayout()->addWidget(plotTitleLabel);
	plotMenu->contentLayout()->addWidget(plotTitle);
	plotMenu->contentLayout()->addWidget(labelsSwitch);
	plotMenu->contentLayout()->addWidget(m_curve);
	plotMenu->contentLayout()->setSpacing(10);

	v->setSpacing(10);
	v->addWidget(yaxis);
	v->addWidget(plotMenu);
	v->addWidget(m_deletePlot);
	v->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Expanding));

	m_yCtrl->setVisible(true);

	m_yCtrl->setMin(-140);
	m_yCtrl->setMax(20);
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

void FFTPlotComponentSettings::showDeleteButtons(bool b)
{
	m_deletePlot->setVisible(b);
	m_deletePlotHover->setVisible(b);
}


FFTPlotComponentSettings::~FFTPlotComponentSettings() {}

void FFTPlotComponentSettings::addChannel(ChannelComponent *c)
{
	// https://stackoverflow.com/questions/44501171/qvariant-with-custom-class-pointer-does-not-return-same-address

	auto fftPlotComponentChannel = dynamic_cast<FFTPlotComponentChannel*>(c->plotChannelCmpt());
	m_curve->addChannels(fftPlotComponentChannel->plotChannel());

	if(dynamic_cast<FFTChannel*>(c)) {
		FFTChannel* fc = dynamic_cast<FFTChannel*>(c);
		connections[c] << connect(m_yPwrOffset, &PositionSpinButton::valueChanged, c, [=](double val){
			fc->setPowerOffset(val);
		});
		fc->setPowerOffset(m_yPwrOffset->value());

	}
	m_channels.append(c);
}

void FFTPlotComponentSettings::removeChannel(ChannelComponent *c)
{
	m_channels.removeAll(c);

	auto fftPlotComponentChannel = dynamic_cast<FFTPlotComponentChannel*>(c->plotChannelCmpt());
	m_curve->removeChannels(fftPlotComponentChannel->plotChannel());

	for(const QMetaObject::Connection &c : qAsConst(connections[c])) {
		QObject::disconnect(c);
	}
	connections.remove(c);

}
