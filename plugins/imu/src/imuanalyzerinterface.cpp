#include "imuanalyzerinterface.hpp"

using namespace scopy;

Q_DECLARE_METATYPE(rotation)

std::atomic<bool> m_runThread(false);

void IMUAnalyzerInterface::generateRotation(){
	m_rot = {0.0f, 0.0f, 0.0f};
	bool direction = false;
	while(1){
	if (!m_runThread) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Light pause loop
		continue;
	}

	for(int i = 0; i < 180; i++){
		if(!m_runThread) break;
		if(m_rot.rotX == 0 || m_rot.rotX == 90) direction = !direction;
		if(direction) m_rot.rotX = m_rot.rotX + 1.0f;
		else m_rot.rotX = m_rot.rotX - 1.0f;

		QMetaObject::invokeMethod(this, "generateRot", Qt::QueuedConnection,
					  Q_ARG(rotation, m_rot));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	for(int i = 0; i < 180; i++){
		if(!m_runThread) break;
		if(m_rot.rotY == 0 || m_rot.rotY == 90) direction = !direction;
		if(direction) m_rot.rotY = m_rot.rotY + 1.0f;
		else m_rot.rotY = m_rot.rotY - 1.0f;

		QMetaObject::invokeMethod(this, "generateRot", Qt::QueuedConnection,
					  Q_ARG(rotation, m_rot));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	}
}

IMUAnalyzerInterface::IMUAnalyzerInterface(QWidget *parent) : QWidget{parent}{
	qRegisterMetaType<rotation>("rotation");

	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(false);
	m_tool->bottomContainer()->setVisible(true);

	lay->addWidget(m_tool);

	m_infoBtn = new InfoBtn(this, false);
	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	m_sceneRender = new SceneRenderer(this);
	QTabWidget *tabWidget = new QTabWidget;
	tabWidget->addTab(m_sceneRender,"3D View");
	// tabWidget->setTabPosition(QTabWidget::West);

	m_tool->addWidgetToCentralContainerHelper(tabWidget);

	m_rstPos = new QPushButton(this);
	m_rstPos->setText("Reset Position");
	m_tool->addWidgetToBottomContainerHelper(m_rstPos, TTA_LEFT);

	connect(m_rstPos, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetPos);

	m_rstView = new QPushButton(this);
	m_rstView->setText("Reset View");
	m_tool->addWidgetToBottomContainerHelper(m_rstView, TTA_LEFT);

	connect(m_rstView, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetView);

	connect(this, &IMUAnalyzerInterface::generateRot, m_sceneRender, &SceneRenderer::setRot);

	connect(m_runBtn, &QPushButton::pressed,[](){
		m_runThread = !m_runThread;
	});

	t = std::thread(&IMUAnalyzerInterface::generateRotation, this);

	BubbleLevelRenderer *bubbleLevelRenderer = new BubbleLevelRenderer();
	tabWidget->addTab(bubbleLevelRenderer, "2D View");

}

IMUAnalyzerInterface::~IMUAnalyzerInterface(){
	t.join();
}

#include "moc_imuanalyzerinterface.cpp"
