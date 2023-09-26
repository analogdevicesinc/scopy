#include "hcursor.h"

using namespace scopy;


HCursor::HCursor(PlotWidget *p, PlotAxis *ax,bool bottom, QPen pen) :
	Cursor(p, ax, pen)
{
	m_symbolCtrl = p->symbolCtrl();
	m_cursorBar = new VertBar(p->plot());
	auto hCursorBar = static_cast<VertBar*>(m_cursorBar);
	m_symbolCtrl->attachSymbol(m_cursorBar);

	m_cursorBar->setCanLeavePlot(m_canLeavePlot);
	m_cursorBar->setVisible(true);
	m_cursorBar->setMobileAxis(m_axis->axisId());
	m_cursorBar->setPen(pen);

	m_cursorHandle = new PlotLineHandleH(
		QPixmap(":/gui/icons/h_cursor_handle.svg"),
		(bottom) ? p->bottomHandlesArea() : p->topHandlesArea(), !bottom);

	auto m_hCursorHandle = static_cast<PlotLineHandleH*>(m_cursorHandle);
	m_hCursorHandle->setPen(pen);
	m_hCursorHandle->setVisible(true);

	/* When bar position changes due to plot resizes update the handle */
	connect(p->rightHandlesArea(), &HandlesArea::sizeChanged, m_cursorHandle, [=](){
		m_hCursorHandle->updatePosition();
	});

	connect(hCursorBar, &VertBar::pixelPositionChanged, this,
			[=](int pos) {
				m_hCursorHandle->setPositionSilenty(pos);
			});

	connect(m_cursorHandle, &PlotLineHandleH::positionChanged, this,
			[=](int pos) {
				double new_pos = computePosition(pos);
				if (!m_canLeavePlot) {
					new_pos = std::max(std::min(m_axis->max(), new_pos), m_axis->min());
				}
				hCursorBar->setPosition(new_pos);
				Q_EMIT positionChanged(new_pos);
			});

	connect(this, &HCursor::update, m_hCursorHandle, &PlotLineHandleH::triggerMove);
}

HCursor::~HCursor()
{

}

void HCursor::setCanLeavePlot(bool leave)
{
	m_canLeavePlot = leave;
	m_cursorBar->setCanLeavePlot(m_canLeavePlot);
}

double HCursor::getPosition()
{
	return static_cast<VertBar*>(m_cursorBar)->getPosition();
}

void HCursor::setPosition(double pos)
{
	// cursors cannot be outside of the axis
	if (!m_canLeavePlot) {
		pos = std::max(std::min(m_axis->max(), pos), m_axis->min());
	}
	static_cast<VertBar*>(m_cursorBar)->setPosition(pos);
}

double HCursor::computePosition(int pos) {
	QwtScaleMap xMap = m_plot->canvasMap(m_axis->axisId());
	double offset = xMap.invTransform(pos);
	return offset;
}
