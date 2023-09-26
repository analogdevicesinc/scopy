#include "cursor.h"
#include "plot_line_handle.h"

using namespace scopy;


Cursor::Cursor(PlotWidget *p, PlotAxis *ax, QPen pen)
	: QObject(p),
	m_plotWidget(p),
	m_axis(ax),
	m_plot(p->plot()),
	m_pen(pen),
	m_canLeavePlot(false)
{

}

Cursor::~Cursor()
{

}

void Cursor::setCanLeavePlot(bool leave)
{
	m_canLeavePlot = leave;
}

PlotLineHandle *Cursor::cursorHandle() const {
	return m_cursorHandle;
}

void Cursor::setAxis(PlotAxis *ax) {
	m_axis = ax;
}

void Cursor::setVisible(bool visible)
{
	m_cursorHandle->setVisible(visible);
	m_cursorBar->setVisible(visible);
}

bool Cursor::isVisible()
{
	return m_cursorHandle->isVisible() || m_cursorBar->isVisible();
}
