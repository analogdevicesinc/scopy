#include "3Drenderer.hpp"

using namespace scopy;

Qt3DRender::QMaterial* createWireframeMaterial(Qt3DCore::QNode* parent = nullptr) {
	// Create a new material
	auto* material = new Qt3DRender::QMaterial(parent);

	       // Shader
	auto* shader = new Qt3DRender::QShaderProgram();
	shader->setVertexShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl("qrc:/shaders/basic.vert")));
	shader->setFragmentShaderCode(Qt3DRender::QShaderProgram::loadSource(QUrl("qrc:/shaders/basic.frag")));

	       // Render pass
	auto* renderPass = new Qt3DRender::QRenderPass();
	renderPass->setShaderProgram(shader);

	       // Polygon mode set to line (wireframe)
	// auto* polygonMode = new QPolygonMode();
	// polygonMode->setMode(QPolygon::Line);

	auto* renderStateSet = new Qt3DRender::QRenderStateSet();
	//renderStateSet->addRenderState(polygonMode);
	//renderPass->setRenderStateSet(renderStateSet);

	       // Technique
	auto* technique = new Qt3DRender::QTechnique();
	technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
	technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
	technique->graphicsApiFilter()->setMajorVersion(3);
	technique->graphicsApiFilter()->setMinorVersion(1);
	technique->addRenderPass(renderPass);

	       // Effect
	auto* effect = new Qt3DRender::QEffect();
	effect->addTechnique(technique);

	       // Set the effect on the material
	material->setEffect(effect);

	return material;
}

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
	camera->setPosition(QVector3D(0, 2.5f, 5.0f));
	camera->setViewCenter(QVector3D(0, 0, 0));

	// Camera controls
	auto *camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
	camController->setCamera(camera);

	//Initialize cube

	// Cube mesh
	auto *cubeMesh = new Qt3DExtras::QCuboidMesh();

	 // Transform
	cubeTransform = new Qt3DCore::QTransform();
	cubeTransform->setScale(2.0f);

	// Material
	m_cubeMaterial = new Qt3DExtras::QPhongMaterial();
	m_cubeMaterial->setDiffuse(m_cubeColor);

	// Entity
	cubeEntity = new Qt3DCore::QEntity(rootEntity);
	cubeEntity->addComponent(cubeMesh);
	cubeEntity->addComponent(cubeTransform);
	cubeEntity->addComponent(m_cubeMaterial);

	//Add line axes
	auto *planeMesh = new Qt3DExtras::QPlaneMesh(rootEntity);
	planeMesh->setWidth(400);
	planeMesh->setHeight(300);

	m_planeMaterial = new Qt3DExtras::QPhongMaterial();
	m_planeMaterial->setDiffuse(m_planeColor);

	 // Transform
	planeTransform = new Qt3DCore::QTransform();
	planeTransform->setTranslation((QVector3D(0.0f,-1.0f,0.0f)));

	auto meshEntity = new Qt3DCore::QEntity(rootEntity);
	meshEntity->addComponent(planeMesh);
	meshEntity->addComponent(planeTransform);
	meshEntity->addComponent(m_planeMaterial);

	// Set root entity
	view->setRootEntity(rootEntity);
}

void SceneRenderer::resetView(){
	view->camera()->lens()->setPerspectiveProjection(45.0f, 16.f/9.f, 0.1f, 1000.0f);
	view->camera()->setPosition(QVector3D(0, 2.5f, 5.0f));
	view->camera()->setViewCenter(QVector3D(0, 0, 0));
	view->camera()->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}

void SceneRenderer::resetPos(){
	setRot(rotation{0.0f, 0.0f, 0.0f});
}

void SceneRenderer::setRot(rotation rot){
	cubeTransform->setRotationX(rot.rotX);
	cubeTransform->setRotationY(rot.rotY);
	cubeTransform->setRotationZ(rot.rotZ);
}

void SceneRenderer::updateCubeColor(QColor color){
	m_cubeMaterial->setDiffuse(color);
}

void SceneRenderer::updatePlaneColor(QColor color){
	m_planeMaterial->setDiffuse(color);
}


#include "moc_3Drenderer.cpp"

