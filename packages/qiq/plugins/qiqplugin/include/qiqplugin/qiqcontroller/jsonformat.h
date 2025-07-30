#ifndef JSONFORMAT_H
#define JSONFORMAT_H

#include "commandformat.h"

namespace scopy::qiqplugin {

class JsonFormat : public CommandFormat
{
public:
	QString sendSetInputConfig(const InputConfig &config) override;
	QString sendSetAnalysisConfig(const QString &type, const QVariantMap &config) override;
	QString sendSetOutputConfig(const OutputConfig &config) override;
	QString sendRun() override;
	QString sendGetAnalysisTypes() override;
	QString sendGetAnalysisInfo(const QString &type) override;
	QVariantMap parseResponse(const QString &data) override;
	QString getProtocolName() override;

private:
	QString buildJsonCommand(const QString &cmd, const QVariantMap &param);
	QVariantMap parseJsonResponse(const QString &response);

	const QString PROTOCOL_NAME = "json";
};

} // namespace scopy::qiqplugin

#endif // JSONFORMAT_H
