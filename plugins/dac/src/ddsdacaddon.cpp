#include "ddsdacaddon.h"
#include "dacdatamodel.h"
#include "txnode.h"

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
	txsContainer->setLayout(txsContainerLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(txsContainer);
	m_layout->addWidget(scrollArea);

	for (int i = 0; i <  model->getDdsTxs().size(); i++) {
		TxNode *txNode = model->getDdsTxs().value(model->getDdsTxs().keys().at(i));
		QWidget *tx = new QWidget(this);
//		StyleHelper::BackgroundWidget(tx);
		QVBoxLayout *txLay = new QVBoxLayout(this);
		txLay->setMargin(10);
		txLay->setSpacing(10);
		txLay->setMargin(0);
		txLay->setSpacing(10);
		tx->setLayout(txLay);

		MapStackedWidget *ddsModeStack = new MapStackedWidget(this);
		QWidget *txChns = new QWidget(this);
		QHBoxLayout *txChnsLay = new QHBoxLayout(this);
		txChnsLay->setContentsMargins(0, 0, 10, 0);
		txChnsLay->setSpacing(10);
		txChns->setLayout(txChnsLay);
		StyleHelper::BackgroundWidget(txChns);

		ddsModeStack->add("0", txChns);

		auto txNodeChildren = txNode->getTones();
		int key = 0;

		for (TxNode *nodeChildren : txNodeChildren) {
			QWidget *chnIQ = new QWidget(this);
			QVBoxLayout *chnIqLay = new QVBoxLayout(this);
			chnIqLay->setMargin(10);
			chnIqLay->setSpacing(10);
			chnIQ->setLayout(chnIqLay);
			QLabel *chnIqLbl = new QLabel(this);
			chnIqLbl->setText("Channel " + nodeChildren->getUuid());

			QWidget *toneList = new QWidget(this);
			QHBoxLayout *toneListLay = new QHBoxLayout(this);
			toneListLay->setMargin(0);
			toneListLay->setSpacing(10);
			toneList->setLayout(toneListLay);
			chnIqLay->addWidget(chnIqLbl);
			chnIqLay->addWidget(toneList);

			for (int j = 0; j < nodeChildren->getTones().size(); j++){
				TxNode *nodeTone = nodeChildren->getTones().value(nodeChildren->getTones().keys().at(j));
				QWidget *tone = new QWidget(this);
				QVBoxLayout *toneLay = new QVBoxLayout(this);
				toneLay->setMargin(0);
				toneLay->setSpacing(10);
				tone->setLayout(toneLay);

				auto frequency = IIOWidgetBuilder()
						.channel(nodeTone->getChannel())
						.attribute("frequency").uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
						.parent(this).buildSingle();

				auto scale = IIOWidgetBuilder()
						.channel(nodeTone->getChannel())
						.attribute("scale").uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
						.parent(this).buildSingle();

				auto phase = IIOWidgetBuilder()
						.channel(nodeTone->getChannel())
						.attribute("phase").uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
						.parent(this).buildSingle();
				toneLay->addWidget(frequency);
				toneLay->addWidget(scale);
				toneLay->addWidget(phase);
				toneListLay->addWidget(tone);
			}
			txChnsLay->addWidget(chnIQ);
		}

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
