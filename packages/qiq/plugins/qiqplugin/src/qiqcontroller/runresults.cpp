#include "qiqcontroller/runresults.h"

using namespace scopy::qiqplugin;

RunResults::RunResults()
{
	// TODO: Initialize members
}

bool RunResults::isValid()
{
	// TODO: Check if results are valid
	return true;
}

QVariantMap RunResults::getResultForAnalysis(QString type)
{
	// TODO: Get result for specific analysis type
	return QVariantMap();
}

QVariant RunResults::getMeasurement(QString name)
{
	// TODO: Get specific measurement by name
	return QVariant();
}

QString RunResults::getOutputFile() const { return m_outputFile; }

void RunResults::setOutputFile(const QString &outputFile) { m_outputFile = outputFile; }

QVariantMap RunResults::getResults() const { return m_results; }

void RunResults::setResults(const QVariantMap &results) { m_results = results; }

QVariantMap RunResults::getMeasurements() const { return m_measurements; }

void RunResults::setMeasurements(const QVariantMap &measurements) { m_measurements = measurements; }
