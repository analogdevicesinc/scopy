#include "imuanalyzersettings.hpp"

using namespace scopy;

ImuAnalyzerSettings::ImuAnalyzerSettings(SceneRenderer *scRend, BubbleLevelRenderer *blRend, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget("Settings", QPen(Qt::blue), this);

	MenuCollapseSection *generalSettings = new MenuCollapseSection(
		"General Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, this);

	MenuCollapseSection *bubbleLevelSettings = new MenuCollapseSection(
		"2D Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, this);

	MenuCollapseSection *sceneRendererSettings = new MenuCollapseSection(
		"3D Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, this);

	QPushButton *cubeColorButton = new QPushButton("Select New Cube Color");
	sceneRendererSettings->contentLayout()->addWidget(cubeColorButton);

	connect(this, &ImuAnalyzerSettings::updateCubeColor, scRend, &SceneRenderer::updateCubeColor);
	connect(cubeColorButton, &QPushButton::clicked, [&]() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Choose a Color");
		if(color.isValid())
			emit updateCubeColor(color);
	});

	QPushButton *planeColorButton = new QPushButton("Select New Plane Color");
	sceneRendererSettings->contentLayout()->addWidget(planeColorButton);

	connect(this, &ImuAnalyzerSettings::updatePlaneColor, scRend, &SceneRenderer::updatePlaneColor);
	connect(planeColorButton, &QPushButton::clicked, [&]() {
		QColor color = QColorDialog::getColor(Qt::white, this, "Choose a Color");
		if(color.isValid())
			emit updatePlaneColor(color);
	});

	lay->addWidget(header);
	lay->addWidget(generalSettings);
	lay->addWidget(bubbleLevelSettings);
	lay->addWidget(sceneRendererSettings);

	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}
