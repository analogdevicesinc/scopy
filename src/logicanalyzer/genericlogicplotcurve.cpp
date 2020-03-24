#include "genericlogicplotcurve.h"


GenericLogicPlotCurve::GenericLogicPlotCurve(const QString &name, double pixelOffset,
					     double traceHeight, double sampleRate,
					     double timeTriggerOffset, uint64_t bufferSize):
	QwtPlotCurve(),
	m_name(name),
	m_pixelOffset(pixelOffset),
	m_traceHeight(traceHeight),
	m_sampleRate(sampleRate),
	m_timeTriggerOffset(timeTriggerOffset),
	m_bufferSize(bufferSize)
{

}

QString GenericLogicPlotCurve::getName() const
{
	return m_name;
}

double GenericLogicPlotCurve::getPixelOffset() const
{
	return m_pixelOffset;
}

double GenericLogicPlotCurve::getTraceHeight() const
{
	return m_traceHeight;
}

double GenericLogicPlotCurve::getSampleRate() const
{
	return m_sampleRate;
}

double GenericLogicPlotCurve::getTimeTriggerOffset() const
{
	return m_timeTriggerOffset;
}

uint64_t GenericLogicPlotCurve::getBufferSize() const
{
	return m_bufferSize;
}

void GenericLogicPlotCurve::setName(const QString &name)
{
	if (m_name != name) {
		m_name = name;
	}
}

void GenericLogicPlotCurve::setPixelOffset(double pixelOffset)
{
	if (m_pixelOffset != pixelOffset) {
		m_pixelOffset = pixelOffset;

		setBaseline(m_pixelOffset + m_traceHeight);
	}
}

void GenericLogicPlotCurve::setTraceHeight(double traceHeight)
{
	if (m_traceHeight != traceHeight) {
		m_traceHeight = traceHeight;

		setBaseline(m_pixelOffset + m_traceHeight);
	}
}

void GenericLogicPlotCurve::setSampleRate(double sampleRate)
{
	if (m_sampleRate != sampleRate) {
		m_sampleRate = sampleRate;
	}
}

void GenericLogicPlotCurve::setTimeTriggerOffset(double timeTriggerOffset)
{
	if (m_timeTriggerOffset != timeTriggerOffset) {
		m_timeTriggerOffset = timeTriggerOffset;
	}
}

void GenericLogicPlotCurve::setBufferSize(uint64_t bufferSize)
{
	if (m_bufferSize != bufferSize) {
		m_bufferSize = bufferSize;
	}
}

uint64_t GenericLogicPlotCurve::fromTimeToSample(double time) const
{
	const double totalTime = static_cast<double>(m_bufferSize) / m_sampleRate;
	const double tmin = -(totalTime / 2.0 - (m_timeTriggerOffset * (1.0 / m_sampleRate)));
	const double tmax = totalTime / 2.0 + (m_timeTriggerOffset * (1.0 / m_sampleRate));
	const double smin = 0;
	const double smax = m_bufferSize;

	if (time > tmax) {
		time = tmax;
	}

	if (time < tmin) {
		time = tmin;
	}

	return (time - tmin) / (tmax - tmin) * (smax - smin) + smin;
}

double GenericLogicPlotCurve::fromSampleToTime(uint64_t sample) const
{
	const double totalTime = static_cast<double>(m_bufferSize) / m_sampleRate;
	const double tmin = -(totalTime / 2.0 - (m_timeTriggerOffset * (1.0 / m_sampleRate)));
	const double tmax = totalTime / 2.0 + (m_timeTriggerOffset * (1.0 / m_sampleRate));
	const double smin = 0;
	const double smax = m_bufferSize;

	if (sample > smax) {
		sample = smax;
	}

	return (sample - smin) / (smax - smin) * (tmax - tmin) + tmin;
}
