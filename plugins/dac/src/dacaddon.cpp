#include "dacaddon.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace scopy;
using namespace scopy::dac;
DacAddon::DacAddon(QWidget *parent)
	: QWidget(parent)
	, m_isRunning(false)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

DacAddon::~DacAddon() {}

void DacAddon::enable(bool enable) {}

QMap<QString, MenuControlButton *> DacAddon::getChannelBtns() { return m_channelBtns; }

QMap<QString, QWidget *> DacAddon::getChannelMenus() { return m_channelMenus; }

void DacAddon::setRunning(bool running) { m_isRunning = running; }

bool DacAddon::isRunning() { return m_isRunning; }
