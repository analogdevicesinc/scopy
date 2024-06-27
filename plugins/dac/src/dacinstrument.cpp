#include "dacinstrument.h"
#include "dacdatamanager.h"
#include "dacdatamodel.h"

#include <QLoggingCategory>
#include <QHBoxLayout>
#include <gui/widgets/menucontrolbutton.h>
#include <stylehelper.h>


using namespace scopy;
Q_LOGGING_CATEGORY(CAT_DACINSTRUMENT, "DACInstrument")

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
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->topContainer()->setVisible(true);
	tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);
	lay->addWidget(tool);

	openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
//	rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();

	infoBtn = new InfoBtn(this);
	printBtn = new PrintBtn(this);

	devicesGroup = new QButtonGroup(this);
	devicesBtn = new MenuControlButton(this);
	devicesBtn->setName("Devices");
	devicesBtn->setOpenMenuChecksThis(true);
	devicesBtn->setDoubleClickToOpenMenu(true);
	devicesBtn->checkBox()->setVisible(false);
	devicesBtn->setChecked(true);

	deviceStack = new MapStackedWidget(this);
	tool->rightStack()->add(devicesMenuId, deviceStack);
	connect(devicesBtn->button(), &QAbstractButton::toggled, this, [=](bool b) {
		if(b)
			tool->requestMenu(devicesMenuId);
	});

	dacManagerStack = new MapStackedWidget(this);
	tool->addWidgetToCentralContainerHelper(dacManagerStack);

//	rightMenuBtnGrp->addButton(channelsBtn->button());

	connect(devicesBtn, &QPushButton::toggled, dynamic_cast<MenuHAnim*>(tool->leftContainer()),
		&MenuHAnim::toggleMenu);

	vcm = new VerticalChannelManager(this);
	tool->leftStack()->add(verticalChannelManagerId, vcm);

	tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	tool->addWidgetToTopContainerHelper(printBtn, TTA_LEFT);
	tool->addWidgetToBottomContainerHelper(devicesBtn, TTA_LEFT);

	setupDacDataManagers();

	for (DacDataManager *dac : m_dacDataManagers) {
		MenuControlButton *devBtn = addDevice(dac, vcm);
		vcm->add(devBtn);
	}

	//adc instrument
	//adddevice + add widgets to device stack + create buttons


//	connect(infoBtn, &QPushButton::clicked, this, &DacInstrument::startTutorial);

}

void DacInstrument::setupDacDataManagers()
{
	QStringList deviceList;
	QMap<QString, QStringList> devChannelMap;
	int devCount = iio_context_get_devices_count(m_conn->context());
	qDebug(CAT_DACINSTRUMENT) << " Found " << devCount << "devices";
	for(int i = 0; i < devCount; i++) {
		iio_device *dev = iio_context_get_device(m_conn->context(), i);
		QString dev_name = QString::fromLocal8Bit(iio_device_get_name(dev));

		qDebug(CAT_DACINSTRUMENT) << "Looking for scanelements in " << dev_name;
		QStringList channelList;
		for(int j = 0; j < iio_device_get_channels_count(dev); j++) {

			struct iio_channel *chn = iio_device_get_channel(dev, j);
			QString chn_name = QString::fromLocal8Bit(iio_channel_get_id(chn));
			qDebug(CAT_DACINSTRUMENT) << "Verify if " << chn_name << "is scan element";
			if(chn_name == "timestamp" /*|| chn_name == "accel_z" || chn_name =="accel_y"*/)
				continue;
			if(iio_channel_is_output(chn) && iio_channel_is_scan_element(chn)) {
				channelList.append(chn_name);
				deviceList.append(dev_name);
				DacDataModel *model = new DacDataModel(dev, this);
				m_dacDataManagers.append(new DacDataManager(model, dacManagerStack));
				break;
			}
		}
//		if(channelList.isEmpty())
//			continue;

//		devChannelMap.insert(dev_name, channelList);
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
	QString id = dev->getName() + QString::number(uuid++);
	dacManagerStack->add(id, dev);
//	deviceStack->add(id, dev->getWidget()); // add chnl menu
	connect(devBtn, &QPushButton::toggled, this, [=](bool b) {
		if(b) {
//			tool->requestMenu(devicesMenuId);
//			deviceStack->show(id);
			dacManagerStack->show(id);
		}
	});
	return devBtn;
}

DacInstrument::~DacInstrument() {

}
