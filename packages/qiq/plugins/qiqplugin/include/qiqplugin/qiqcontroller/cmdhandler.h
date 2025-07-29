#ifndef CMDHANDLER_H
#define CMDHANDLER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "commandformat.h"

namespace scopy::qiqplugin {

class CmdHandler : public QObject
{
	Q_OBJECT
public:
	explicit CmdHandler(CommandFormat *cmdFormat, QObject *parent = nullptr);
	~CmdHandler() = default;

	void setCmdFormat(CommandFormat *cmdFormat);
	void startProcess(const QString &program, const QStringList &arguments);
	void stopProcess();
	bool isProcessRunning();
	void sendCommand(const QString &cmd);

	void processData(QString &data);

	CommandFormat *cmdFormat() const;

Q_SIGNALS:
	void responseReceived(QVariantMap response);
	void errorOccurred(QString error);
	void processFinished(int exitCode);

private Q_SLOTS:
	void onProcessReadyReadStandardOutput();
	void onProcessReadyReadStandardError();
	void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onProcessError(QProcess::ProcessError error);

private:
	CommandFormat *m_cmdFormat;
	QProcess *m_cliProcess;
	QString m_response;
};

} // namespace scopy::qiqplugin

#endif // CMDHANDLER_H
