#ifndef RUNRESULTS_H
#define RUNRESULTS_H

#include <QString>
#include <QVariant>
#include <QVariantMap>

namespace scopy::qiqplugin {
class RunResults
{
public:
	RunResults();
	~RunResults() = default;

	bool isValid();
	QVariantMap getResultForAnalysis(QString type);
	QVariant getMeasurement(QString name);

	QString getOutputFile() const;
	void setOutputFile(const QString &outputFile);

	QVariantMap getResults() const;
	void setResults(const QVariantMap &results);

	QVariantMap getMeasurements() const;
	void setMeasurements(const QVariantMap &measurements);

private:
	QString m_outputFile;
	QVariantMap m_results;
	QVariantMap m_measurements;
};
} // namespace scopy::qiqplugin

#endif // RUNRESULTS_H
