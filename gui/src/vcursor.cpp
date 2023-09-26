#include "vcursor.h"

using namespace scopy;


VCursor::VCursor(PlotWidget *p, PlotAxis *ax, bool right, QPen pen) :
	Cursor(p, ax, pen)
{
	m_symbolCtrl = p->symbolCtrl();
	m_cursorBar = new HorizBar(p->plot());
	auto hCursorBar = static_cast<HorizBar*>(m_cursorBar);
	m_symbolCtrl->attachSymbol(m_cursorBar);

	m_cursorBar->setCanLeavePlot(!m_canLeavePlot);
	m_cursorBar->setVisible(true);
	m_cursorBar->setMobileAxis(m_axis->axisId());
	m_cursorBar->setPen(pen);

	m_cursorHandle = new PlotLineHandleV(
		QPixmap(":/gui/icons/v_cursor_handle.svg"),
		(right) ? p->rightHandlesArea() : p->leftHandlesArea(), !right);

	auto m_hCursorHandle = static_cast<PlotLineHandleV*>(m_cursorHandle);
	m_hCursorHandle->setPen(pen);
	m_hCursorHandle->setVisible(true);

	/* When bar position changes due to plot resizes update the handle */
	connect(p->rightHandlesArea(), &HandlesArea::sizeChanged, m_cursorHandle, [=](){
		m_hCursorHandle->updatePosition();
	});

	connect(hCursorBar, &HorizBar::pixelPositionChanged, this,
			[=](int pos) {
				m_hCursorHandle->setPositionSilenty(pos);
			});

	connect(m_cursorHandle, &PlotLineHandleV::positionChanged, this,
			[=](int pos) {
				double new_pos = computePosition(pos);
				if (!m_canLeavePlot) {
					new_pos = std::max(std::min(m_axis->max(), new_pos), m_axis->min());
				}
				hCursorBar->setPosition(new_pos);
				Q_EMIT positionChanged(new_pos);
			});

	connect(this, &VCursor::update, m_hCursorHandle, &PlotLineHandleV::triggerMove);
}

VCursor::~VCursor()
{

}

void VCursor::setCanLeavePlot(bool leave)
{
	m_canLeavePlot = leave;
	m_cursorBar->setCanLeavePlot(m_canLeavePlot);
}

double VCursor::getPosition()
{
	return static_cast<HorizBar*>(m_cursorBar)->getPosition();
}

void VCursor::setPosition(double pos)
{
	// cursors cannot be outside of the axis
	pos = std::max(std::min(m_axis->max(), pos), m_axis->min());
	static_cast<HorizBar*>(m_cursorBar)->setPosition(pos);
}

double VCursor::computePosition(int pos) {
	QwtScaleMap yMap = m_plot->canvasMap(m_axis->axisId());
	double offset = yMap.invTransform(pos);
	return offset;
}
