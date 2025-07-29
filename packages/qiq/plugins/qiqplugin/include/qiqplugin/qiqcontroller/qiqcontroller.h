#ifndef QIQCONTROLLER_H
#define QIQCONTROLLER_H

#include "qiqcontroller/qiqplotinfo.h"
#include "cmdhandler.h"
#include "outputconfig.h"
#include "outputinfo.h"
#include "runresults.h"
#include <QObject>

namespace scopy::qiqplugin {

class QIQController : public QObject
{
	Q_OBJECT

private:
	CmdHandler *m_cmdHandler;
	// QIQConfiguration *m_qiqConfig;

public:
	// Constructor
	explicit QIQController(CommandFormat *cmdFormat, QObject *parent = nullptr);

	// Destructor
	~QIQController() = default;

	// Configuration methods
	void setCmdFormat(CommandFormat *cmdFormat);
	void configureInput(InputConfig &config);
	void configureAnalysis(QString &type, QVariantMap &config);
	void configureOutput(OutputConfig &config);

	// Control methods
	void runAnalysis();
	void getAnalysisTypes();
	void getAnalysisInfo(QString type);
	// QIQConfiguration getCurrentConfig();
	bool isReady();

Q_SIGNALS:
	void inputConfigured(InputConfig config);
	// maybe plot info ??
	void analysisConfigured(QString type, QVariantMap config, OutputInfo outputInfo);
	void analysisInfo(QString type, QVariantMap param, OutputInfo outputInfo,
			  QList<QIQPlotInfo> plotInfo); // plot info
	void outputConfigured(OutputConfig config);
	void processDataStarted();
	void processDataCompleted(RunResults results);
	void analysisTypesReceived(QStringList types);
	void errorOccurred(QString operation, QString error);
	void configurationChanged();

private Q_SLOTS:
	// Communication error handling
	void onCommunicationError(QString error);

	// Response handlers from CommandFormat
	void onProcessFinished(int exitCode);
	void onResponseReceived(QVariantMap response);
	void handleSetInputConfigResponse(QVariantMap response);
	void handleSetAnalysisConfigResponse(QVariantMap response);
	void handleSetOutputConfigResponse(QVariantMap response);
	void handleRunResponse(QVariantMap response);
	void handleGetAnalysisTypesResponse(QVariantMap response);
	void handleGetAnalysisInfoResponse(QVariantMap response);

	// Configuration and timeout management
	// void updateConfiguration(QString responseType, QVariantMap data);
	void startOperationTimeout(QString operation);
	void onOperationTimeout();
};

} // namespace scopy::qiqplugin

#endif // QIQCONTROLLER_H
