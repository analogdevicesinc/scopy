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

	addAxis(m_plotAxis);
	minMaxLayout->addWidget(m_min);
	minMaxLayout->addWidget(m_max);

}


void MenuPlotAxisRangeControl::addAxis(PlotAxis *ax) {
	// Connects

	if(connections.contains(ax))
		return;

	connections[ax] << connect(m_min, &PositionSpinButton::valueChanged, ax, &PlotAxis::setMin);
	connections[ax] << connect(m_min, &PositionSpinButton::valueChanged, this,
		[=](double) { Q_EMIT intervalChanged(m_min->value(), m_max->value()); });
	connections[ax] << connect(ax, &PlotAxis::minChanged, this, [=]() {
		QSignalBlocker b(m_min);
		m_min->setValue(ax->min());
		Q_EMIT intervalChanged(m_min->value(), m_max->value());
	});

	connections[ax] << connect(m_max, &PositionSpinButton::valueChanged, ax, &PlotAxis::setMax);
	connections[ax] << connect(m_max, &PositionSpinButton::valueChanged, this,
		[=](double) { Q_EMIT intervalChanged(m_min->value(), m_max->value()); });
	connections[ax] << connect(ax, &PlotAxis::maxChanged, this, [=]() {
		QSignalBlocker b(m_max);
		m_max->setValue(ax->max());
		Q_EMIT intervalChanged(m_min->value(), m_max->value());
	});
}

void MenuPlotAxisRangeControl::removeAxis(PlotAxis *ax) {
	for(const QMetaObject::Connection &c : qAsConst(connections[ax])) {
		QObject::disconnect(c);
	}
	connections.remove(ax);
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
