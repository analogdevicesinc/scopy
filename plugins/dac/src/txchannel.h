#ifndef TXCHANNEL_H
#define TXCHANNEL_H

#include <QWidget>
#include <QList>

namespace scopy {
class TxNode;
class TxTone;
class TxChannel : public QWidget
{
	Q_OBJECT
public:
	TxChannel(TxNode *node, unsigned int nbTonesMode, QWidget *parent  = nullptr);
	virtual ~TxChannel();

	void read();
Q_SIGNALS:
	void frequencyUpdated(int toneIdx, QString frequency);
	void phaseUpdated(int toneIdx, QString phase);
	void scaleUpdated(int toneIdx, QString scale);

public Q_SLOTS:
	void updateFrequency(int toneIdx, QString frequency);
	void updatePhase(int toneIdx, QString phase);
	void updateScale(int toneIdx, QString scale);

private:
	TxNode *m_node;
	QList<TxTone*> m_tones;
	bool m_linkTones;

	TxTone *setupTxTone(TxNode *nodeTone, unsigned int index);
};


} // namespace scopy

#endif // TXCHANNEL_H
