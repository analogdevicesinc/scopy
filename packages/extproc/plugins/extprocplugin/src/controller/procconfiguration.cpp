#include "controller/procconfiguration.h"

using namespace scopy::extprocplugin;

ProcConfiguration::ProcConfiguration(QObject *parent)
	: QObject(parent)
	, m_inputConfig{}
	, m_outputConfig{}
	, m_analysis{}
{}

ProcConfiguration::~ProcConfiguration() {}

InputConfig ProcConfiguration::getInputConfig() const { return m_inputConfig; }

OutputConfig ProcConfiguration::getOutputConfig() const { return m_outputConfig; }

AnalysisConfig ProcConfiguration::getAnalysisList() const { return m_analysis; }

void ProcConfiguration::setInputConfig(const InputConfig &inputConfig) { m_inputConfig = inputConfig; }

void ProcConfiguration::setOutputConfig(const OutputConfig &outputConfig) { m_outputConfig = outputConfig; }

void ProcConfiguration::setAnalysis(const AnalysisConfig &analysis) { m_analysis = analysis; }

void ProcConfiguration::reset()
{
	m_inputConfig = {};
	m_outputConfig = {};
	m_analysis = {};
}

bool ProcConfiguration::isComplete() const
{
	QVariantMap input = m_inputConfig.toVariantMap();
	QVariantMap output = m_outputConfig.toVariantMap();
	return !input.isEmpty() && !output.isEmpty();
}
