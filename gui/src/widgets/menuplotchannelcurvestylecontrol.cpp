#include "plotchannel.h"
#include "menuplotchannelcurvestylecontrol.h"
#include <menusectionwidget.h>
#include <menucollapsesection.h>

using namespace scopy::gui;

MenuPlotChannelCurveStyleControl::MenuPlotChannelCurveStyleControl(QWidget *parent)
	: QWidget(parent)
{
	createCurveMenu(this);
}

MenuPlotChannelCurveStyleControl::~MenuPlotChannelCurveStyleControl() {}

void MenuPlotChannelCurveStyleControl::createCurveMenu(QWidget *parent)
{
	QHBoxLayout *curveSettingsLay = new QHBoxLayout(this);
	curveSettingsLay->setMargin(0);
	curveSettingsLay->setSpacing(10);
	setLayout(curveSettingsLay);

	cbThicknessW = new MenuCombo("Thickness", this);
	QComboBox *cbThickness = cbThicknessW->combo();
	cbThickness->addItem("1");
	cbThickness->addItem("2");
	cbThickness->addItem("3");
	cbThickness->addItem("4");
	cbThickness->addItem("5");

	connect(cbThickness, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		for(auto ch : qAsConst(m_channels)) {
			ch->setThickness(cbThickness->itemText(idx).toFloat());
		}
	});

	cbStyleW = new MenuCombo("Style", this);
	auto cbStyle = cbStyleW->combo();
	cbStyle->addItem("Lines", PlotChannel::PCS_LINES);
	cbStyle->addItem("Dots", PlotChannel::PCS_DOTS);
	cbStyle->addItem("Steps", PlotChannel::PCS_STEPS);
	cbStyle->addItem("Sticks", PlotChannel::PCS_STICKS);
	cbStyle->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	StyleHelper::MenuComboBox(cbStyle, "cbStyle");

	connect(cbStyle, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		for(auto ch : qAsConst(m_channels)) {
			ch->setStyle(cbStyle->itemData(idx).toInt());
		}
	});

	curveSettingsLay->addWidget(cbThicknessW);
	curveSettingsLay->addWidget(cbStyleW);
}

void MenuPlotChannelCurveStyleControl::addChannels(PlotChannel *c)
{
	c->setThickness(cbThicknessW->combo()->currentText().toInt());
	c->setStyle(cbStyleW->combo()->currentIndex());

	connect(c, &PlotChannel::styleChanged, this, &MenuPlotChannelCurveStyleControl::setStyleSlot);
	connect(c, &PlotChannel::thicknessChanged, this, &MenuPlotChannelCurveStyleControl::setThicknessSlot);

	m_channels.append(c);
}

void MenuPlotChannelCurveStyleControl::removeChannels(PlotChannel *c) {
	disconnect(c, &PlotChannel::styleChanged, this, &MenuPlotChannelCurveStyleControl::setStyleSlot);
	disconnect(c, &PlotChannel::thicknessChanged, this, &MenuPlotChannelCurveStyleControl::setThicknessSlot);
	m_channels.removeAll(c);
}

void MenuPlotChannelCurveStyleControl::setStyleSlot()
{
	if(m_channels.count() <= 0)
		return;

	int style = m_channels[0]->style();
	for(PlotChannel *c : qAsConst(m_channels)) {
		if(style != c->style())	{
			// "Mixed style should be written here"
			return;
		}
	}

	cbStyleW->combo()->setCurrentIndex(cbStyleW->combo()->findData(style));

}

void MenuPlotChannelCurveStyleControl::setThicknessSlot()
{
	if(m_channels.count() <= 0)
		return;

	int thickness = m_channels[0]->thickness();
	for(PlotChannel *c : qAsConst(m_channels)) {
		if(thickness != c->thickness())	{
			// "Mixed thickness should be written here"
			return;
		}
	}

	qInfo()<<m_channels.count() << thickness;
	cbThicknessW->combo()->setCurrentText(QString::number(thickness));
}



#include "moc_menuplotchannelcurvestylecontrol.cpp"
