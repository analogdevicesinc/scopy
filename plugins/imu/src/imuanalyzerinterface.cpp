#include "imuanalyzerinterface.hpp"

using namespace scopy;

IMUAnalyzerInterface::IMUAnalyzerInterface(QWidget *parent) : QWidget{parent}{
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
	m_tool->addWidgetToCentralContainerHelper(m_sceneRender);

	m_rstPos = new QPushButton(this);
	m_rstPos->setText("Reset Position");
	m_tool->addWidgetToBottomContainerHelper(m_rstPos, TTA_LEFT);

	m_rstView = new QPushButton(this);
	m_rstView->setText("Reset View");
	m_tool->addWidgetToBottomContainerHelper(m_rstView, TTA_LEFT);

	connect(m_rstView, &QPushButton::clicked, m_sceneRender, &SceneRenderer::resetView);
}

#include "moc_imuanalyzerinterface.cpp"
