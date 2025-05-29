#include "bubblelevelrenderer.hpp"

using namespace scopy;

BubbleLevelRenderer::BubbleLevelRenderer(QWidget *parent) : QWidget{parent} {
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	m_rot = {0.0f, 0.0f, 0.0f};
	m_displayPoints = "XY";

	plotWidget = new PlotWidget(this);
	lay->addWidget(plotWidget);

	plotWidget->xAxis()->setInterval(-180.0,180.0);
	plotWidget->yAxis()->setInterval(-180.0,180.0);

	plotWidget->xAxis()->setVisible(true);
	plotWidget->yAxis()->setVisible(true);


	point = new QwtPlotCurve("Point");
	point->setSamples(xLinePoint, yLinePoint);
	point->setStyle(QwtPlotCurve::Dots);
	point->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::red), QPen(Qt::red), QSize(20, 20)));
	point->attach(plotWidget->plot());
}

void BubbleLevelRenderer::setRot(data3P rot){
	m_rot = rot;

	if(m_displayPoints == "XY"){
		xLinePoint = {double(m_rot.dataX),double(m_rot.dataX)};
		yLinePoint = {double(m_rot.dataY),double(m_rot.dataY)};

	} else if(m_displayPoints == "XZ"){
		xLinePoint = {double(m_rot.dataX),double(m_rot.dataX)};
		yLinePoint = {double(m_rot.dataZ),double(m_rot.dataZ)};

	} else if(m_displayPoints == "YZ"){
		xLinePoint = {double(m_rot.dataY),double(m_rot.dataY)};
		yLinePoint = {double(m_rot.dataZ),double(m_rot.dataZ)};
	}

	point->setSamples(xLinePoint, yLinePoint);
	plotWidget->replot();
}

void BubbleLevelRenderer::setDisplayPoints(QString displayP){
	m_displayPoints = displayP;
}
