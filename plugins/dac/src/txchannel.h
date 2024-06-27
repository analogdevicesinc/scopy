#ifndef TXCHANNEL_H
#define TXCHANNEL_H

#include <QWidget>
#include <QMap>

namespace scopy {
namespace dac {
class TxNode;
class TxTone;
class TxChannel : public QWidget
{
	Q_OBJECT
public:
	TxChannel(TxNode *node, unsigned int nbTonesMode, QWidget *parent = nullptr);
	virtual ~TxChannel();

	void read();
	void enable(bool enable);
	QString channelUuid() const;
	QString frequency(unsigned int toneIdx);
	QString phase(unsigned int toneIdx);
	TxTone *tone(unsigned int toneIdx);
	unsigned int toneCount();
	QMap<QString, QWidget *> getToneMenus() const;
Q_SIGNALS:
	void resetChannelScales();
public Q_SLOTS:
	void scaleUpdated(int toneIdx, QString oldScale, QString newScale);

private Q_SLOTS:
	void resetToneScales();

private:
	TxNode *m_node;
	QMap<int, TxTone *> m_tones;
	unsigned int m_nbTonesMode;

	TxTone *setupTxTone(TxNode *nodeTone, unsigned int index);
};
} // namespace dac
} // namespace scopy

#endif // TXCHANNEL_H
