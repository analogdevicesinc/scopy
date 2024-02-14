#include "time_ycontrol.h"

using namespace scopy::grutil;
TimeYControl::TimeYControl(PlotAxis *m_plotAxis, QWidget *parent)
	: QWidget(parent)
{
	// Y-MIN-MAX
	QHBoxLayout *yMinMaxLayout = new QHBoxLayout(this);
	setLayout(yMinMaxLayout);
	yMinMaxLayout->setMargin(0);
	yMinMaxLayout->setSpacing(10);

	m_ymin = new PositionSpinButton(
		{
			{"V", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"YMin", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, this);

	m_ymax = new PositionSpinButton(
		{
			{"V", 1e0},
			{"k", 1e3},
			{"M", 1e6},
			{"G", 1e9},
		},
		"YMax", (double)((long)(-1 << 31)), (double)((long)1 << 31), false, false, this);

	yMinMaxLayout->addWidget(m_ymin);
	yMinMaxLayout->addWidget(m_ymax);
	// Connects
	connect(m_ymin, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMin);
	connect(m_plotAxis, &PlotAxis::minChanged, this, [=]() {
		QSignalBlocker b(m_ymin);
		m_ymin->setValue(m_plotAxis->min());
	});

	connect(m_ymax, &PositionSpinButton::valueChanged, m_plotAxis, &PlotAxis::setMax);
	connect(m_plotAxis, &PlotAxis::maxChanged, this, [=]() {
		QSignalBlocker b(m_ymax);
		m_ymax->setValue(m_plotAxis->max());
	});
}

TimeYControl::~TimeYControl() {}

void TimeYControl::setMin(double val) { m_ymin->setValue(val); }

void TimeYControl::setMax(double val) { m_ymax->setValue(val); }
