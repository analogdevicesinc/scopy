#include "imuanalyzerinterface.hpp"

using namespace scopy;

IMUAnalyzerInterface::IMUAnalyzerInterface(QWidget *parent) : QWidget{parent}{
    QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	setLayout(lay);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->leftContainer()->setVisible(true);
	m_tool->rightContainer()->setVisible(true);
	m_tool->bottomContainer()->setVisible(true);

	lay->addWidget(m_tool);

	m_infoBtn = new InfoBtn(this, false);
	m_tool->addWidgetToTopContainerHelper(m_infoBtn, TTA_LEFT);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
}

#include "moc_imuanalyzerinterface.cpp"
