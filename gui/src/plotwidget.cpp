#include "plotwidget.h"
#include <DisplayPlot.h>

#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotLayout>
#include <QwtPlotSeriesItem>
#include <customqwtscaledraw.hpp>
#include <QMouseEvent>
#include <osc_scale_engine.h>
#include <QObject>
#include "plotaxis.h"
#include <pluginbase/preferences.h>
#include <QwtPlotOpenGLCanvas>
#include <QLabel>
#include <QDebug>
#include "DisplayPlot.h"

using namespace scopy;


PlotWidget::PlotWidget(QWidget *parent) : QWidget(parent) {

	m_selectedChannel = nullptr;
	m_plot = new QwtPlot(this);
	m_layout = new QGridLayout(this);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	setLayout(m_layout);

	QPen pen(QColor("#9E9E9F"));

	setupOpenGLCanvas();
	setupHandlesArea();

	m_xAxis = new PlotAxis(QwtAxis::XBottom,this,pen,this);
	m_yAxis = new PlotAxis(QwtAxis::YLeft,this,pen,this);
	m_yAxis->setVisible(true);
	m_xAxis->setVisible(true);

	setupAxisScales();
	setAxisScalesVisible(true);

	// Plot needs a grid
	QPen gridpen(QColor("#353537"));
	EdgelessPlotGrid *d_grid = new EdgelessPlotGrid();
	QColor majorPenColor(gridpen.color());
	d_grid->setMajorPen(majorPenColor, 1.0, Qt::DashLine);
	d_grid->attach(m_plot);

	//	QwtPlotMarker *d_origin = new QwtPlotMarker();
	//	d_origin->setLineStyle( QwtPlotMarker::Cross );
	//	d_origin->setValue( 0, 0.0 );
	//	d_origin->setLinePen( Qt::gray, 0.0, Qt::DashLine );
	//	d_origin->attach( m_plot );

	graticule = new Graticule(m_plot);
	connect(this, SIGNAL(canvasSizeChanged()),graticule,SLOT(onCanvasSizeChanged()));
	setDisplayGraticule(false);

	m_plot->plotLayout()->setAlignCanvasToScales(false);
	m_plot->plotLayout()->setCanvasMargin(0);
	m_plot->plotLayout()->setSpacing(0);
	setupZoomer();


}
void PlotWidget::setupHandlesArea() {
	m_symbolCtrl = new SymbolController(m_plot);

	/* Adjacent areas */
	m_bottomHandlesArea = new HorizHandlesArea(m_plot->canvas());
	m_rightHandlesArea = new VertHandlesArea(m_plot->canvas());
	m_topHandlesArea = new HorizHandlesArea(m_plot->canvas());
	m_leftHandlesArea = new VertHandlesArea(m_plot->canvas());

	m_leftHandlesArea->setMinimumWidth(50);	
	m_leftHandlesArea->setBottomPadding(0);
	m_leftHandlesArea->setTopPadding(0); /// Why ?
	m_leftHandlesArea->setVisible(false);

	m_rightHandlesArea->setMinimumWidth(50);
	m_rightHandlesArea->setBottomPadding(0);
	m_rightHandlesArea->setTopPadding(0); /// Why ?
	m_rightHandlesArea->setVisible(false);

	m_bottomHandlesArea->setMinimumHeight(50);
	m_bottomHandlesArea->setLeftPadding(0);
	m_bottomHandlesArea->setRightPadding(0); /// Why ?
	m_bottomHandlesArea->setVisible(false);

	m_topHandlesArea->setMinimumHeight(50);
	m_topHandlesArea->setLeftPadding(0);
	m_topHandlesArea->setRightPadding(0); /// Why ?
	m_topHandlesArea->setVisible(false);

//	m_bufferPreviewer = new AnalogBufferPreviewer(this);
//	m_bufferPreviewer->setMinimumHeight(20);
//	m_bufferPreviewer->setCursorPos(0.5);
//	m_bufferPreviewer->setHighlightPos(0.05);
//	m_bufferPreviewer->setHighlightWidth(0.2);
//	m_bufferPreviewer->setCursorVisible(false);
//	m_bufferPreviewer->setWaveformPos(0.1);
//	m_bufferPreviewer->setWaveformWidth(0.5);

//	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, this, [=]() {
//		horiz_offset = m_bufferPreviewer->highlightPos();
//	});
//	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, this, [=](int value) {
////		qInfo()<<value;
//		double moveTo = 0.0;
//		double min = xAxis()->min();
//		double max = xAxis()->max();
//		int width = m_bufferPreviewer->width();
//		double xA0xisWidth = max - min;

//		moveTo = value * xAxisWidth / width;
//		xAxis()->setInterval(min - moveTo, max - moveTo);
//		m_plot->replot();

//		auto delta = horiz_offset + (value/(float)width);

//		qInfo()<< delta << value << width;
//		m_bufferPreviewer->setHighlightPos(delta);


////		updateBufferPreviewer();

//	} );
//	m_layout->addWidget(m_bufferPreviewer,0,1);
	m_layout->addWidget(m_bottomHandlesArea,3,1);
	m_layout->addWidget(m_rightHandlesArea,2,2);
	m_layout->addWidget(m_leftHandlesArea,2,0);
	m_layout->addWidget(m_topHandlesArea,1,1);
	m_layout->addWidget(m_plot,2,1);
}

void PlotWidget::setupZoomer() {
	// zoomer
	// OscPlotZoomer - need constructor -
	m_zoomer = new ExtendingPlotZoomer(xAxis()->axisId(), yAxis()->axisId(), m_plot->canvas(), false);


	m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
							  Qt::RightButton, Qt::ControlModifier);
	m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
							  Qt::RightButton);
	m_zoomer->setTrackerMode(QwtPicker::ActiveOnly);
	const QColor c("#999999");
	m_zoomer->setRubberBandPen(c);
	m_zoomer->setTrackerPen(c);

	m_zoomer->setEnabled(true);
	m_zoomer->setZoomBase(false);
	m_plot->setMouseTracking(true);

	/*connect(m_zoomer,&ExtendingPlotZoomer::zoomed,this, [=](const QRectF &rect ) {
		for(int i = 0; i < 4;i++) {
			for(int j = 0 ;j < m_plotAxis[i].count();j++) {
				QwtPlotZoomer *zoomer = m_plotAxis[i][j]->zoomer();
				if(zoomer != nullptr) {
					if(zoomer->zoomRectIndex() < m_zoomer->zoomRectIndex()) {
						qInfo()<<zoomer->zoomRectIndex() << m_zoomer->zoomRectIndex() << i << j<< "ZoomIn";
						zoomer->zoom(rect);
					} else {
						qInfo()<<zoomer->zoomRectIndex() << m_zoomer->zoomRectIndex() << i<<j<<"ZoomOut";
						zoomer->zoom(0);
						zoomer->setZoomBase();
					}
				}
			}
		}
		m_plot->replot();
	});*/
}


PlotWidget::~PlotWidget() {

}

void PlotWidget::setupAxisScales() {
	for (unsigned int i = 0; i < 4; i++) {
		QwtScaleDraw::Alignment scale =
		    static_cast<QwtScaleDraw::Alignment>(i);
		auto scaleItem = new EdgelessPlotScaleItem(scale);

		scaleItem->scaleDraw()->setAlignment(scale);
		scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
		scaleItem->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
		scaleItem->setFont(m_plot->axisWidget(0)->font());

		QPalette palette = scaleItem->palette();
		palette.setBrush(QPalette::WindowText, QColor(0x6E6E6F));
		palette.setBrush(QPalette::Text, QColor(0x6E6E6F));
		scaleItem->setPalette(palette);
		scaleItem->setBorderDistance(0);
		scaleItem->attach(m_plot);
		m_scaleItems.push_back(scaleItem);
		scaleItem->setZ(200);
	}
}

void PlotWidget::setupOpenGLCanvas()
{
	bool useOpenGLCanvas = Preferences::GetInstance()->get("general_use_opengl").toBool();
	if(useOpenGLCanvas) {
		QwtPlotOpenGLCanvas* plotCanvas = qobject_cast< QwtPlotOpenGLCanvas* >( m_plot->canvas() );
		if ( plotCanvas == NULL )
		{
			plotCanvas = new QwtPlotOpenGLCanvas(m_plot);
			plotCanvas->setPaintAttribute(QwtPlotAbstractGLCanvas::BackingStore );
			m_plot->setCanvas( plotCanvas );
		} else {
			;
		}
	} else {
		QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( m_plot->canvas() );
		plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, true);
	}
}

void PlotWidget::setAxisScalesVisible(bool visible) {
	for(QwtPlotScaleItem* scale : qAsConst(m_scaleItems)){
		if(visible){
			scale->attach(m_plot);
		} else {
			scale->detach();
		}
	}
}

void PlotWidget::addPlotChannel(PlotChannel *ch)
{
	m_plotChannels.append(ch);
}

void PlotWidget::removePlotChannel(PlotChannel *ch)
{
	m_plotChannels.removeAll(ch);
}

void PlotWidget::addPlotAxisHandle(PlotAxisHandle *ax) {

	m_plotAxisHandles[ax->axis()->position()].append(ax);
}

void PlotWidget::removePlotAxisHandle(PlotAxisHandle *ax) {
	m_plotAxisHandles[ax->axis()->position()].removeAll(ax);
}


void PlotWidget::addPlotAxis(PlotAxis *ax)
{
	m_plotAxis[ax->position()].append(ax);
}

bool PlotWidget::getDisplayGraticule() const
{
	return displayGraticule;
}

void PlotWidget::setDisplayGraticule(bool newDisplayGraticule)
{
	displayGraticule = newDisplayGraticule;
	setAxisScalesVisible(!displayGraticule);
	graticule->enableGraticule(displayGraticule);
	m_plot->replot();
}

bool PlotWidget::eventFilter(QObject *object, QEvent *event)
{
	if (object == m_plot->canvas()) {
		switch (event->type()) {
		case QEvent::MouseMove: {
			Q_EMIT mouseMove(static_cast< QMouseEvent* >( event ));
			break;
		}
		case QEvent::MouseButtonPress: {
			Q_EMIT mouseButtonPress(static_cast< QMouseEvent* >( event ));
			break;
		}
		case QEvent::MouseButtonRelease: {
			Q_EMIT mouseButtonRelease(static_cast< QMouseEvent* >( event ));
			break;
		}
		case QEvent::Resize: {
//			updateHandleAreaPadding(d_labelsEnabled);

			//force cursor handles to emit position changed
			//when the plot canvas is being resized
//			d_hCursorHandle1->triggerMove();
//			d_hCursorHandle2->triggerMove();
//			d_vCursorHandle1->triggerMove();
//			d_vCursorHandle2->triggerMove();

			/* update the size of the gates when the plot canvas is resized */
//			updateGateMargins();

			Q_EMIT canvasSizeChanged();
			break;
		}
		default:
			break;
		}
	}
	return QObject::eventFilter(object, event);
}

QList<PlotAxis *> &PlotWidget::plotAxis(int position) {
	return m_plotAxis[position];
}

PlotAxis *PlotWidget::xAxis() {
	return m_xAxis;
}

PlotAxis *PlotWidget::yAxis() {
	return m_yAxis;
}


QwtPlot *PlotWidget::plot() const
{
	return m_plot;
}

void PlotWidget::replot()
{
	m_plot->replot();
}

void PlotWidget::selectChannel(PlotChannel *ch)
{
	m_yAxis->setVisible(false);
	if(m_selectedChannel != nullptr) {
		m_selectedChannel->yAxis()->setVisible(false);
	}
	m_selectedChannel = ch;

	m_selectedChannel->xAxis()->setVisible(false);
	m_selectedChannel->yAxis()->setVisible(true);

	if(m_selectedChannel->curve()) {
		m_selectedChannel->raise();
	}
}

PlotChannel *PlotWidget::selectedChannel() const
{
	return m_selectedChannel;
}

VertHandlesArea *PlotWidget::leftHandlesArea() const
{
	return m_leftHandlesArea;
}

VertHandlesArea *PlotWidget::rightHandlesArea() const
{
	return m_rightHandlesArea;
}

HorizHandlesArea *PlotWidget::topHandlesArea() const
{
	return m_topHandlesArea;
}

HorizHandlesArea *PlotWidget::bottomHandlesArea() const
{
	return m_bottomHandlesArea;
}

SymbolController *PlotWidget::symbolCtrl() const
{
	return m_symbolCtrl;
}

