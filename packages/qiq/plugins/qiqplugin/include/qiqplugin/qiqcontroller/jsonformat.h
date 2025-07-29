#ifndef JSONFORMAT_H
#define JSONFORMAT_H

#include "commandformat.h"

namespace scopy::qiqplugin {

class JsonFormat : public CommandFormat
{
public:
	QString sendSetInputConfig(InputConfig &config) override;
	QString sendSetAnalysisConfig(QString &type, QVariantMap &config) override;
	QString sendSetOutputConfig(OutputConfig &config) override;
	QString sendRun() override;
	QString sendGetAnalysisTypes() override;
	QString sendGetAnalysisInfo(QString type) override;
	QVariantMap parseResponse(const QString &data) override;
	QString getProtocolName() override;

private:
	QString buildJsonCommand(const QString &cmd, QVariantMap param);
	QVariantMap parseJsonResponse(const QString &response);

	const QString PROTOCOL_NAME = "json";
};

} // namespace scopy::qiqplugin

#endif // JSONFORMAT_H
