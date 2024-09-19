#include "dacinstrument.h"
#include "dacdatamanager.h"
#include "dac_logging_categories.h"

#include <QHBoxLayout>
#include <stylehelper.h>

using namespace scopy;
using namespace scopy::dac;
using namespace scopy::gui;

DacInstrument::DacInstrument(const Connection *conn, QWidget *parent)
	: QWidget(parent)
	, m_conn(conn)
{
	StyleHelper::GetInstance()->initColorMap();
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);
	tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->bottomContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->topContainer()->setVisible(true);
	tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);
	lay->addWidget(tool);

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
	rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();

	infoBtn = new InfoBtn(this);
	settingsBtn = new GearBtn(this);

	devicesGroup = new QButtonGroup(this);
	devicesBtn = new MenuControlButton(this);
	devicesBtn->setName("Devices");
	devicesBtn->setOpenMenuChecksThis(true);
	devicesBtn->setDoubleClickToOpenMenu(true);
	devicesBtn->checkBox()->setVisible(false);
	devicesBtn->setChecked(true);

	rightMenuBtnGrp->addButton(settingsBtn);
	rightMenuBtnGrp->addButton(devicesBtn->button());

	connect(infoBtn, &QPushButton::clicked, this, &DacInstrument::startTutorial);
	connect(devicesBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim *>(tool->leftContainer()),
		&MenuHAnim::toggleMenu);

	connect(settingsBtn, &QPushButton::toggled, this, [this](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});

	deviceStack = new MapStackedWidget(this);
	tool->rightStack()->add(devicesMenuId, deviceStack);
	connect(devicesBtn->button(), &QAbstractButton::toggled, this, [this](bool b) {
		if(b)
			tool->requestMenu(devicesMenuId);
	});

	dacManagerStack = new MapStackedWidget(this);
	tool->addWidgetToCentralContainerHelper(dacManagerStack);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(devicesBtn, TTA_LEFT);

	setupDacDataManagers();
	setupDacDataDeviceButtons();
}

void DacInstrument::startTutorial()
{
	QWidget *parent = Util::findContainingWindow(this);
	DacDataManager *currentDac = dynamic_cast<DacDataManager *>(dacManagerStack->currentWidget());
	if(currentDac) {
		parent = Util::findContainingWindow(currentDac);
	}
	m_dacBufferTutorial = new TutorialBuilder(this, ":/dacinstrument/tutorial_chapters.json", "dacbuffer", parent);
	m_dacBufferNonCyclicTutorial =
		new TutorialBuilder(this, ":/dacinstrument/tutorial_chapters.json", "dacbuffernoncyclic", parent);
	m_dacDdsTutorial = new TutorialBuilder(this, ":/dacinstrument/tutorial_chapters.json", "dacdds", parent);

	connect(m_dacBufferTutorial, &TutorialBuilder::finished, this, &DacInstrument::startBufferNonCyclicTutorial);
	connect(m_dacBufferNonCyclicTutorial, &TutorialBuilder::finished, this, &DacInstrument::startDdsTutorial);
	connect(m_dacBufferTutorial, &gui::TutorialBuilder::aborted, this, &DacInstrument::abortTutorial);

	startBufferTutorial();
}

void DacInstrument::runToggled(bool toggled)
{
	for(auto dac : m_dacDataManagers) {
		dac->runToggled(toggled);
	}
}

void DacInstrument::dacRunning(bool toggled)
{
	bool run = toggled;
	for(auto dac : m_dacDataManagers) {
		run = run || dac->isRunning();
	}
	Q_EMIT running(run);
}

void DacInstrument::startBufferTutorial()
{
	DacDataManager *currentDac = dynamic_cast<DacDataManager *>(dacManagerStack->currentWidget());
	if(currentDac) {
		if(currentDac->isBufferCapable()) {
			qInfo(CAT_DAC) << "Start Dac buffer tutorial";
			currentDac->toggleBufferMode();
			m_dacBufferTutorial->setTitle("Dac Buffer");
			m_dacBufferTutorial->start();
		} else {
			startDdsTutorial();
		}
	}
}

void DacInstrument::startBufferNonCyclicTutorial()
{
	DacDataManager *currentDac = dynamic_cast<DacDataManager *>(dacManagerStack->currentWidget());
	if(currentDac) {
		if(currentDac->isBufferCapable()) {
			qInfo(CAT_DAC) << "Start Dac Buffer non cyclic tutorial";
			currentDac->toggleCyclicBuffer(false);
			m_dacBufferNonCyclicTutorial->setTitle("Dac Buffer");
			m_dacBufferNonCyclicTutorial->start();
		}
	}
}

void DacInstrument::startDdsTutorial()
{
	DacDataManager *currentDac = dynamic_cast<DacDataManager *>(dacManagerStack->currentWidget());
	if(currentDac) {
		if(currentDac->isBufferCapable()) {
			currentDac->toggleCyclicBuffer(true);
		}
		if(currentDac->isDds()) {
			qInfo(CAT_DAC) << "Start Dac DDS tutorial";
			currentDac->toggleDdsMode();
			m_dacDdsTutorial->setTitle("Dac DDS");
			m_dacDdsTutorial->start();
		}
	}
}

void DacInstrument::abortTutorial()
{
	disconnect(m_dacBufferTutorial, &gui::TutorialBuilder::finished, this,
		   &DacInstrument::startBufferNonCyclicTutorial);
	disconnect(m_dacBufferNonCyclicTutorial, &gui::TutorialBuilder::finished, this,
		   &DacInstrument::startDdsTutorial);
}

void DacInstrument::setupDacDataManagers()
{
	QStringList deviceList;
	int devCount = iio_context_get_devices_count(m_conn->context());
	qDebug(CAT_DAC_INSTRUMENT) << " Found " << devCount << "devices";
	for(int i = 0; i < devCount; i++) {
		iio_device *dev = iio_context_get_device(m_conn->context(), i);
		QString dev_name = QString::fromLocal8Bit(iio_device_get_name(dev));

		qDebug(CAT_DAC_INSTRUMENT) << "Looking for scan elements in " << dev_name;
		QStringList channelList;
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {

			struct iio_channel *chn = iio_device_get_channel(dev, j);
			QString chn_name = QString::fromLocal8Bit(iio_channel_get_id(chn));
			qDebug(CAT_DAC_INSTRUMENT) << "Verify if " << chn_name << "is scan element";
			if(chn_name == "timestamp" /*|| chn_name == "accel_z" || chn_name =="accel_y"*/)
				continue;
			if(iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				channelList.append(chn_name);
				deviceList.append(dev_name);

				// Create a DataManager for each detected dac iio_device
				auto dm = new DacDataManager(dev, dacManagerStack);
				m_dacDataManagers.append(dm);
				connect(dm, &DacDataManager::running, this, &DacInstrument::dacRunning);

				auto name = dm->getName();
				if(tool->rightStack()->contains(name)) {
					name = name + QString::number(m_dacDataManagers.size() - 1);
				}

				// Connect the DacManager menu request for its stack
				connect(dm, &DacDataManager::requestMenu, [&, name]() { tool->requestMenu(name); });

				// Map the DacManager menu stack to its name
				tool->rightStack()->add(name, dm->getRightMenuStack());

				// Add all the DacManager control btns to the group
				auto menuBtns = dm->getMenuControlBtns();
				for(MenuControlButton *btn : qAsConst(menuBtns)) {
					devicesGroup->addButton(btn);
				}
				break;
			}
		}
	}
}

void DacInstrument::setupDacDataDeviceButtons()
{
	// If only one dac device available, no need to have a leftside manager
	if(m_dacDataManagers.size() == 1) {
		DacDataManager *dev = m_dacDataManagers.at(0);
		devicesBtn->setName(dev->getName());
		devicesBtn->setCheckable(false);
		addDeviceToStack(dev, devicesBtn);
		return;
	}

	vcm = new VerticalChannelManager(this);
	tool->leftContainer()->setVisible(true);
	tool->leftStack()->add(verticalChannelManagerId, vcm);
	for(DacDataManager *dac : qAsConst(m_dacDataManagers)) {
		MenuControlButton *devBtn = addDevice(dac, vcm);
		vcm->add(devBtn);
	}
}

MenuControlButton *DacInstrument::addDevice(DacDataManager *dev, QWidget *parent)
{
	auto devBtn = new MenuControlButton(parent);
	devBtn->setName(dev->getName());
	devBtn->checkBox()->setVisible(false);
	devBtn->setCheckable(true);
	devBtn->button()->setVisible(false);
	devBtn->setOpenMenuChecksThis(true);
	devBtn->setDoubleClickToOpenMenu(true);
	devicesGroup->addButton(devBtn);
	addDeviceToStack(dev, devBtn);
	return devBtn;
}

void DacInstrument::addDeviceToStack(DacDataManager *dev, MenuControlButton *btn)
{
	QString id = dev->getName() + QString::number(uuid++);
	dacManagerStack->add(id, dev);
	deviceStack->add(id, dev->getWidget());
	connect(btn, &QPushButton::toggled, this, [=, this](bool b) {
		if(b) {
			tool->requestMenu(devicesMenuId);
			deviceStack->show(id);
			dacManagerStack->show(id);
		}
	});
}

DacInstrument::~DacInstrument() {}
