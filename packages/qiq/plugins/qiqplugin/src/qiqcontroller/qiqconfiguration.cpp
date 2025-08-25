#include "qiqcontroller/qiqconfiguration.h"

using namespace scopy::qiqplugin;

QIQConfiguration::QIQConfiguration(QObject *parent)
	: QObject(parent)
	, m_inputConfig{}
	, m_outputConfig{}
	, m_analysis{}
{}

QIQConfiguration::~QIQConfiguration() {}

InputConfig QIQConfiguration::getInputConfig() const { return m_inputConfig; }

OutputConfig QIQConfiguration::getOutputConfig() const { return m_outputConfig; }

AnalysisConfig QIQConfiguration::getAnalysisList() const { return m_analysis; }

void QIQConfiguration::setInputConfig(const InputConfig &inputConfig) { m_inputConfig = inputConfig; }

void QIQConfiguration::setOutputConfig(const OutputConfig &outputConfig) { m_outputConfig = outputConfig; }

void QIQConfiguration::setAnalysis(const AnalysisConfig &analysis) { m_analysis = analysis; }

void QIQConfiguration::reset()
{
	m_inputConfig = {};
	m_outputConfig = {};
	m_analysis = {};
}

bool QIQConfiguration::isComplete() const
{
	QVariantMap input = m_inputConfig.toVariantMap();
	QVariantMap output = m_outputConfig.toVariantMap();
	return !input.isEmpty() && !output.isEmpty();
}
