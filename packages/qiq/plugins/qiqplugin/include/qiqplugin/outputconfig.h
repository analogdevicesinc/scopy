#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QVariantMap>

namespace scopy::qiqplugin {

class OutputConfig
{
public:
	OutputConfig();
	~OutputConfig();

	bool isValid() const;
	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	QString outputFile() const;
	void setOutputFile(const QString &file);

	QString outputFileFormat() const;
	void setOutputFileFormat(const QString &format);

	QStringList enabledAnalysis() const;
	void setEnabledAnalysis(const QStringList &list);

private:
	QString m_outputFile;
	QString m_outputFileFormat;
	QStringList m_enabledAnalysis;
};
} // namespace scopy::qiqplugin

#endif // OUTPUTCONFIG_H
