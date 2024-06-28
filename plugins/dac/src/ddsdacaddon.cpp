#include "ddsdacaddon.h"
#include "dacdatamodel.h"
#include "txnode.h"
#include "txchannel.h"

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
DdsDacAddon::DdsDacAddon(DacDataModel *model, QWidget *parent)
	: DacAddon("yellow", parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
//	m_layout->setSpacing()
	setLayout(m_layout);
//	StyleHelper::GetInstance()->initColorMap();

	QWidget *txsContainer = new QWidget(this);
	QVBoxLayout *txsContainerLayout = new QVBoxLayout(this);
	txsContainerLayout->setMargin(0);
	txsContainerLayout->setSpacing(10);
	txsContainerLayout->setContentsMargins(0, 0, 10, 0);
	txsContainer->setLayout(txsContainerLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(txsContainer);
	m_layout->addWidget(scrollArea);

	for (int i = 0; i <  model->getDdsTxs().size(); i++) {
		TxNode *txNode = model->getDdsTxs().value(model->getDdsTxs().keys().at(i));
		QWidget *tx = setupDdsTx(txNode);
		txsContainerLayout->addWidget(tx);
	}
	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	txsContainerLayout->addItem(spacer);
}

QWidget *DdsDacAddon::setupDdsTx(TxNode *txNode)
{
	QWidget *tx = new QWidget(this);
	QVBoxLayout *txLay = new QVBoxLayout(this);
	txLay->setMargin(0);
	txLay->setSpacing(5);
	tx->setLayout(txLay);

	MapStackedWidget *ddsModeStack = new MapStackedWidget(this);
	ddsModeStack->add(QString::number(TxMode::DISABLED), setupTxMode(txNode, TxMode::DISABLED));
	ddsModeStack->add(QString::number(TxMode::ONE_TONE), setupTxMode(txNode, TxMode::ONE_TONE));
	ddsModeStack->add(QString::number(TxMode::TWO_TONES), setupTxMode(txNode, TxMode::TWO_TONES));
	ddsModeStack->add(QString::number(TxMode::INDEPENDENT_IQ_CTRL), setupTxMode(txNode, TxMode::INDEPENDENT_IQ_CTRL));

	QWidget *txHeader = new QWidget(this);
	QHBoxLayout *txHeaderLay = new QHBoxLayout(this);
	txHeaderLay->setMargin(0);
	txHeaderLay->setSpacing(10);
	txHeader->setLayout(txHeaderLay);

	MenuControlButton *txBtn = new MenuControlButton(this);
	txBtn->setName(txNode->getUuid());
	txBtn->setOpenMenuChecksThis(true);
	txBtn->setDoubleClickToOpenMenu(true);
	txBtn->checkBox()->setVisible(false);
	txBtn->button()->setVisible(true);

	MenuCombo *combo = new MenuCombo("DDS MODE", this);
	StyleHelper::IIOComboBox(combo->combo());
	StyleHelper::BackgroundWidget(combo);
	auto cb = combo->combo();
	combo->combo()->addItem("Disabled", TxMode::DISABLED);
	combo->combo()->addItem("One CW Tone", TxMode::ONE_TONE);
	combo->combo()->addItem("Two CW Tones", TxMode::TWO_TONES);
	combo->combo()->addItem("Independent I/Q Control", TxMode::INDEPENDENT_IQ_CTRL);
	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		TxMode *current = dynamic_cast<TxMode*>(ddsModeStack->get(QString::number(mode)));
		if (current) {
			current->read();
		}
		ddsModeStack->show(QString::number(mode));
	});

	txHeaderLay->addWidget(txBtn);
	txHeaderLay->addWidget(combo);

	txLay->addWidget(txHeader);
	txLay->addWidget(ddsModeStack);

	return tx;

}

QWidget *DdsDacAddon::setupTxMode(TxNode *txNode, unsigned int mode)
{
	TxMode *txModeWidget = new TxMode(txNode, mode, this);
	return txModeWidget;
//	m_txModes.append(txModeWidget);
}

DdsDacAddon::~DdsDacAddon()
{

}

TxMode::TxMode(TxNode *node, unsigned int mode, QWidget *parent)
	: QWidget(parent)
	, m_mode(mode)
	, m_node(node)
{
	//		return new QWidget(this);
	QVBoxLayout *txChnsLay = new QVBoxLayout(this);
	txChnsLay->setSpacing(10);
	this->setLayout(txChnsLay);
	StyleHelper::BackgroundWidget(this);
	if (mode != DISABLED) {
		auto txNodeChildren = m_node->getTones();
		for (TxNode *child : txNodeChildren) {
			TxChannel *txChn = new TxChannel(child, mode, this);
			m_txChannels.append(txChn);
			txChnsLay->addWidget(txChn);
		}

		//	if (txChannels.size() == 0) {
		//		return nullptr;
		// TBD handle nullwidget or set to disabled?
		//	}

		auto emitterChannel = m_txChannels.at(0);
		switch (mode) {
		case ONE_TONE:
			for (int i = 1; i < m_txChannels.size(); i++) {
				m_txChannels.at(i)->setVisible(false);
			}
		case TWO_TONES:
			for (int i = 1; i < m_txChannels.size(); i++) {
				connect(emitterChannel, &TxChannel::frequencyUpdated,
					m_txChannels[i], &TxChannel::updateFrequency);
				connect(emitterChannel, &TxChannel::phaseUpdated,
					m_txChannels[i], &TxChannel::updatePhase);
				connect(emitterChannel, &TxChannel::scaleUpdated,
					m_txChannels[i], &TxChannel::updateScale);
			}
			break;
		default:
			break;
		}
	}
}

TxMode::~TxMode()
{

}

void TxMode::read()
{
	for (TxChannel* chn : m_txChannels) {
		chn->read();
	}
}
