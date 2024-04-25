#include "plotwidget.h"

#include "DisplayPlot.h"
#include "plotaxis.h"

#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QwtPlot>
#include <QwtPlotCurve>
#include <QwtPlotLayout>
#include <QwtPlotOpenGLCanvas>
#include <QwtPlotSeriesItem>
#include <plotnavigator.hpp>
#include <plottracker.hpp>

#include <DisplayPlot.h>
#include <customqwtscaledraw.hpp>
#include <osc_scale_engine.h>
#include <pluginbase/preferences.h>

using namespace scopy;

PlotWidget::PlotWidget(QWidget *parent)
	: QWidget(parent)
{

	m_selectedChannel = nullptr;
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_plot = new QwtPlot(this);
	m_plot->canvas()->setContentsMargins(0, 0, 0, 0);
	m_layout = new QGridLayout(this);
	m_layout->addWidget(m_plot, 1, 0);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	setLayout(m_layout);

	QPen pen(QColor("#9E9E9F"));

	setupOpenGLCanvas();

	m_xPosition = Preferences::get("adc_plot_xaxis_label_position").toInt();
	m_yPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();

	// default Axis
	new PlotAxis(QwtAxis::XBottom, this, pen, this);
	new PlotAxis(QwtAxis::YLeft, this, pen, this);
	hideDefaultAxis();

	m_xAxis = new PlotAxis(m_xPosition, this, pen, this);
	m_yAxis = new PlotAxis(m_yPosition, this, pen, this);

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
	connect(this, SIGNAL(canvasSizeChanged()), graticule, SLOT(onCanvasSizeChanged()));
	setDisplayGraticule(false);

	m_plot->plotLayout()->setAlignCanvasToScales(true);
	m_plot->plotLayout()->setCanvasMargin(0);
	m_plot->plotLayout()->setSpacing(0);

	setupNavigator();

	m_plot->canvas()->installEventFilter(this);
}

void PlotWidget::setupNavigator()
{
	m_navigator = new PlotNavigator(this);
	connect(m_navigator, &PlotNavigator::rectChanged, this, &PlotWidget::plotScaleChanged);

	m_tracker = new PlotTracker(this);
}

void PlotWidget::addPlotInfoSlot(QWidget *w) { m_layout->addWidget(w, 0, 0); }

PlotWidget::~PlotWidget() {}

void PlotWidget::setupAxisScales()
{
	for(unsigned int i = 0; i < 4; i++) {
		QwtScaleDraw::Alignment scale = static_cast<QwtScaleDraw::Alignment>(i);
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

PlotAxis *PlotWidget::plotAxisFromId(QwtAxisId axisId)
{
	for(QList<PlotAxis *> axes : m_plotAxis) {
		for(PlotAxis *axis : axes) {
			if(axis->axisId() == axisId) {
				return axis;
			}
		}
	}

	return nullptr;
}

PlotNavigator *PlotWidget::navigator() const { return m_navigator; }

PlotTracker *PlotWidget::tracker() const { return m_tracker; }

void PlotWidget::setupOpenGLCanvas()
{
	bool useOpenGLCanvas = Preferences::GetInstance()->get("general_use_opengl").toBool();
	if(useOpenGLCanvas) {
		QwtPlotOpenGLCanvas *plotCanvas = qobject_cast<QwtPlotOpenGLCanvas *>(m_plot->canvas());
		if(plotCanvas == NULL) {
			plotCanvas = new QwtPlotOpenGLCanvas(m_plot);
			plotCanvas->setPaintAttribute(QwtPlotAbstractGLCanvas::BackingStore);
			m_plot->setCanvas(plotCanvas);
		} else {
			;
		}
	} else {
		QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>(m_plot->canvas());
		plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, true);
	}
}

void PlotWidget::setAxisScalesVisible(bool visible)
{
	for(QwtPlotScaleItem *scale : qAsConst(m_scaleItems)) {
		if(visible) {
			scale->attach(m_plot);
		} else {
			scale->detach();
		}
	}
}

void PlotWidget::addPlotChannel(PlotChannel *ch)
{
	m_plotChannels.append(ch);
	if(m_selectedChannel == nullptr) {
		selectChannel(ch);
	}

	connect(ch, &PlotChannel::doReplot, this, &PlotWidget::replot);
	connect(ch, &PlotChannel::attachCurve, this, [=](QwtPlotCurve *curve) { curve->attach(m_plot); });
	m_navigator->addChannel(ch);
	m_tracker->addChannel(ch);
	Q_EMIT addedChannel(ch);
}

void PlotWidget::removePlotChannel(PlotChannel *ch)
{
	m_plotChannels.removeAll(ch);

	m_navigator->removeChannel(ch);
	m_tracker->removeChannel(ch);
	Q_EMIT removedChannel(ch);
}

QList<PlotChannel *> PlotWidget::getChannels() { return m_plotChannels; }

void PlotWidget::addPlotAxisHandle(PlotAxisHandle *ax) { m_plotAxisHandles[ax->axis()->position()].append(ax); }

void PlotWidget::removePlotAxisHandle(PlotAxisHandle *ax) { m_plotAxisHandles[ax->axis()->position()].removeAll(ax); }

void PlotWidget::addPlotAxis(PlotAxis *ax) { m_plotAxis[ax->position()].append(ax); }

bool PlotWidget::getDisplayGraticule() const { return displayGraticule; }

void PlotWidget::setDisplayGraticule(bool newDisplayGraticule)
{
	displayGraticule = newDisplayGraticule;
	setAxisScalesVisible(!displayGraticule);
	graticule->enableGraticule(displayGraticule);
	m_plot->replot();
}

bool PlotWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == m_plot->canvas()) {
		switch(event->type()) {
		case QEvent::MouseMove: {
			Q_EMIT mouseMove(static_cast<QMouseEvent *>(event));
			break;
		}
		case QEvent::MouseButtonPress: {
			Q_EMIT mouseButtonPress(static_cast<QMouseEvent *>(event));
			break;
		}
		case QEvent::MouseButtonRelease: {
			Q_EMIT mouseButtonRelease(static_cast<QMouseEvent *>(event));
			break;
		}
		case QEvent::Resize: {
			Q_EMIT canvasSizeChanged();
			break;
		}
		default:
			break;
		}
	}

	return QObject::eventFilter(object, event);
}

QList<PlotAxis *> &PlotWidget::plotAxis(int position) { return m_plotAxis[position]; }

PlotAxis *PlotWidget::xAxis() { return m_xAxis; }

PlotAxis *PlotWidget::yAxis() { return m_yAxis; }

QwtPlot *PlotWidget::plot() const { return m_plot; }

void PlotWidget::replot() { m_plot->replot(); }

void PlotWidget::hideAxisLabels()
{
	m_yAxis->setVisible(false);
	m_xAxis->setVisible(false);
	if(m_selectedChannel != nullptr) {
		m_selectedChannel->yAxis()->setVisible(false);
		m_selectedChannel->xAxis()->setVisible(false);
	}
}

void PlotWidget::setAlignCanvasToScales(bool alignCanvasToScales)
{
	m_plot->plotLayout()->setAlignCanvasToScales(alignCanvasToScales);
}

void PlotWidget::hideDefaultAxis()
{
	m_plot->setAxisVisible(QwtAxisId(QwtAxis::XBottom, 0), false);
	m_plot->setAxisVisible(QwtAxisId(QwtAxis::XTop, 0), false);
	m_plot->setAxisVisible(QwtAxisId(QwtAxis::YLeft, 0), false);
	m_plot->setAxisVisible(QwtAxisId(QwtAxis::YRight, 0), false);
}

bool PlotWidget::showYAxisLabels() const { return m_showYAxisLabels; }

void PlotWidget::setShowYAxisLabels(bool newShowYAxisLabels) { m_showYAxisLabels = newShowYAxisLabels; }

bool PlotWidget::showXAxisLabels() const { return m_showXAxisLabels; }

void PlotWidget::setShowXAxisLabels(bool newShowXAxisLabels) { m_showXAxisLabels = newShowXAxisLabels; }

void PlotWidget::showAxisLabels()
{
	if(m_selectedChannel != nullptr) {
		m_selectedChannel->xAxis()->setVisible(m_showXAxisLabels);
		m_selectedChannel->yAxis()->setVisible(m_showYAxisLabels);
	}
}

void PlotWidget::selectChannel(PlotChannel *ch)
{
	hideAxisLabels();
	m_selectedChannel = ch;
	showAxisLabels();

	if(!m_selectedChannel)
		return;

	if(m_selectedChannel->curve()) {
		m_selectedChannel->raise();
	}

	Q_EMIT channelSelected(m_selectedChannel);
}

void PlotWidget::setUnitsVisible(bool visible)
{
	for(PlotChannel *ch : getChannels()) {
		ch->xAxis()->setUnitsVisible(visible);
		ch->yAxis()->setUnitsVisible(visible);
	}
}

PlotChannel *PlotWidget::selectedChannel() const { return m_selectedChannel; }

#include "moc_plotwidget.cpp"
