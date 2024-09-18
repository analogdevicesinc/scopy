#include "browsemenu/instrumentmenu.h"
#include "qdebug.h"

using namespace scopy;

InstrumentMenu::InstrumentMenu(QWidget *parent)
	: QWidget(parent)
	, m_btnGroup(new QButtonGroup(this))
{
	m_uuid = 0;
	QVBoxLayout *lay = new QVBoxLayout();
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

	m_scroll = new QScrollArea(parent);
	QWidget *wScroll = new QWidget(m_scroll);

	m_layScroll = new QVBoxLayout();
	m_layScroll->setMargin(0);
	m_layScroll->setSpacing(10);

	wScroll->setLayout(m_layScroll);
	m_scroll->setWidgetResizable(true);
	m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scroll->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);

	m_scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	setLayout(lay);

	m_spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layScroll->addSpacerItem(m_spacer);

	lay->addWidget(m_scroll);
}

InstrumentMenu::~InstrumentMenu() { qInfo() << "Instrument removed"; }

void InstrumentMenu::add(QWidget *w)
{
	int spacerIndex = m_layScroll->indexOf(m_spacer);
	m_layScroll->insertWidget(spacerIndex, w);
	m_uuid++;
}

void InstrumentMenu::add(int index, QString itemId, QWidget *w)
{
	m_widgetMap.insert(itemId, w);
	if(index < 0) {
		add(w);
	} else {
		add(index, w);
	}
}

void InstrumentMenu::add(int index, QWidget *w)
{
	m_layScroll->insertWidget(index, w);
	m_uuid++;
}
void InstrumentMenu::remove(QWidget *w)
{
	m_widgetMap.remove(widgetName(w));
	m_layScroll->removeWidget(w);
}

int InstrumentMenu::indexOf(QWidget *w) { return m_layScroll->indexOf(w); }

void InstrumentMenu::colapseAll()
{
	for(QWidget *w : qAsConst(m_widgetMap)) {
		Collapsable *c = dynamic_cast<Collapsable *>(w);
		if(c != nullptr) {
			c->setCollapsed(true);
		}
	}
}

QButtonGroup *InstrumentMenu::btnGroup() const { return m_btnGroup; }

QString InstrumentMenu::widgetName(QWidget *w) { return m_widgetMap.key(w, ""); }
