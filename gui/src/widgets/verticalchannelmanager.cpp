#include <widgets/verticalchannelmanager.h>

using namespace scopy;
VerticalChannelManager::VerticalChannelManager(QWidget *parent)
	: QWidget(parent)
{
	lay = new QVBoxLayout(this);
	setLayout(lay);
	lay->setMargin(0);
	lay->setSpacing(6);
	spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
	lay->addSpacerItem(spacer);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

VerticalChannelManager::~VerticalChannelManager() {}

void VerticalChannelManager::add(QWidget *ch)
{
	int position = lay->indexOf(spacer);
	lay->insertWidget(position, ch);
	ch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

#include "moc_verticalchannelmanager.cpp"
