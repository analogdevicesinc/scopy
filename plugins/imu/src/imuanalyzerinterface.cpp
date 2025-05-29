#include "imuanalyzerinterface.hpp"

using namespace scopy;

Q_DECLARE_METATYPE(data3P)

std::atomic<bool> m_runThread(false);

IMUAnalyzerInterface::IMUAnalyzerInterface(QString uri, QWidget *parent) : QWidget{parent}{
	qRegisterMetaType<data3P>("data3P");

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
	m_tool->centralContainer()->layout()->setSpacing(10);

	lay->addWidget(m_tool);

	m_dataV = new DataVisualizer();
	m_tool->addWidgetToCentralContainerHelper(m_dataV);
	m_dataV->hide();

	connect(this, &IMUAnalyzerInterface::updateValues, m_dataV, &DataVisualizer::updateValues);

	m_infoBtn = new InfoBtn(this, false);
	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	QTabWidget *tabWidget = new QTabWidget(this);
	m_tool->addWidgetToCentralContainerHelper(tabWidget);

	m_sceneRender = new SceneRenderer();
	tabWidget->addTab(m_sceneRender,"3D View");

	m_rstView = new MenuControlButton(this);
	m_rstView->setName("Reset View");
	m_rstView->checkBox()->setVisible(false);
	m_rstView->button()->setVisible(false);
	m_rstView->setCheckable(false);
	m_tool->addWidgetToBottomContainerHelper(m_rstView, TTA_LEFT);

	connect(m_rstView, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetView);

	m_measureBtn = new MenuControlButton(this);
	m_measureBtn->setName("Measure");
	m_measureBtn->checkBox()->setVisible(false);
	m_measureBtn->button()->setVisible(false);
	m_measureBtn->setCheckable(true);
	m_measureBtn->setChecked(false);
	m_tool->addWidgetToBottomContainerHelper(m_measureBtn, TTA_RIGHT);

	connect(m_measureBtn, &QPushButton::toggled, this, [=, this](bool toggled){
		if(toggled)
			m_dataV->show();
		else
			m_dataV->hide();
	});

	connect(this, &IMUAnalyzerInterface::generateRot, m_sceneRender, &SceneRenderer::setRot);

	connect(m_runBtn, &QPushButton::toggled,[=, this](bool toggled){
		m_runThread = !m_runThread;
		if(toggled){
			t = std::thread(&IMUAnalyzerInterface::generateRotation, this);
		} else {
			t.join();
		}
	});

	m_bubbleLevelRenderer = new BubbleLevelRenderer(tabWidget);
	tabWidget->addTab(m_bubbleLevelRenderer, "2D View");

	connect(this, &IMUAnalyzerInterface::generateRot, m_bubbleLevelRenderer, &BubbleLevelRenderer::setRot);

	m_gearBtn = new GearBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	m_gearBtn->setChecked(true);

	QString key = "Settings";
	m_settingsPanel = new ImuAnalyzerSettings(m_sceneRender, m_bubbleLevelRenderer, m_device);

	QScrollArea *scrollArea = new QScrollArea;
	scrollArea->setWidget(m_settingsPanel);
	scrollArea->setWidgetResizable(true);

	m_tool->rightStack()->add(key,scrollArea);

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

	iio_channel *linearAccChX = iio_device_find_channel(m_device, "accel_x", false);
	iio_channel *linearAccChY = iio_device_find_channel(m_device, "accel_y", false);
	iio_channel *linearAccChZ = iio_device_find_channel(m_device, "accel_z", false);

	iio_channel *tempCh = iio_device_find_channel(m_device, "temp0", false);

	double samplingFreq;
	iio_device_attr_read_double(m_device, "sampling_frequency", &samplingFreq);

	double linearAccGainX, linearAccGainY, linearAccGainZ;
		iio_channel_attr_read_double(linearAccChX, "scale", &linearAccGainX);
		iio_channel_attr_read_double(linearAccChY, "scale", &linearAccGainY);
		iio_channel_attr_read_double(linearAccChZ, "scale", &linearAccGainZ);

	double tempGain, tempOffset;
	if(tempCh != nullptr){
		iio_channel_attr_read_double(tempCh, "scale", &tempGain);
		iio_channel_attr_read_double(tempCh, "offset", &tempOffset);
	}

	double linearAccX, linearAccY, linearAccZ;
	double temp;

	while(m_runThread){

		iio_channel_attr_read_double(linearAccChX, "raw", &linearAccX);
		iio_channel_attr_read_double(linearAccChY, "raw", &linearAccY);
		iio_channel_attr_read_double(linearAccChZ, "raw", &linearAccZ);

		m_dist.dataX = float(linearAccX * linearAccGainX);
		m_dist.dataY = float(linearAccY * linearAccGainY);
		m_dist.dataZ = float(linearAccZ * linearAccGainZ);

		if(tempCh != nullptr){
			iio_channel_attr_read_double(tempCh, "raw", &temp);
			temp = temp * tempGain - tempOffset;
		}

		m_rot.dataX = atan2(-m_dist.dataX, sqrt(m_dist.dataY * m_dist.dataY + m_dist.dataZ * m_dist.dataZ)) * 180 / 3.14f;
		m_rot.dataY = atan2(m_dist.dataY, m_dist.dataZ)  * 180 / 3.14f;
		m_rot.dataZ = 0;

		QMetaObject::invokeMethod(this, "generateRot", Qt::QueuedConnection,
					  Q_ARG(data3P, m_rot));
		QMetaObject::invokeMethod(this, "updateValues", Qt::QueuedConnection,
					  Q_ARG(data3P, m_rot),Q_ARG(data3P, m_dist),Q_ARG(float, float(temp)));
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
