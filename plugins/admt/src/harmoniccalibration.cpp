#include "harmoniccalibration.h"

#include <stylehelper.h>

using namespace scopy::admt;

HarmonicCalibration::HarmonicCalibration(struct iio_context *context, QWidget *parent)
{
    this->context = context;

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
    infoButton = new InfoBtn(this);
    runButton = new RunBtn(this);
    tool->addWidgetToTopContainerHelper(infoButton, TTA_LEFT);
    tool->addWidgetToTopContainerHelper(runButton, TTA_RIGHT);

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

    QStackedWidget *centralWidget = new QStackedWidget(this);
	tool->addWidgetToCentralContainerHelper(centralWidget);
}

HarmonicCalibration::~HarmonicCalibration() {}

void HarmonicCalibration::getRotationData()
{
    int devCount = iio_context_get_devices_count(context);
    auto s = std::to_string(devCount);
    QString qstr = QString::fromStdString(s);
    rotationLineEdit->setText(qstr);
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