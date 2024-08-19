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

	m_min = new MenuSpinbox("Min", 0, "counts", -1e9, 1e9, true, false, this);
	m_max = new MenuSpinbox("Max", 0, "counts", -1e9, 1e9, true, false, this);

	m_min->setScaleRange(1, 1e9);
	m_max->setScaleRange(1, 1e9);

	addAxis(m_plotAxis);
	minMaxLayout->addWidget(m_min);
	minMaxLayout->addWidget(m_max);
}

void MenuPlotAxisRangeControl::addAxis(PlotAxis *ax)
{
	// Connects

	if(connections.contains(ax))
		return;

	connections[ax] << connect(m_min, &MenuSpinbox::valueChanged, ax, &PlotAxis::setMin);
	connections[ax] << connect(m_min, &MenuSpinbox::valueChanged, this,
				   [=](double) { Q_EMIT intervalChanged(m_min->value(), m_max->value()); });
	connections[ax] << connect(ax, &PlotAxis::minChanged, this, [=]() {
		QSignalBlocker b(m_min);
		m_min->setValue(ax->min());
		Q_EMIT intervalChanged(m_min->value(), m_max->value());
	});

	connections[ax] << connect(m_max, &MenuSpinbox::valueChanged, ax, &PlotAxis::setMax);
	connections[ax] << connect(m_max, &MenuSpinbox::valueChanged, this,
				   [=](double) { Q_EMIT intervalChanged(m_min->value(), m_max->value()); });
	connections[ax] << connect(ax, &PlotAxis::maxChanged, this, [=]() {
		QSignalBlocker b(m_max);
		m_max->setValue(ax->max());
		Q_EMIT intervalChanged(m_min->value(), m_max->value());
	});
}

void MenuPlotAxisRangeControl::removeAxis(PlotAxis *ax)
{
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

MenuSpinbox *MenuPlotAxisRangeControl::minSpinbox() { return m_min; }

MenuSpinbox *MenuPlotAxisRangeControl::maxSpinbox() { return m_max; }

#include "moc_menuplotaxisrangecontrol.cpp"
