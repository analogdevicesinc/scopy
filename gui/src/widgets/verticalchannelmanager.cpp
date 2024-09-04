#include <widgets/verticalchannelmanager.h>
#include <QScrollArea>

using namespace scopy;
VerticalChannelManager::VerticalChannelManager(QWidget *parent)
	: QWidget(parent)
{
	lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);

	setLayout(lay);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);

	QWidget *m_container = new QWidget(this);
	m_contLayout = new QVBoxLayout(m_container);
	m_contLayout->addSpacerItem(spacer);
	m_contLayout->setMargin(0);
	m_contLayout->setSpacing(6);
	m_container->setLayout(m_contLayout);

	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(m_container);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	lay->addWidget(m_scrollArea);
}

VerticalChannelManager::~VerticalChannelManager() {}

void VerticalChannelManager::add(QWidget *ch)
{
	int position = m_contLayout->indexOf(spacer);
	m_contLayout->insertWidget(position, ch);
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void VerticalChannelManager::addEnd(QWidget *ch)
{
	m_contLayout->addWidget(ch);
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void VerticalChannelManager::remove(QWidget *ch) { m_contLayout->removeWidget(ch); }

#include "moc_verticalchannelmanager.cpp"
