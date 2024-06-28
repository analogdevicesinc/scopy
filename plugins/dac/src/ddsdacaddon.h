#ifndef DDSDACADDON_H_
#define DDSDACADDON_H_

#include <QWidget>
#include "dacaddon.h"

namespace scopy {
class DacDataModel;
class TxNode;
class TxChannel;
class DdsDacAddon : public DacAddon
{
	Q_OBJECT
public:
	DdsDacAddon(DacDataModel *model, QWidget *parent=nullptr);
	virtual ~DdsDacAddon();

private:
	DacDataModel *m_model;
	QWidget *setupDdsTx(TxNode *txNode);
//	QWidget *setupTxChannel(TxNode *node, unsigned int mode);
//	TxTone *setupTxTone(TxNode *nodeTone, unsigned int index);
	QWidget *setupTxMode(TxNode *txNode, unsigned int mode);
};

class TxMode : public QWidget
{
	Q_OBJECT
public:
	TxMode(TxNode *node, unsigned int mode, QWidget *parent = nullptr);
	virtual ~TxMode();

	typedef enum
	{
		DISABLED,
		ONE_TONE,
		TWO_TONES,
		INDEPENDENT_IQ_CTRL
	} DdsMode;

	void read();
private:
	TxNode *m_node;
	unsigned int m_mode;
	QList<TxChannel*> m_txChannels;
};
} // namespace scopy

#endif // DDSDACADDON_H_
