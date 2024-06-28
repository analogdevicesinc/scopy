#ifndef TXTONE_H
#define TXTONE_H

#include <iio-widgets/iiowidget.h>
#include <QWidget>

namespace scopy {
class TxNode;
class TxTone : public QWidget
{
	Q_OBJECT
public:
	TxTone(TxNode *node, unsigned int idx, QWidget *parent  = nullptr);
	virtual ~TxTone();

	void read();
Q_SIGNALS:
	void frequencyUpdated(unsigned int toneIdx, QString frequency);
	void scaleUpdated(unsigned int toneIdx, QString scale);
	void phaseUpdated(unsigned int toneIdx, QString phase);

public Q_SLOTS:
	void updateFrequency(QString frequency);
	void updateScale(QString scale);
	void updatePhase(QString phase);

private:
	TxNode *m_node;
	unsigned int m_idx;
	IIOWidget *m_frequency;
	IIOWidget *m_scale;
	IIOWidget *m_phase;

	void forwardFreqChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
	void forwardScaleChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
	void forwardPhaseChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
};
} // namespace scopy
#endif // TXTONE_H
