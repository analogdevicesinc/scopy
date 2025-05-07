#include "imuanalyzerinterface.hpp"

using namespace scopy;

Q_DECLARE_METATYPE(rotation)

std::atomic<bool> m_runThread(false);

IMUAnalyzerInterface::IMUAnalyzerInterface(QString uri, QWidget *parent) : QWidget{parent}{
	qRegisterMetaType<rotation>("rotation");

	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	m_uri = uri;
	initIIODevice();

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->setRightContainerWidth(300);

	lay->addWidget(m_tool);

	m_infoBtn = new InfoBtn(this, false);
	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	m_sceneRender = new SceneRenderer(this);
	QTabWidget *tabWidget = new QTabWidget;
	tabWidget->addTab(m_sceneRender,"3D View");

	m_tool->addWidgetToCentralContainerHelper(tabWidget);

	m_rstPos = new MenuControlButton(this);
	m_rstPos->setName("Reset Position");
	m_rstPos->checkBox()->setVisible(false);
	m_rstPos->button()->setVisible(false);
	m_rstPos->setCheckable(false);
	m_tool->addWidgetToBottomContainerHelper(m_rstPos, TTA_LEFT);

	connect(m_rstPos, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetPos);

	m_rstView = new MenuControlButton(this);
	m_rstView->setName("Reset View");
	m_rstView->checkBox()->setVisible(false);
	m_rstView->button()->setVisible(false);
	m_rstView->setCheckable(false);
	m_tool->addWidgetToBottomContainerHelper(m_rstView, TTA_LEFT);

	connect(m_rstView, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetView);

	connect(this, &IMUAnalyzerInterface::generateRot, m_sceneRender, &SceneRenderer::setRot);

	connect(m_runBtn, &QPushButton::pressed,[](){
		m_runThread = !m_runThread;
	});

	t = std::thread(&IMUAnalyzerInterface::generateRotation, this);

	BubbleLevelRenderer *bubbleLevelRenderer = new BubbleLevelRenderer();
	tabWidget->addTab(bubbleLevelRenderer, "2D View");

	connect(this, &IMUAnalyzerInterface::generateRot, bubbleLevelRenderer, &BubbleLevelRenderer::setRot);

	m_gearBtn = new GearBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	m_gearBtn->setChecked(true);

	QString key = "Settings";
	m_settingsPanel = new ImuAnalyzerSettings(m_sceneRender, bubbleLevelRenderer, this);
	m_tool->rightStack()->add(key,m_settingsPanel);

	connect(m_gearBtn, &GearBtn::toggled, this, [=, this](bool toggled) {
		if(toggled)
			m_tool->requestMenu(key);

		m_tool->openRightContainerHelper(toggled);
	});
}

IMUAnalyzerInterface::~IMUAnalyzerInterface(){
	t.join();
}

void IMUAnalyzerInterface::generateRotation(){

	iio_channel *anglVelChX = iio_device_find_channel(m_device, "anglvel_x", false);
	iio_channel *anglVelChY = iio_device_find_channel(m_device, "anglvel_y", false);
	iio_channel *anglVelChZ = iio_device_find_channel(m_device, "anglvel_z", false);

	double anglVelGainX;
	iio_channel_attr_read_double(anglVelChX, "scale", &anglVelGainX);
	double anglVelGainY;
	iio_channel_attr_read_double(anglVelChY, "scale", &anglVelGainY);
	double anglVelGainZ;
	iio_channel_attr_read_double(anglVelChZ, "scale", &anglVelGainZ);

	double anglVelX, anglVelY, anglVelZ;

	m_rot = {0.0f, 0.0f, 0.0f};
	bool directionX = false, directionY = false;
	while(1){
		if (!m_runThread) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Light pause loop
			continue;
		}

		iio_channel_attr_read_double(anglVelChX, "raw", &anglVelX);
		iio_channel_attr_read_double(anglVelChY, "raw", &anglVelY);
		iio_channel_attr_read_double(anglVelChZ, "raw", &anglVelZ);

		m_rot.rotX += float(anglVelX * anglVelGainX / 2);
		m_rot.rotY += float(anglVelY * anglVelGainY / 2);
		m_rot.rotZ += float(anglVelZ * anglVelGainZ / 2);

		QMetaObject::invokeMethod(this, "generateRot", Qt::QueuedConnection,
					  Q_ARG(rotation, m_rot));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void IMUAnalyzerInterface::initIIODevice(){
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	for(int i = 0; i < iio_context_get_devices_count(conn->context()); i++) {
		m_device = iio_context_get_device(conn->context(), i);
		std::string name = iio_device_get_name(m_device);
		if(name.find("adis") != std::string::npos){
			return;
		}
	}
}

#include "moc_imuanalyzerinterface.cpp"
