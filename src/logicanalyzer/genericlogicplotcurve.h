#ifndef GENERICLOGICPLOTCURVE_H
#define GENERICLOGICPLOTCURVE_H

#include <qwt_plot_curve.h>

class GenericLogicPlotCurve : public QwtPlotCurve
{
public:
	GenericLogicPlotCurve(const QString &name = {}, double pixelOffset = 0.0,
			      double traceHeight = 0.0, double sampleRate = 0.0,
			      double timeTriggerOffset = 0.0, uint64_t bufferSize = 0.0);

	QString getName() const;
	double getPixelOffset() const;
	double getTraceHeight() const;
	double getSampleRate() const;
	double getTimeTriggerOffset() const;
	uint64_t getBufferSize() const;

	void setName(const QString &name);
	void setPixelOffset(double pixelOffset);
	void setTraceHeight(double traceHeight);
	void setSampleRate(double sampleRate);
	void setTimeTriggerOffset(double timeTriggerOffset);
	void setBufferSize(uint64_t bufferSize);

protected:
	uint64_t fromTimeToSample(double time) const;
	double fromSampleToTime(uint64_t sample) const;

protected:
	QString m_name;
	double m_pixelOffset;
	double m_traceHeight;
	double m_sampleRate;
	double m_timeTriggerOffset;
	uint64_t m_bufferSize;
};

#endif // GENERICLOGICPLOTCURVE_H
