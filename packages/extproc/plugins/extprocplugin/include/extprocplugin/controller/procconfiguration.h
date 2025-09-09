#ifndef QIQCONFIGURATION_H
#define QIQCONFIGURATION_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include <inputconfig.h>
#include <outputconfig.h>
#include <analysisconfig.h>

namespace scopy::extprocplugin {

class ProcConfiguration : public QObject
{
	Q_OBJECT
public:
	explicit ProcConfiguration(QObject *parent = nullptr);
	~ProcConfiguration();

	// Getters
	InputConfig getInputConfig() const;
	OutputConfig getOutputConfig() const;
	AnalysisConfig getAnalysisList() const;

	// Setters
	void setInputConfig(const InputConfig &inputConfig);
	void setOutputConfig(const OutputConfig &outputConfig);
	void setAnalysis(const AnalysisConfig &analysisList);

	// Utility methods
	void reset();
	bool isComplete() const;

private:
	InputConfig m_inputConfig;
	OutputConfig m_outputConfig;
	AnalysisConfig m_analysis;
};
} // namespace scopy::extprocplugin

#endif // QIQCONFIGURATION_H
