#include "plotcursors.h"

using namespace scopy;


Cursor::Cursor(PlotWidget *p, PlotAxis *ax, QPen pen)
	: QObject(p),
	m_plotWidget(p),
	m_axis(ax),
	m_plot(p->plot()),
	m_pen(pen) {
}

Cursor::~Cursor() {
}

PlotLineHandle *Cursor::cursorHandle() const {
	return m_cursorHandle;
}

void Cursor::setAxis(PlotAxis *ax) {
	m_axis = ax;
}

void Cursor::setVisible(bool b)
{
	m_cursorHandle->setVisible(b);
	m_cursorBar->setVisible(b);
}

VCursor::VCursor(PlotWidget *p, PlotAxis *ax, bool right, QPen pen) : Cursor(p, ax, pen) {
	m_symbolCtrl = p->symbolCtrl();
	m_cursorBar = new HorizBar(p->plot());
	auto hCursorBar = static_cast<HorizBar*>(m_cursorBar);
	m_symbolCtrl->attachSymbol(m_cursorBar);

	m_cursorBar->setCanLeavePlot(true);
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
				double offset = computePosition(pos);
				hCursorBar->setPosition(offset);
				Q_EMIT positionChanged(offset);
			});
}

VCursor::~VCursor() {

}

double VCursor::computePosition(int pos) {
	QwtScaleMap yMap = m_plot->canvasMap(m_axis->axisId());
	double offset = yMap.invTransform(pos);
	return offset;
}


HCursor::HCursor(PlotWidget *p, PlotAxis *ax,bool bottom, QPen pen) : Cursor(p, ax, pen) {
	m_symbolCtrl = p->symbolCtrl();
	m_cursorBar = new VertBar(p->plot());
	auto hCursorBar = static_cast<VertBar*>(m_cursorBar);
	m_symbolCtrl->attachSymbol(m_cursorBar);

	m_cursorBar->setCanLeavePlot(true);
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
				double offset = computePosition(pos);
				hCursorBar->setPosition(offset);
				Q_EMIT positionChanged(offset);
			});
}

HCursor::~HCursor() {

}

double HCursor::computePosition(int pos) {
	QwtScaleMap xMap = m_plot->canvasMap(m_axis->axisId());
	double offset = xMap.invTransform(pos);
	return offset;
}
