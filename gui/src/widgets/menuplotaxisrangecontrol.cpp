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
	QString unit = m_plotAxis->getUnits();
	m_min = new PositionSpinButton(
		{
			{"" + unit, 1e0},
			{"k" + unit, 1e3},
			{"M" + unit, 1e6},
			{"G" + unit, 1e9},
		},
		"Min", -DBL_MAX, DBL_MAX, false, false, this);

	m_max = new PositionSpinButton(
		{
			{"" + unit, 1e0},
			{"k" + unit, 1e3},
			{"M" + unit, 1e6},
			{"G" + unit, 1e9},
		},
		"Max", -DBL_MAX, DBL_MAX, false, false, this);

	minMaxLayout->addWidget(m_min);
	minMaxLayout->addWidget(m_max);
	// Connects
	connect(m_min, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_min, &PositionSpinButton::valueChanged, this,
		[=](double) { Q_EMIT intervalChanged(m_min->value(), m_max->value()); });
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=]() {
		QSignalBlocker b(m_min);
		m_min->setValue(m_plotAxis->min());
		Q_EMIT intervalChanged(m_min->value(), m_max->value());
	});

	connect(m_max, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
	connect(m_max, &PositionSpinButton::valueChanged, this,
		[=](double) { Q_EMIT intervalChanged(m_min->value(), m_max->value()); });
	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=]() {
		QSignalBlocker b(m_max);
		m_max->setValue(m_plotAxis->max());
		Q_EMIT intervalChanged(m_min->value(), m_max->value());
	});
}

MenuPlotAxisRangeControl::~MenuPlotAxisRangeControl() {}

double MenuPlotAxisRangeControl::min() { return m_min->value(); }

double MenuPlotAxisRangeControl::max() { return m_max->value(); }

void MenuPlotAxisRangeControl::setMin(double val)
{
	m_min->setValue(val);
	Q_EMIT intervalChanged(val, m_max->value());
}

void MenuPlotAxisRangeControl::setMax(double val)
{
	m_max->setValue(val);
	Q_EMIT intervalChanged(m_min->value(), val);
}

#include "moc_menuplotaxisrangecontrol.cpp"
