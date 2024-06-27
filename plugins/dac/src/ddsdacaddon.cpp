#include "ddsdacaddon.h"
#include "dacdatamodel.h"
#include "txnode.h"
#include "txchannel.h"
#include "txtone.h"
#include "dac_logging_categories.h"

#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menucombo.h>
#include <gui/mapstackedwidget.h>
#include <iio-widgets/iiowidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <stylehelper.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

using namespace scopy;
using namespace scopy::dac;
DdsDacAddon::DdsDacAddon(DacDataModel *model, QWidget *parent)
	: DacAddon(parent)
	, m_model(model)
{
	auto m_layout = new QHBoxLayout();
	m_layout->setMargin(0);
	m_layout->setAlignment(Qt::AlignTop);
	setLayout(m_layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	QWidget *txsContainer = new QWidget(scrollArea);
	QVBoxLayout *txsContainerLayout = new QVBoxLayout(txsContainer);
	txsContainerLayout->setMargin(0);
	txsContainerLayout->setSpacing(10);
	txsContainerLayout->setAlignment(Qt::AlignTop);
	txsContainer->setLayout(txsContainerLayout);

	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(txsContainer);

	for(int i = 0; i < model->getDdsTxs().size(); i++) {
		TxNode *txNode = model->getDdsTxs().value(model->getDdsTxs().keys().at(i));
		QWidget *tx = setupDdsTx(txNode);
		txsContainerLayout->addWidget(tx);
	}
	txsContainerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
	m_layout->addWidget(scrollArea);
}

QWidget *DdsDacAddon::setupDdsTx(TxNode *txNode)
{
	QWidget *tx = new QWidget(this);
	QVBoxLayout *txLay = new QVBoxLayout();
	txLay->setMargin(0);
	txLay->setSpacing(5);
	tx->setLayout(txLay);
	tx->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

	MapStackedWidget *ddsModeStack = new MapStackedWidget(this);
	ddsModeStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	ddsModeStack->add(QString::number(TxMode::DISABLED), setupTxMode(txNode, TxMode::DISABLED));
	ddsModeStack->add(QString::number(TxMode::ONE_TONE), setupTxMode(txNode, TxMode::ONE_TONE));
	ddsModeStack->add(QString::number(TxMode::TWO_TONES), setupTxMode(txNode, TxMode::TWO_TONES));
	ddsModeStack->add(QString::number(TxMode::INDEPENDENT_IQ_CTRL),
			  setupTxMode(txNode, TxMode::INDEPENDENT_IQ_CTRL));

	QWidget *txHeader = new QWidget(this);
	QHBoxLayout *txHeaderLay = new QHBoxLayout();
	txHeaderLay->setMargin(0);
	txHeaderLay->setSpacing(5);
	txHeader->setLayout(txHeaderLay);

	MenuSectionWidget *txLabelSection = new MenuSectionWidget(this);
	txLabelSection->setProperty("tutorial_name", "TX_INDICATOR");
	QLabel *txLabel = new QLabel(txNode->getUuid(), txLabelSection);
	StyleHelper::MenuMediumLabel(txLabel);
	txLabelSection->contentLayout()->addWidget(txLabel);
	txLabelSection->setFixedHeight(60);

	MenuSectionWidget *txReadSection = new MenuSectionWidget(this);
	txReadSection->setProperty("tutorial_name", "TX_READ");
	txReadSection->setFixedHeight(60);
	txReadSection->setFixedWidth(60);
	QPushButton *m_readBtn = new QPushButton(txReadSection);
	StyleHelper::RefreshButton(m_readBtn, "ReadAttributesButton");
	connect(m_readBtn, &QPushButton::clicked, this, [=, this]() {
		qDebug(CAT_DAC_DDS) << "Read button pressed";
		dynamic_cast<TxMode *>(ddsModeStack->currentWidget())->read();
	});
	txReadSection->contentLayout()->addWidget(m_readBtn);
	txReadSection->contentLayout()->setMargin(15);
	txReadSection->contentLayout()->setSpacing(0);

	MenuSectionWidget *ddsModeSection = new MenuSectionWidget(this);
	ddsModeSection->setProperty("tutorial_name", "TX_MODE_SELECTOR");
	m_ddsModeCombo = new MenuCombo("DDS MODE", this);
	StyleHelper::IIOComboBox(m_ddsModeCombo->combo());
	StyleHelper::BackgroundWidget(m_ddsModeCombo);
	auto cb = m_ddsModeCombo->combo();
	m_ddsModeCombo->combo()->addItem("Disabled", TxMode::DISABLED);
	m_ddsModeCombo->combo()->addItem("One CW Tone", TxMode::ONE_TONE);
	m_ddsModeCombo->combo()->addItem("Two CW Tones", TxMode::TWO_TONES);
	m_ddsModeCombo->combo()->addItem("Independent I/Q Control", TxMode::INDEPENDENT_IQ_CTRL);
	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		TxMode *current = dynamic_cast<TxMode *>(ddsModeStack->get(QString::number(mode)));
		if(current) {
			current->enable(mode != TxMode::DISABLED);
			current->read();
		}
		ddsModeStack->show(QString::number(mode));
	});

	ddsModeSection->contentLayout()->addWidget(m_ddsModeCombo);
	txHeaderLay->addWidget(txLabelSection, 1);
	txHeaderLay->addWidget(txReadSection, 1);
	txHeaderLay->addWidget(ddsModeSection, 4);

	txLay->addWidget(txHeader);
	txLay->addWidget(ddsModeStack);

	return tx;
}

QWidget *DdsDacAddon::setupTxMode(TxNode *txNode, unsigned int mode)
{
	TxMode *txModeWidget = new TxMode(txNode, mode, this);
	return txModeWidget;
}

DdsDacAddon::~DdsDacAddon() {}

void DdsDacAddon::enable(bool enable) { m_model->enableDds(enable); }

TxMode::TxMode(TxNode *node, unsigned int mode, QWidget *parent)
	: QWidget(parent)
	, m_mode(mode)
	, m_node(node)
{
	QVBoxLayout *txChnsLay = new QVBoxLayout();
	this->setLayout(txChnsLay);
	txChnsLay->setSpacing(10);
	txChnsLay->setMargin(0);

	auto txNodeChildren = m_node->getTones();
	for(TxNode *child : txNodeChildren) {
		TxChannel *txChn = new TxChannel(child, mode, this);
		m_txChannels.append(txChn);
		if(mode != DISABLED) {
			txChnsLay->addWidget(txChn);
		} else {
			txChn->setVisible(false);
		}
	}

	auto emitterChannel = m_txChannels.at(0);
	auto toneCount = emitterChannel->toneCount();
	switch(mode) {
	case ONE_TONE:
	case TWO_TONES:
		for(int chIdx = 1; chIdx < m_txChannels.size(); chIdx++) {
			m_txChannels.at(chIdx)->setVisible(false);
			// Pair all the first channel tones to the
			// corresponding tones in each channel
			for(int toneIdx = 1; toneIdx <= toneCount; toneIdx++) {
				TxTone *emitterTone = emitterChannel->tone(toneIdx);
				if(!emitterTone) {
					continue;
				}
				TxTone *pairTone = m_txChannels.at(chIdx)->tone(toneIdx);
				if(!pairTone) {
					continue;
				}
				emitterTone->setPairedTone(pairTone);
			}
		}
		break;
	default:
		break;
	}
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
}

TxMode::~TxMode() {}

void TxMode::read()
{
	for(TxChannel *chn : qAsConst(m_txChannels)) {
		chn->read();
	}
}

void TxMode::enable(bool enabled)
{
	// Write all the scale attrs to IIO
	for(int chIdx = 0; chIdx < m_txChannels.size(); chIdx++) {
		Q_EMIT m_txChannels[chIdx]->resetChannelScales();
	}

	// Set the "raw" attr to 0 or 1
	m_node->enableDds(enabled);
}
