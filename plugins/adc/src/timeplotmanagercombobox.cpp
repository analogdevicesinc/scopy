#include "timeplotmanagercombobox.h"
#include "timeplotcomponentchannel.h"
#include <menusectionwidget.h>
using namespace scopy;
using namespace scopy::adc;

TimePlotManagerCombobox::TimePlotManagerCombobox(TimePlotManager *man, ChannelComponent *c, QWidget *parent)  : QWidget(parent) {
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	MenuSectionWidget *sec = new MenuSectionWidget(this);
	m_mcombo = new MenuCombo("PLOT", sec);
	m_combo = m_mcombo->combo();
	m_man = man;
	m_ch = c;

	// add all plots from manager
	for( TimePlotComponent* plt : man->plots()) {
		addPlot(plt);
	}

	// select current plot in combo
	uint32_t uuid = c->plotChannelCmpt()->m_plotComponent->uuid();
	m_combo->setCurrentIndex(findIndexFromUuid(uuid));

	connect(m_combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx){
		uint32_t uuid = m_combo->itemData(idx).toULongLong();
		man->moveChannel(m_ch, uuid);
		man->replot();
	});
	lay->addWidget(sec);
	sec->contentLayout()->addWidget(m_mcombo);

}

TimePlotManagerCombobox::~TimePlotManagerCombobox() {}

void TimePlotManagerCombobox::renamePlotSlot() {
	TimePlotComponent* plt = dynamic_cast<TimePlotComponent*>(QObject::sender());
	renamePlot(plt);
}

void TimePlotManagerCombobox::addPlot(TimePlotComponent *p) {
	m_combo->addItem(p->name(),p->uuid());
	connect(p, &TimePlotComponent::nameChanged, this, &TimePlotManagerCombobox::renamePlotSlot);

}

void TimePlotManagerCombobox::removePlot(TimePlotComponent *p) {
	int idx = findIndexFromUuid(p->uuid());
	m_combo->removeItem(idx);
	disconnect(p, &TimePlotComponent::nameChanged, this, &TimePlotManagerCombobox::renamePlotSlot);
}

void TimePlotManagerCombobox::renamePlot(TimePlotComponent *p) {
	int idx = findIndexFromUuid(p->uuid());
	m_combo->setItemText(idx, p->name());
}

int TimePlotManagerCombobox::findIndexFromUuid(uint32_t uuid) {
	for(int i = 0; i < m_combo->count();i++) {
		if(uuid == m_combo->itemData(i)) {
			return i;
		}
	}
	return -1;
}
