#ifndef ANALYSISCONFIG_H
#define ANALYSISCONFIG_H

#include <QVariantMap>
#include "outputinfo.h"

namespace scopy::qiqplugin {

class AnalysisConfig
{
public:
	AnalysisConfig();
	~AnalysisConfig();

	bool isValid() const;
	void fromVariantMap(const QVariantMap &params);
	QVariantMap toVariantMap() const;

	QString analysisType() const;
	void setAnalysisType(const QString &type);

	OutputInfo outputInfo() const;
	void setOutputInfo(const OutputInfo &info);

	QVariantMap params() const;
	void setParams(const QVariantMap &params);

private:
	QString m_analysisType;
	OutputInfo m_outInfo;
	QVariantMap m_params;
};

} // namespace scopy::qiqplugin

#endif // ANALYSISCONFIG_H
