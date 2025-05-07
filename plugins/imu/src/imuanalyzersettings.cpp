#include "imuanalyzersettings.hpp"

using namespace scopy;

ImuAnalyzerSettings::ImuAnalyzerSettings(SceneRenderer *scRend, BubbleLevelRenderer *blRend, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(10);
	setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget("Settings", QPen(Qt::blue), this);

	MenuSectionWidget *generalSettings = new MenuSectionWidget(this);
	MenuCollapseSection *generalSettingsWidget = new MenuCollapseSection(
		"General Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, generalSettings);

	MenuSectionWidget *bubbleLevelSettings = new MenuSectionWidget(this);
	MenuCollapseSection *bubbleLevelSettingsWidget = new MenuCollapseSection(
		"2D Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, bubbleLevelSettings);

	MenuCombo *displayPoints = new MenuCombo("Display Points");
	displayPoints->combo()->addItem("XY");
	displayPoints->combo()->addItem("XZ");
	displayPoints->combo()->addItem("YZ");
	displayPoints->combo()->setCurrentIndex(0);

	connect(this, &ImuAnalyzerSettings::updateDisplayPoints, blRend, &BubbleLevelRenderer::setDisplayPoints);
	connect(displayPoints->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		emit updateDisplayPoints(displayPoints->combo()->itemText(idx));
	});

	bubbleLevelSettingsWidget->contentLayout()->addWidget(displayPoints);

	MenuSectionWidget *sceneRendererSettings = new MenuSectionWidget(this);
	MenuCollapseSection *sceneRendererSettingsWidget = new MenuCollapseSection(
		"3D Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, sceneRendererSettings);

	QPushButton *cubeColorButton = new QPushButton("Select New Cube Color");
	sceneRendererSettingsWidget->contentLayout()->addWidget(cubeColorButton);

	connect(this, &ImuAnalyzerSettings::updateCubeColor, scRend, &SceneRenderer::updateCubeColor);
	connect(cubeColorButton, &QPushButton::clicked, [&]() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Choose a Color");
		if(color.isValid())
			emit updateCubeColor(color);
	});

	QPushButton *planeColorButton = new QPushButton("Select New Plane Color");
	sceneRendererSettingsWidget->contentLayout()->addWidget(planeColorButton);

	connect(this, &ImuAnalyzerSettings::updatePlaneColor, scRend, &SceneRenderer::updatePlaneColor);
	connect(planeColorButton, &QPushButton::clicked, [&]() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Choose a Color");
		if(color.isValid())
			emit updatePlaneColor(color);
	});

	generalSettings->layout()->addWidget(generalSettingsWidget);
	bubbleLevelSettings->layout()->addWidget(bubbleLevelSettingsWidget);
	sceneRendererSettings->layout()->addWidget(sceneRendererSettingsWidget);

	lay->addWidget(header);
	lay->addWidget(generalSettings);
	lay->addWidget(bubbleLevelSettings);
	lay->addWidget(sceneRendererSettings);

	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
