#ifndef GENERICLOGICPLOTCURVE_H
#define GENERICLOGICPLOTCURVE_H

#include <qwt_plot_curve.h>

enum class LogicPlotCurveType : int {
	Data = 0,
	Annotations = 1,
};

class GenericLogicPlotCurve : public QwtPlotCurve
{
public:
	GenericLogicPlotCurve(const QString &name = {}, LogicPlotCurveType type = LogicPlotCurveType::Data,
			      double pixelOffset = 0.0, double traceHeight = 0.0, double sampleRate = 0.0,
			      double timeTriggerOffset = 0.0, uint64_t bufferSize = 0.0);

	QString getName() const;
	double getPixelOffset() const;
	double getTraceHeight() const;
	double getSampleRate() const;
	double getTimeTriggerOffset() const;
	uint64_t getBufferSize() const;

	LogicPlotCurveType getType() const;

	void setName(const QString &name);
	void setPixelOffset(double pixelOffset);
	void setTraceHeight(double traceHeight);
	void setSampleRate(double sampleRate);
	void setTimeTriggerOffset(double timeTriggerOffset);
	void setBufferSize(uint64_t bufferSize);

	// Classes who inherit from GenericLogicPlotCurve must provide their
	// own behaviour for these methods
	virtual void dataAvailable(uint64_t from, uint64_t to) {}
	virtual void reset() {}

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
	LogicPlotCurveType m_type;
};

#endif // GENERICLOGICPLOTCURVE_H
