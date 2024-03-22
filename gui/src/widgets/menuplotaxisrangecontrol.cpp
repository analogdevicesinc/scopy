#include "menuplotaxisrangecontrol.h"
#include <cfloat>

using namespace scopy::gui;
MenuPlotAxisRangeControl::MenuPlotAxisRangeControl(PlotAxis *m_plotAxis, QWidget *parent)
	: QWidget(parent)
{
	// Y-MIN-MAX
	QHBoxLayout *minMaxLayout = new QHBoxLayout(this);
	setLayout(minMaxLayout);
	minMaxLayout->setMargin(0);
	minMaxLayout->setSpacing(10);

	m_min = new PositionSpinButton(
		{
			{"V", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"Min", -DBL_MAX, DBL_MAX, false, false, this);

	m_max = new PositionSpinButton(
		{
			{"V", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"Max", -DBL_MAX, DBL_MAX, false, false, this);

	minMaxLayout->addWidget(m_min);
	minMaxLayout->addWidget(m_max);
	// Connects
	connect(m_min, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=]() {
		QSignalBlocker b(m_min);
		m_min->setValue(m_plotAxis->min());
	});

	connect(m_max, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=]() {
		QSignalBlocker b(m_max);
		m_max->setValue(m_plotAxis->max());
	});
}

MenuPlotAxisRangeControl::~MenuPlotAxisRangeControl() {}

void MenuPlotAxisRangeControl::setMin(double val) { m_min->setValue(val); }

void MenuPlotAxisRangeControl::setMax(double val) { m_max->setValue(val); }

#include "moc_menuplotaxisrangecontrol.cpp"
