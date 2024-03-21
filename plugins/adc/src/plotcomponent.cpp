#include "plotcomponent.h"

using namespace scopy;
using namespace adc;

PlotComponent::PlotComponent(QWidget *parent) : QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_lay = new QVBoxLayout(this);
	m_lay->setMargin(0);
	m_lay->setSpacing(0);
	m_plot = new PlotWidget(this);
	m_lay->addWidget(m_plot);
	setLayout(m_lay);
}

PlotComponent::~PlotComponent()
{

}

PlotWidget *PlotComponent::plot()
{
	return m_plot;
}

void PlotComponent::replot()
{
	m_plot->replot();
}

QString PlotComponent::getName()
{
	return "PlotComponent";
}

void PlotComponent::onStart()
{

}

void PlotComponent::onStop()
{

}

void PlotComponent::onInit()
{

}

void PlotComponent::onDeinit()
{

}
