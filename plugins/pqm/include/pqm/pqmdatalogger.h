#ifndef PQMDATALOGGER_H
#define PQMDATALOGGER_H

#include <QMap>
#include <QObject>
#include <QQueue>
#include <QFutureWatcher>

namespace scopy::pqm {
class PqmDataLogger : public QObject
{
	Q_OBJECT
public:
	enum ActiveInstrument
	{
		None,
		Waveform,
		Harmonics
	};
	PqmDataLogger(QObject *parent = nullptr);
	~PqmDataLogger();

	void setChnlsName(QVector<QString> chnlsName);
	void acquireBufferData(double val, int chIdx);
	void acquireAttrData(QString attrName, QString value, QString chId);
	void log();
	void writeToFile();
public Q_SLOTS:
	void logPressed(ActiveInstrument instr, const QString &filePath = "");

private:
	void acquireHarmonics(QString value, QString chId);
	void createHeader();

	ActiveInstrument m_crtInstr;
	QString m_filePath;
	QStringList m_chnlsName;

	QQueue<QString> m_logQue;
	QFutureWatcher<void> *m_writeFw;
	QMutex m_mutex;
};

} // namespace scopy::pqm

#endif // PQMDATALOGGER_H
