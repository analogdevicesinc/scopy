#include "plotaxishandle.h"
#include "plotaxis.h"
#include <qwt_scale_map.h>

using namespace scopy;

PlotAxisHandle::PlotAxisHandle(PlotWidget *plot, PlotAxis *ax)
	: QWidget(plot)
	, m_plotWidget(plot)
	, m_axis(ax)
	, m_plot(plot->plot())
{
	init();
}

PlotAxisHandle::~PlotAxisHandle()
{
	if(m_handle != nullptr) {
		delete m_handle;
	}
}

void PlotAxisHandle::init()
{
	m_handle = new AxisHandle(m_axis->axisId(), HandlePos::SOUTH_OR_EAST, m_plot);
	connect(m_plot, &QObject::destroyed, this, [=]() { m_handle = nullptr; });
	m_pos = pixelToScale(m_handle->getPos());

	connect(m_plotWidget, &PlotWidget::canvasSizeChanged, this, &PlotAxisHandle::updatePos);
	connect(m_plotWidget, &PlotWidget::plotScaleChanged, this, &PlotAxisHandle::updatePos);
	connect(m_axis, &PlotAxis::axisScaleUpdated, this, &PlotAxisHandle::updatePos);

	connect(this, &PlotAxisHandle::updatePos, this, [=]() {
		if(scaleToPixel(m_pos) != m_handle->getPos()) {
			setPositionSilent(m_pos);
		}
	});

	connect(m_handle, &AxisHandle::pixelPosChanged, this, [=](int pos) {
		if(pos != scaleToPixel(m_pos)) {
			Q_EMIT scalePosChanged(pixelToScale(pos));
			m_pos = pixelToScale(pos);
		}
	});
}

void PlotAxisHandle::setAxis(PlotAxis *axis)
{
	disconnect(m_axis, &PlotAxis::axisScaleUpdated, this, nullptr);
	m_axis = axis;
	connect(m_axis, &PlotAxis::axisScaleUpdated, this, &PlotAxisHandle::updatePos);
	m_handle->setAxis(axis->axisId());
	Q_EMIT updatePos();
}

PlotAxis *PlotAxisHandle::axis() const { return m_axis; }

AxisHandle *PlotAxisHandle::handle() const { return m_handle; }

double PlotAxisHandle::getPosition() const { return m_pos; }

void PlotAxisHandle::setPosition(double pos)
{
	setPositionSilent(pos);
	Q_EMIT scalePosChanged(pos);
}

void PlotAxisHandle::setPositionSilent(double pos)
{
	m_pos = pos;
	m_handle->setPosSilent(scaleToPixel(pos));
}

double PlotAxisHandle::pixelToScale(int pos)
{
	QwtScaleMap map = m_plot->canvasMap(m_axis->axisId());
	return map.invTransform(pos);
}

int PlotAxisHandle::scaleToPixel(double pos)
{
	QwtScaleMap map = m_plot->canvasMap(m_axis->axisId());
	return map.transform(pos);
}

#include "moc_plotaxishandle.cpp"
