#include "3Drenderer.hpp"

using namespace scopy;

SceneRenderer::SceneRenderer(QWidget *parent) : QWidget{parent} {
	view = new Qt3DExtras::Qt3DWindow();
	//view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
	container = QWidget::createWindowContainer(view);
	container->setMinimumSize(400, 300);
	container->setFocusPolicy(Qt::StrongFocus);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(container);
	layout->setMargin(0);

	       // Root entity
	rootEntity = new Qt3DCore::QEntity();

	       // Camera
	Qt3DRender::QCamera *camera = view->camera();
	camera->lens()->setPerspectiveProjection(45.0f, 16.f/9.f, 0.1f, 1000.0f);
	camera->setPosition(QVector3D(0, 0, 10.0f));
	camera->setViewCenter(QVector3D(0, 0, 0));

	       // Camera controls
	auto *camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
	camController->setCamera(camera);

	       // Cube mesh
	auto *mesh = new Qt3DExtras::QCuboidMesh();

	       // Transform
	transform = new Qt3DCore::QTransform();

	       // Material
	auto *material = new Qt3DExtras::QPhongMaterial();
	material->setDiffuse(QColor(QRgb(0xbeb32b)));

	       // Entity
	auto *cubeEntity = new Qt3DCore::QEntity(rootEntity);
	cubeEntity->addComponent(mesh);
	cubeEntity->addComponent(transform);
	cubeEntity->addComponent(material);

	       // Set root entity
	view->setRootEntity(rootEntity);

	       // Rotation animation
	// connect(&timer, &QTimer::timeout, this, &Scene3D::updateRotation);
	// timer.start(16); // ~60 FPS
}

void SceneRenderer::resetView(){
	view->camera()->setPosition(QVector3D(0, 0, 10.0f));
	view->camera()->setViewCenter(QVector3D(0, 0, 0));
}

#include "moc_3Drenderer.cpp"

