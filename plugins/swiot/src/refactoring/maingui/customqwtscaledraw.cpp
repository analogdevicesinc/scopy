#include "customqwtscaledraw.hpp"

using namespace adiscope;

CustomQwtScaleDraw::CustomQwtScaleDraw()
{}

CustomQwtScaleDraw::~CustomQwtScaleDraw()
{}

QwtText CustomQwtScaleDraw::label(double value) const
{
	MetricPrefixFormatter m_prefixFormater;
	return QwtText(QString::number(m_prefixFormater.getFormatedValue(value)) + " " + m_prefixFormater.getFormatedMeasureUnit(value) + m_unitOfMeasure);
}

void CustomQwtScaleDraw::setUnitOfMeasure(QString unitOfMeasure)
{
	m_unitOfMeasure = unitOfMeasure;
}
