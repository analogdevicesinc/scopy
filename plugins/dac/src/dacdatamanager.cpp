#include "dacdatamanager.h"
#include "dacdatamodel.h"
#include "ddsdacaddon.h"
#include "bufferdacaddon.h"

#include <stylehelper.h>
#include <gui/widgets/menuheader.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>

#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidget.h>

#include <QRandomGenerator>
#include <QLabel>
#include <QScrollArea>

using namespace scopy;
using namespace scopy::dac;
DacDataManager::DacDataManager(struct iio_device *dev, QWidget *parent)
	: QWidget(parent)
{
	m_model = new DacDataModel(dev, this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_layout = new QVBoxLayout();
	m_layout->setMargin(0);
	m_layout->setSpacing(5);
	setLayout(m_layout);

	rightMenuStack = new MapStackedWidget(this);
	dacAddonStack = new MapStackedWidget(this);

	MenuSectionWidget *modeSection = new MenuSectionWidget(this);
	m_mode = new MenuCombo("MODE", this);
	m_mode->setProperty("tutorial_name", "MODE_SELECTOR");
	StyleHelper::IIOComboBox(m_mode->combo());
	StyleHelper::BackgroundWidget(m_mode);
	auto cb = m_mode->combo();
	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this](int idx) {
		auto current = dynamic_cast<DacAddon *>(dacAddonStack->currentWidget());
		if(current) {
			current->enable(false);
		}

		auto mode = cb->itemData(idx).toInt();
		auto next = dynamic_cast<DacAddon *>(dacAddonStack->get(QString::number(mode)));
		if(next) {
			next->enable(true);
			Q_EMIT running(next->isRunning());
		}
		dacAddonStack->show(QString::number(mode));
	});

	setupDacMode("Disabled", DAC_DISABLED);
	setupDacMode("Buffer", DAC_BUFFER);
	setupDacMode("DDS", DAC_DDS);
	Q_EMIT m_mode->combo()->currentIndexChanged(0);
	modeSection->contentLayout()->addWidget(m_mode);

	m_color = StyleHelper::getColor("ScopyBlue");
	m_layout->addWidget(modeSection);
	m_layout->addWidget(dacAddonStack);
	m_layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

	cb->setCurrentIndex(DAC_BUFFER);

	// Setup menu widget
	m_widget = createMenu();
}

DacDataManager::~DacDataManager() {}

QString DacDataManager::getName() const { return m_model->getName(); }

QWidget *DacDataManager::getWidget() { return m_widget; }

QColor DacDataManager::getColor() const { return m_color; }

void DacDataManager::setColor(QColor newColor) { m_color = newColor; }

void DacDataManager::setupDdsDac() {}

QWidget *DacDataManager::createAttrMenu(QWidget *parent)
{
	MenuSectionWidget *attrContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr = new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE,
							    MenuCollapseSection::MHW_BASEWIDGET, attrContainer);
	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder(attr).device(m_model->getDev()).buildAll();

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10);
	layout->setMargin(0);

	for(auto w : attrWidgets) {
		layout->addWidget(w);
	}

	attr->contentLayout()->addLayout(layout);
	attrContainer->contentLayout()->addWidget(attr);
	attr->header()->setChecked(true);
	return attrContainer;
}

QWidget *DacDataManager::createMenu()
{
	QWidget *w = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea();
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout(wScroll);
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(getName(), m_color, w);
	QWidget *attrMenu = createAttrMenu(w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(attrMenu);

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}

void DacDataManager::setupDacMode(QString mode_name, unsigned int mode)
{
	if(!m_model->isDds() && (mode == DAC_DDS))
		return;
	if(!m_model->isBufferCapable() && (mode == DAC_BUFFER))
		return;
	DacAddon *dac;
	switch(mode) {
	case DAC_DDS:
		dac = new DdsDacAddon(m_model, this);
		break;
	case DAC_BUFFER:
		dac = new BufferDacAddon(m_model, this);
		break;
	default:
		dac = new DacAddon(this);
		break;
	}
	auto channelBtns = dac->getChannelBtns();
	auto channelMenus = dac->getChannelMenus();
	for(auto btn : qAsConst(channelBtns)) {
		QString uuid = channelBtns.key(btn);
		QWidget *menu = channelMenus.value(uuid);
		if(menu) {
			rightMenuStack->add(uuid, menu);
		}
		m_menuControlBtns.push_back(btn);
	}
	connect(dac, &DacAddon::requestChannelMenu, this, &DacDataManager::handleChannelMenuRequest);
	connect(dac, &DacAddon::running, this, &DacDataManager::running);
	dacAddonStack->add(QString::number(mode), dac);
	m_mode->combo()->addItem(mode_name, mode);
}

void DacDataManager::handleChannelMenuRequest(QString uuid)
{
	rightMenuStack->show(uuid);
	Q_EMIT requestMenu();
}

MapStackedWidget *DacDataManager::getRightMenuStack() const { return rightMenuStack; }

QList<MenuControlButton *> DacDataManager::getMenuControlBtns() const { return m_menuControlBtns; }

bool DacDataManager::isBufferCapable() const { return m_model->isBufferCapable(); }

bool DacDataManager::isDds() const { return m_model->isDds(); }

void DacDataManager::toggleCyclicBuffer(bool toggled)
{
	if(isBufferCapable()) {
		auto dac = dynamic_cast<BufferDacAddon *>(dacAddonStack->currentWidget());
		if(!dac) {
			return;
		}
		Q_EMIT dac->toggleCyclic(toggled);
	}
}

void DacDataManager::toggleBufferMode()
{
	if(isBufferCapable()) {
		m_mode->combo()->setCurrentIndex(DAC_BUFFER);
	}
}

void DacDataManager::toggleDdsMode()
{
	if(isDds()) {
		m_mode->combo()->setCurrentIndex(DAC_DDS);
	}
}

void DacDataManager::runToggled(bool toggled)
{
	dynamic_cast<DacAddon *>(dacAddonStack->currentWidget())->setRunning(toggled);
}

bool DacDataManager::isRunning() { return dynamic_cast<DacAddon *>(dacAddonStack->currentWidget())->isRunning(); }
