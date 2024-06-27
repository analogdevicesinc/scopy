#include "ddsdacaddon.h"
#include "dacdatamodel.h"
#include "txnode.h"

#include <gui/widgets/menucontrolbutton.h>
#include <gui/widgets/menucombo.h>
#include <gui/mapstackedwidget.h>
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
	m_layout->setMargin(50);
	m_layout->setSpacing(10);
	setLayout(m_layout);
//	StyleHelper::GetInstance()->initColorMap();

	QWidget *txsContainer = new QWidget(this);
	QVBoxLayout *txsContainerLayout = new QVBoxLayout(this);
	txsContainer->setLayout(txsContainerLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(txsContainer);
	m_layout->addWidget(scrollArea);

	for (int i = 0; i <  model->getDdsTxs().size(); i++) {
		TxNode *txNode = model->getDdsTxs().value(model->getDdsTxs().keys().at(i));
		QWidget *tx = new QWidget(this);
		QVBoxLayout *txLay = new QVBoxLayout(this);
		tx->setLayout(txLay);

		MapStackedWidget *ddsModeStack = new MapStackedWidget(this);
		auto txNodeChildren = txNode->getTones();
		for (TxNode *nodeChildren : txNodeChildren) {
			QWidget *chnIQ = new QWidget(this);
			QVBoxLayout *chnIqLay = new QVBoxLayout(this);
			chnIQ->setLayout(chnIqLay);
			QLabel *chnIqLbl = new QLabel(this);
			chnIqLbl->setText("Channel " + nodeChildren->getUuid());

			QHBoxLayout *toneListLay = new QHBoxLayout(this);
			chnIqLay->addWidget(chnIqLbl);
			chnIqLay->addItem(toneListLay);

			for (int j = 0; j < nodeChildren->getTones().size(); j++){
				TxNode *nodeTone = nodeChildren->getTones().value(nodeChildren->getTones().keys().at(j));
				QWidget *tone = new QWidget(this);
				QVBoxLayout *toneLay = new QVBoxLayout(this);
				tone->setLayout(toneLay);

			}

		}

		QWidget *txHeader = new QWidget(this);
		QHBoxLayout *txHeaderLay = new QHBoxLayout(this);
		txHeader->setLayout(txHeaderLay);

		MenuControlButton *txBtn = new MenuControlButton(this);
		txBtn->setName(txNode->getUuid());
		txBtn->setOpenMenuChecksThis(true);
		txBtn->setDoubleClickToOpenMenu(true);
		txBtn->checkBox()->setVisible(false);
		txBtn->button()->setVisible(true);

		MenuCombo *combo = new MenuCombo("DDS MODE", this);
		auto cb = combo->combo();
		combo->combo()->addItem("Disabled", 0);
		combo->combo()->addItem("One CW Tone", 1);
		combo->combo()->addItem("Two CW Tones", 2);
		combo->combo()->addItem("Independent I/Q Control", 3);
		connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
			auto mode = cb->itemData(idx).toInt();
			ddsModeStack->show(QString::number(mode));
		});

		txHeaderLay->addWidget(txBtn);
		txHeaderLay->addWidget(combo);

		txLay->addWidget(txHeader);
		txLay->addWidget(ddsModeStack);

		txsContainerLayout->addWidget(tx);
	}
	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	txsContainerLayout->addItem(spacer);
}

DdsDacAddon::~DdsDacAddon()
{

}
