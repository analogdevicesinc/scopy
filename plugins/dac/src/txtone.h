#ifndef TXTONE_H
#define TXTONE_H

#include <iio-widgets/iiowidget.h>
#include <QWidget>

namespace scopy {
namespace dac {
class TxNode;
class TxTone : public QWidget
{
	Q_OBJECT
public:
	TxTone(TxNode *node, unsigned int idx, QWidget *parent = nullptr);
	virtual ~TxTone();

	void read();
	QString toneUuid() const;
	QString frequency();
	QString phase();
	QString scale();
	void setPairedTone(TxTone *tone);
	static QString scaleDStoUI(QString data);
	static QString scaleUItoDS(QString data);
Q_SIGNALS:
	void frequencyUpdated(unsigned int toneIdx, QString frequency);
	void scaleUpdated(unsigned int toneIdx, QString oldScale, QString scale);
	void phaseUpdated(unsigned int toneIdx, QString phase);

public Q_SLOTS:
	void updateFrequency(QString frequency);
	void updateScale(QString scale);
	void updatePhase(QString phase);

private Q_SLOTS:
	void forwardFreqChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
	void forwardScaleChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
	void forwardPhaseChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);

private:
	QString phaseDStoUI(QString data);
	QString phaseUItoDS(QString data);
	QString frequencyDStoUI(QString data);
	QString frequencyUItoDS(QString data);
	QString computePairedPhase(QString frequency, QString phase);

	TxNode *m_node;
	TxTone *m_pairedTone;
	unsigned int m_idx;
	IIOWidget *m_frequency;
	IIOWidget *m_scale;
	IIOWidget *m_phase;
};
} // namespace dac
} // namespace scopy
#endif // TXTONE_H
