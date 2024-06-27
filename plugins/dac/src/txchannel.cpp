#include "txchannel.h"
#include "txtone.h"
#include "txnode.h"
#include "dac_logging_categories.h"

#include <menucollapsesection.h>
#include <menusectionwidget.h>

using namespace scopy;
using namespace scopy::dac;
TxChannel::TxChannel(TxNode *node, unsigned int nbTonesMode, QWidget *parent)
	: QWidget(parent)
	, m_node(node)
	, m_nbTonesMode(nbTonesMode)
{
	QVBoxLayout *chnIqLay = new QVBoxLayout();
	chnIqLay->setMargin(0);
	chnIqLay->setSpacing(10);
	this->setLayout(chnIqLay);

	connect(this, &TxChannel::resetChannelScales, this, &TxChannel::resetToneScales);

	auto children = m_node->getTones();
	bool isTone = (children.size() == 0);
	if(!isTone) {
		unsigned int i = 1;
		QWidget *toneList = new QWidget(this);
		toneList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
		QHBoxLayout *toneListLay = new QHBoxLayout();
		toneList->setLayout(toneListLay);
		toneListLay->setMargin(0);
		toneListLay->setSpacing(10);

		chnIqLay->addWidget(toneList);

		for(TxNode *nodeTone : children) {
			MenuSectionWidget *toneSection = new MenuSectionWidget(this);

			TxTone *tone = setupTxTone(nodeTone, i);
			connect(tone, &TxTone::scaleUpdated, this, &TxChannel::scaleUpdated);
			// if hidden don't add to layout and set visible to false
			if(i <= m_nbTonesMode) {
				toneListLay->addWidget(toneSection);
				toneSection->contentLayout()->addWidget(tone);
			} else {
				toneSection->setVisible(false);
			}
			i++;
		}
	} else {
		MenuSectionWidget *toneSection = new MenuSectionWidget(this);
		this->layout()->addWidget(toneSection);
		auto tone = setupTxTone(node, 1);
		toneSection->contentLayout()->addWidget(tone);
	}
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}

TxChannel::~TxChannel() {}

void TxChannel::resetToneScales()
{
	auto keys = m_tones.keys();
	for(unsigned int idx : keys) {
		if(idx > m_nbTonesMode) {
			m_tones[idx]->updateScale("0");
		}
	}
}

void TxChannel::scaleUpdated(int toneIdx, QString oldScale, QString newScale)
{
	auto keys = m_tones.keys();
	for(unsigned int idx : keys) {
		if(idx != toneIdx) {
			// compute paired scale
			auto scale2 = m_tones[idx]->scale();
			auto scale1 = newScale;

			bool ok;
			double d_scale1 = scale1.toDouble(&ok);
			if(!ok) {
				return;
			}
			double d_scale2 = scale2.toDouble(&ok);
			if(!ok) {
				return;
			}

			if((d_scale1 + d_scale2) > 1) {
				m_tones[toneIdx]->updateScale(oldScale);
			}
		}
	}
}

TxTone *TxChannel::setupTxTone(TxNode *nodeTone, unsigned int index)
{
	TxTone *tone = new TxTone(nodeTone, index, this);
	m_tones.insert(index, tone);
	return tone;
}

void TxChannel::read()
{
	for(auto tone : qAsConst(m_tones)) {
		tone->read();
	}
}

void TxChannel::enable(bool enable) { m_node->enableDds(enable); }

QString TxChannel::channelUuid() const { return m_node->getUuid(); }

QString TxChannel::frequency(unsigned int toneIdx) { return m_tones.value(toneIdx)->frequency(); }

QString TxChannel::phase(unsigned int toneIdx) { return m_tones.value(toneIdx)->phase(); }

unsigned int TxChannel::toneCount() { return m_tones.count(); }

TxTone *TxChannel::tone(unsigned int toneIdx)
{
	if(m_tones.contains(toneIdx)) {
		return m_tones[toneIdx];
	}
	return nullptr;
}
