#ifndef COMMANDFORMAT_H
#define COMMANDFORMAT_H

#include <QString>
#include <QVariantMap>

#include "inputconfig.h"
#include "outputconfig.h"

namespace scopy::qiqplugin {
class CommandFormat
{
public:
	virtual ~CommandFormat() {}

	virtual QString sendSetInputConfig(const InputConfig &config) = 0;
	virtual QString sendSetAnalysisConfig(const QString &type, const QVariantMap &config) = 0;
	virtual QString sendSetOutputConfig(const OutputConfig &config) = 0;
	virtual QString sendRun() = 0;
	virtual QString sendGetAnalysisTypes() = 0;
	virtual QString sendGetAnalysisInfo(const QString &type) = 0;
	virtual QVariantMap parseResponse(const QString &data) = 0;
	virtual QString getProtocolName() = 0;
};

} // namespace scopy::qiqplugin

#endif // COMMANDFORMAT_H
