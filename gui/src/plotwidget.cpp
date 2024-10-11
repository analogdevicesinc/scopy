#include "plotwidget.h"

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
#include <plotinfowidgets.h>
#include <plotnavigator.hpp>
#include <plotscales.h>
#include <plottracker.hpp>
#include <QwtPlotCanvas>
#include <qwt_scale_widget.h>

#include <osc_scale_engine.h>
#include <pluginbase/preferences.h>

#include <QPainter>
#include <QwtLegend>
#include <QwtPlotRenderer>
#include <stylehelper.h>

#include <plotbuttonmanager.h>

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

	m_plot->plotLayout()->setAlignCanvasToScales(true);
	m_plot->plotLayout()->setCanvasMargin(0);
	m_plot->plotLayout()->setSpacing(0);

	setupAxes();
	setupOpenGLCanvas();
	setupNavigator();
	setupPlotInfo();
	setupPlotScales();
	setupPlotButtonManager();

	m_plot->canvas()->installEventFilter(this);
}

void PlotWidget::setupNavigator()
{
	m_navigator = new PlotNavigator(this);
	connect(m_navigator, &PlotNavigator::rectChanged, this, &PlotWidget::plotScaleChanged);

	m_tracker = new PlotTracker(this);
}

PlotInfo *PlotWidget::getPlotInfo() { return m_plotInfo; }

PlotWidget::~PlotWidget() {}

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

PlotScales *PlotWidget::scales() const { return m_plotScales; }

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

void PlotWidget::plotChannelChangeXAxis(PlotChannel *c, PlotAxis *x)
{
	m_navigator->removeChannel(c);
	m_tracker->removeChannel(c);
	c->xAxis()->setVisible(false);
	c->setXAxis(x);
	m_navigator->addChannel(c);
	m_tracker->addChannel(c);
	showAxisLabels();
}

void PlotWidget::plotChannelChangeYAxis(PlotChannel *c, PlotAxis *y)
{
	m_navigator->removeChannel(c);
	m_tracker->removeChannel(c);
	c->yAxis()->setVisible(false);
	c->setYAxis(y);
	m_navigator->addChannel(c);
	m_tracker->addChannel(c);
	showAxisLabels();
}

void PlotWidget::addPlotChannel(PlotChannel *ch)
{
	ch->init();
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

	// QwtAxisId cannot be removed from QwtPlot
	ch->yAxis()->setVisible(false);
	if(m_selectedChannel == ch) {
		if(m_plotChannels.size() > 0) {
			selectChannel(m_plotChannels[0]);
		} else {
			selectChannel(nullptr);
		}
	}

	ch->deinit();
	Q_EMIT removedChannel(ch);
}

QList<PlotChannel *> PlotWidget::getChannels() { return m_plotChannels; }

void PlotWidget::addPlotAxisHandle(PlotAxisHandle *ax) { m_plotAxisHandles[ax->axis()->position()].append(ax); }

void PlotWidget::removePlotAxisHandle(PlotAxisHandle *ax) { m_plotAxisHandles[ax->axis()->position()].removeAll(ax); }

void PlotWidget::addPlotAxis(PlotAxis *ax) { m_plotAxis[ax->position()].append(ax); }

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

QGridLayout *PlotWidget::layout() { return m_layout; }

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

PlotButtonManager *PlotWidget::plotButtonManager() const { return m_plotButtonManager; }

void PlotWidget::setupPlotInfo()
{
	m_plotInfo = new PlotInfo(m_plot->canvas());
	m_plotInfo->addCustomInfo(new FPSInfo(this, this), InfoPosition::IP_LEFT);

	HDivInfo *hDivInfo = new HDivInfo(this);
	m_plotInfo->addCustomInfo(hDivInfo, InfoPosition::IP_LEFT);

	if(!Preferences::GetInstance()->get("show_grid").toBool()) {
		hDivInfo->hide();
	}
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[=](QString preference, QVariant value) {
			if(preference == "show_grid") {
				hDivInfo->setVisible(value.toBool());
			}
		});
}

void PlotWidget::setupPlotScales() { m_plotScales = new PlotScales(this); }

void PlotWidget::setupAxes()
{
	// this is needed to hide default QwtPlot axes
	m_plot->setAxisVisible(QwtAxis::YLeft, false);
	m_plot->setAxisVisible(QwtAxis::XBottom, false);

	m_xPosition = Preferences::get("adc_plot_xaxis_label_position").toInt();
	m_yPosition = Preferences::get("adc_plot_yaxis_label_position").toInt();
	QPen pen(QColor("#9E9E9F"));
	m_xAxis = new PlotAxis(m_xPosition, this, pen, this);
	m_yAxis = new PlotAxis(m_yPosition, this, pen, this);
}

void PlotWidget::setupPlotButtonManager()
{
	m_plotButtonManager = new PlotButtonManager(this);
	m_plotButtonManager->setCollapseOrientation(PlotButtonManager::PBM_RIGHT);
	HoverWidget *hoverPlotButtonManager = new HoverWidget(m_plotButtonManager, m_plot->canvas(), m_plot->canvas());
	hoverPlotButtonManager->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	hoverPlotButtonManager->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverPlotButtonManager->setAnchorOffset(QPoint(0, -20));
	hoverPlotButtonManager->setRelative(true);
	hoverPlotButtonManager->show();
}

QwtSymbol::Style PlotWidget::getCurveStyle(int i)
{
	if(i == 0)
		return QwtSymbol::Ellipse;
	if(i == 1)
		return QwtSymbol::Rect;
	if(i == 2)
		return QwtSymbol::Diamond;
	if(i == 3)
		return QwtSymbol::Triangle;
	if(i == 4)
		return QwtSymbol::Cross;
	if(i == 5)
		return QwtSymbol::XCross;
	if(i == 6)
		return QwtSymbol::Star1;
	if(i == 7)
		return QwtSymbol::Star2;
	if(i == 8)
		return QwtSymbol::Hexagon;
	if(i == 9)
		return QwtSymbol::DTriangle;
	if(i == 10)
		return QwtSymbol::UTriangle;
	if(i == 11)
		return QwtSymbol::LTriangle;

	return QwtSymbol::RTriangle;
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
	} else {
		xAxis()->setVisible(m_showXAxisLabels);
		yAxis()->setVisible(m_showXAxisLabels);
	}
}

void PlotWidget::selectChannel(PlotChannel *ch)
{
	hideAxisLabels();
	m_selectedChannel = ch;
	showAxisLabels();

	if(m_selectedChannel != nullptr) {
		if(m_selectedChannel->curve()) {
			m_selectedChannel->raise();
		}
	}
	// return;

	Q_EMIT channelSelected(m_selectedChannel);
}

void PlotWidget::setUnitsVisible(bool visible)
{
	for(PlotChannel *ch : getChannels()) {
		ch->xAxis()->setUnitsVisible(visible);
		ch->yAxis()->setUnitsVisible(visible);
	}
}

void PlotWidget::printPlot(QPainter *painter, bool useSymbols)
{
	QwtPlotRenderer renderer;
	renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
	renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, true);

	// Change axis and legend color
	auto currentStyle = m_plot->styleSheet();
	m_plot->setStyleSheet("color: #9E9E9F");

	// Apply printable plot changes
	// list of current curve colors
	QList<QPen> plotChColors;
	for(int i = 0; i < getChannels().length(); i++) {
		QPen printPen = getChannels().at(i)->curve()->pen();
		// save current curve color
		plotChColors.push_back(getChannels().at(i)->curve()->pen());
		// get channel colors from StyleHelper
		printPen.setColor(StyleHelper::getColor("CH" + QString::number(i)));
		printPen.setWidth(2);
		getChannels().at(i)->curve()->setPen(printPen);
		if(useSymbols) {
			QwtSymbol *symbol = new QwtSymbol(getCurveStyle(i), Qt::white, printPen, QSize(10, 10));
			getChannels().at(i)->curve()->setSymbol(symbol);
		}
	}

	QwtLegend *legendDisplay = new QwtLegend(m_plot);
	legendDisplay->setDefaultItemMode(QwtLegendData::ReadOnly);
	m_plot->insertLegend(legendDisplay, QwtPlot::TopLegend);
	m_plot->updateLegend();

	m_plot->replot();

	// Print plot to Painter
	renderer.render(m_plot, painter, QRectF(0, 0, 400, 300));

	// revert changes made for printable plot
	m_plot->insertLegend(nullptr);
	m_plot->setStyleSheet(currentStyle);

	// revert curves to original settings
	for(int i = 0; i < getChannels().length(); i++) {
		getChannels().at(i)->curve()->setPen(plotChColors.at(i));
		if(useSymbols) {
			getChannels().at(i)->curve()->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol));
		}
	}
	m_plot->replot();
}

PlotChannel *PlotWidget::selectedChannel() const { return m_selectedChannel; }

#include "moc_plotwidget.cpp"
