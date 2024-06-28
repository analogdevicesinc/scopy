#include "txchannel.h"
#include "txtone.h"
#include "txnode.h"

using namespace scopy;

TxChannel::TxChannel(TxNode *node, unsigned int nbTonesMode, QWidget *parent)
	: QWidget(parent)
	, m_node(node)
	, m_linkTones(false)
{
//	txChnWidget = new QWidget(this);
	QVBoxLayout *chnIqLay = new QVBoxLayout(this);
	chnIqLay->setMargin(10);
	chnIqLay->setSpacing(10);
	this->setLayout(chnIqLay);

	auto children = m_node->getTones();
	bool isTone = (children.size() == 0);
	QWidget *txChnWidget = nullptr;
	if (!isTone) {
		unsigned int i = 1;
		QLabel *chnIqLbl = new QLabel(this);
		chnIqLbl->setText("Channel " + node->getUuid());
		StyleHelper::MenuComboLabel(chnIqLbl);

		QWidget *toneList = new QWidget(this);
		QHBoxLayout *toneListLay = new QHBoxLayout(this);
		toneListLay->setMargin(0);
		toneListLay->setSpacing(10);
		toneList->setLayout(toneListLay);
		chnIqLay->addWidget(chnIqLbl);
		chnIqLay->addWidget(toneList);

		for (TxNode *nodeTone : children) {
			QWidget *tone = setupTxTone(nodeTone, i);
			if (i == nbTonesMode) {
				break;
			}
			i++;
		}
	} else {
		txChnWidget = setupTxTone(node, 1);
//		toneListLay->addWidget(tone);
	}
}

TxChannel::~TxChannel()
{

}

void TxChannel::updateFrequency(int toneIdx, QString frequency)
{
	m_tones.at(toneIdx)->updateFrequency(frequency);
}

void TxChannel::updatePhase(int toneIdx, QString phase)
{
	m_tones.at(toneIdx)->updatePhase(phase);
}

void TxChannel::updateScale(int toneIdx, QString scale)
{
	m_tones.at(toneIdx)->updateScale(scale);
}

TxTone *TxChannel::setupTxTone(TxNode *nodeTone, unsigned int index)
{
	TxTone *tone = new TxTone(nodeTone, index, this);
//	if (m_linkTones) {
		connect(tone, &TxTone::frequencyUpdated, this, &TxChannel::frequencyUpdated);
		connect(tone, &TxTone::scaleUpdated, this, &TxChannel::scaleUpdated);
		connect(tone, &TxTone::phaseUpdated, this, &TxChannel::phaseUpdated);
//	}
	m_tones.append(tone);
	this->layout()->addWidget(tone);
	return tone;
}

void TxChannel::read()
{
	for (TxTone* tone : m_tones) {
		tone->read();
	}
}

