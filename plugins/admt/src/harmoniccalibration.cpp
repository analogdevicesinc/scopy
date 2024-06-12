#include "harmoniccalibration.h"

#include <stylehelper.h>

using namespace scopy;
using namespace scopy::grutil;

HarmonicCalibration::HarmonicCalibration(PlotProxy *proxy, QWidget *parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setMargin(0);
	setLayout(lay);
    tool = new ToolTemplate(this);
	tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tool->topContainer()->setVisible(true);
	tool->leftContainer()->setVisible(true);
	tool->rightContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);
	lay->addWidget(tool);
    tool->setLeftContainerWidth(210);
	tool->setRightContainerWidth(300);
	tool->setTopContainerHeight(100);
	tool->setBottomContainerHeight(90);

    openLastMenuBtn = new OpenLastMenuBtn(dynamic_cast<MenuHAnim *>(tool->rightContainer()), true, this);
    rightMenuBtnGrp = dynamic_cast<OpenLastMenuBtn *>(openLastMenuBtn)->getButtonGroup();
    
    tool->openBottomContainerHelper(false);
	tool->openTopContainerHelper(false);

    settingsButton = new GearBtn(this);
    infoButton = new InfoBtn(this);
    runButton = new RunBtn(this);

    tool->addWidgetToTopContainerMenuControlHelper(openLastMenuBtn, TTA_RIGHT);
	tool->addWidgetToTopContainerMenuControlHelper(settingsButton, TTA_LEFT);

    tool->addWidgetToTopContainerHelper(infoButton, TTA_LEFT);
    tool->addWidgetToTopContainerHelper(runButton, TTA_RIGHT);

    plotAddon = dynamic_cast<GRTimePlotAddon *>(proxy->getPlotAddon());
	tool->addWidgetToCentralContainerHelper(plotAddon->getWidget());

    plotAddonSettings = dynamic_cast<GRTimePlotAddonSettings *>(proxy->getPlotSettings());
	rightMenuBtnGrp->addButton(settingsButton);

    QString settingsMenuId = plotAddonSettings->getName() + QString(uuid++);
	tool->rightStack()->add(settingsMenuId, plotAddonSettings->getWidget());
	connect(settingsButton, &QPushButton::toggled, this, [=, this](bool b) {
		if(b)
			tool->requestMenu(settingsMenuId);
	});

    leftWidget = new QWidget(this);
    leftLayout = new QVBoxLayout(this);
    leftLayout->setMargin(0);
	leftLayout->setSpacing(10);
    leftWidget->setLayout(leftLayout);
    tool->leftStack()->add("left", leftWidget);

    header = new MenuHeaderWidget("ADMT4000", QPen(StyleHelper::getColor("ScopyBlue")), leftWidget);
    leftLayout->addWidget(header);

    scrollArea = new QScrollArea(leftWidget);
    scrollArea->setWidgetResizable(true);
    leftLayout->addWidget(scrollArea);

    leftBody = new QWidget(scrollArea);
    leftBodyLayout = new QVBoxLayout(leftBody);
    leftBodyLayout->setMargin(0);
    leftBodyLayout->setSpacing(10);
    leftBody->setLayout(leftBodyLayout);

    rotationSection = new MenuSectionWidget(leftWidget);
    countSection = new MenuSectionWidget(leftWidget);
    angleSection = new MenuSectionWidget(leftWidget);

    rotationCollapse = new MenuCollapseSection("ROTATION", MenuCollapseSection::MHCW_NONE, rotationSection);
    countCollapse = new MenuCollapseSection("COUNT", MenuCollapseSection::MHCW_NONE, countSection);
    angleCollapse = new MenuCollapseSection("ANGLE", MenuCollapseSection::MHCW_NONE, angleSection);
    rotationCollapse->contentLayout()->setSpacing(10);
    countCollapse->contentLayout()->setSpacing(10);
    angleCollapse->contentLayout()->setSpacing(10);

    rotationLineEdit = new QLineEdit(rotationCollapse);
    countLineEdit = new QLineEdit(countCollapse);
    angleLineEdit = new QLineEdit(angleCollapse);
    StyleHelper::MenuLineEdit(rotationLineEdit, "rotationEdit");
    StyleHelper::MenuLineEdit(countLineEdit, "countEdit");
    StyleHelper::MenuLineEdit(angleLineEdit, "angleEdit");

    getRotationButton = new QPushButton("Get Rotation", rotationCollapse);
    getCountButton = new QPushButton("Get Count", countCollapse);
    getAngleButton = new QPushButton("Get Angle", angleCollapse);
    StyleHelper::BlueButton(getRotationButton, "rotationButton");
    StyleHelper::BlueButton(getCountButton, "countButton");
    StyleHelper::BlueButton(getAngleButton, "angleButton");
    QObject::connect(getRotationButton, &QPushButton::clicked, this, &HarmonicCalibration::getRotationData);
    QObject::connect(getCountButton, &QPushButton::clicked, this, &HarmonicCalibration::getCountData);
    QObject::connect(getAngleButton, &QPushButton::clicked, this, &HarmonicCalibration::getAngleData);

    rotationCollapse->contentLayout()->addWidget(rotationLineEdit);
    rotationCollapse->contentLayout()->addWidget(getRotationButton);
    rotationSection->contentLayout()->addWidget(rotationCollapse);
    countCollapse->contentLayout()->addWidget(countLineEdit);
    countCollapse->contentLayout()->addWidget(getCountButton);
    countSection->contentLayout()->addWidget(countCollapse);
    angleCollapse->contentLayout()->addWidget(angleLineEdit);
    angleCollapse->contentLayout()->addWidget(getAngleButton);
    angleSection->contentLayout()->addWidget(angleCollapse);

    leftBodyLayout->addWidget(rotationSection);
    leftBodyLayout->addWidget(countSection);
    leftBodyLayout->addWidget(angleSection);

    // QStackedWidget *centralWidget = new QStackedWidget(this);
	// tool->addWidgetToCentralContainerHelper(centralWidget);
}

HarmonicCalibration::~HarmonicCalibration() {}

void HarmonicCalibration::getRotationData()
{
    rotationLineEdit->setText("test");
}

void HarmonicCalibration::getCountData()
{
    countLineEdit->setText("test");
}

void HarmonicCalibration::getAngleData()
{
    angleLineEdit->setText("test");
}

QStringList HarmonicCalibration::getDeviceList(iio_context *context)
{
    QStringList deviceList;
    int devCount = iio_context_get_devices_count(context);
    return deviceList;
}