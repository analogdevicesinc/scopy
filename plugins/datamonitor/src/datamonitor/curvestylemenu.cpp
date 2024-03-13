#include "curvestylemenu.hpp"

#include <QBoxLayout>
#include <menucollapsesection.h>
#include <menucombo.h>
#include <menusectionwidget.h>

using namespace scopy;

CurveStyleMenu::CurveStyleMenu(QWidget *parent)
	: QWidget{parent}
{
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	this->setLayout(layout);

	MenuSectionWidget *curvecontainer = new MenuSectionWidget(parent);
	layout->addWidget(curvecontainer);

	curvecontainer->installEventFilter(curvecontainer);
	MenuCollapseSection *curve = new MenuCollapseSection("CURVE", MenuCollapseSection::MHCW_NONE, curvecontainer);

	QWidget *curveSettings = new QWidget(curve);
	QHBoxLayout *curveSettingsLay = new QHBoxLayout(curveSettings);
	curveSettingsLay->setMargin(0);
	curveSettingsLay->setSpacing(10);
	curveSettings->setLayout(curveSettingsLay);

	MenuCombo *cbThicknessW = new MenuCombo("Thickness", curve);
	auto cbThickness = cbThicknessW->combo();
	cbThickness->addItem("1");
	cbThickness->addItem("2");
	cbThickness->addItem("3");
	cbThickness->addItem("4");
	cbThickness->addItem("5");

	cbThicknessW->combo()->installEventFilter(this);

	connect(cbThickness, qOverload<int>(&QComboBox::currentIndexChanged), this,
		[=](int idx) { Q_EMIT curveThicknessChanged(cbThickness->itemText(idx).toFloat()); });

	MenuCombo *cbStyleW = new MenuCombo("Style", curve);
	auto cbStyle = cbStyleW->combo();
	cbStyle->addItem("Lines", PlotChannel::PCS_LINES);
	cbStyle->addItem("Dots", PlotChannel::PCS_DOTS);
	cbStyle->addItem("Steps", PlotChannel::PCS_STEPS);
	cbStyle->addItem("Sticks", PlotChannel::PCS_STICKS);
	cbStyle->addItem("Smooth", PlotChannel::PCS_SMOOTH);
	StyleHelper::MenuComboBox(cbStyle, "cbStyle");

	connect(cbStyle, qOverload<int>(&QComboBox::currentIndexChanged), this,
		&CurveStyleMenu::curveStyleIndexChanged);

	curveSettingsLay->addWidget(cbThicknessW);
	curveSettingsLay->addWidget(cbStyleW);
	curve->contentLayout()->addWidget(curveSettings);
	curvecontainer->contentLayout()->addWidget(curve);
}
