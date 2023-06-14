#include "tooltemplate.h"
#include "ui_tooltemplate.h"

using namespace scopy;
ToolTemplate::ToolTemplate(QWidget *parent) : QWidget(parent)
{
	m_ui = new Ui::ToolTemplate();
	m_ui->setupUi(this);
	m_ui->bottomContainer->setVisible(false);
	m_ui->topContainer->setVisible(false);
	m_ui->leftContainer->setVisible(false);
	m_ui->rightContainer->setVisible(false);
	m_leftStack = new MapStackedWidget(m_ui->leftContainer);
	m_rightStack = new MapStackedWidget(m_ui->rightContainer);

	m_ui->leftContainer->layout()->addWidget(m_leftStack);
	m_ui->rightContainer->layout()->addWidget(m_rightStack);
	m_leftStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_rightStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ToolTemplate::~ToolTemplate()
{
	delete m_ui;
}

QWidget *ToolTemplate::bottomContainer()
{
	return m_ui->bottomContainer;
}

QWidget *ToolTemplate::topContainer()
{
	return m_ui->topContainer;
}

QWidget *ToolTemplate::rightContainer()
{
	return m_ui->rightContainer;
}

QWidget *ToolTemplate::topContainerMenuControl()
{
	return m_ui->topContainerMenuControl;
}

QWidget *ToolTemplate::centralContainer()
{
	return m_ui->centralContainer;
}

MapStackedWidget *ToolTemplate::leftStack()
{
	return m_leftStack;
}

MapStackedWidget *ToolTemplate::rightStack()
{
	return m_rightStack;
}

QWidget *ToolTemplate::leftContainer()
{
	return m_ui->leftContainer;
}

void ToolTemplate::setLeftContainerWidth(int w)
{
	dynamic_cast<MenuAnim*>(m_ui->leftContainer)->setAnimMinWidth(0);
	dynamic_cast<MenuAnim*>(m_ui->leftContainer)->setAnimMaxWidth(w);
	m_ui->leftContainer->setMaximumWidth(w);
	m_ui->leftContainer->setMinimumWidth(w);
}

void ToolTemplate::setRightContainerWidth(int w)
{
	m_ui->topContainerMenuControl->setMaximumWidth(w);
	m_ui->topContainerMenuControl->setMinimumWidth(w);
	dynamic_cast<MenuAnim*>(m_ui->rightContainer)->setAnimMinWidth(0);
	dynamic_cast<MenuAnim*>(m_ui->rightContainer)->setAnimMaxWidth(w);
	m_ui->rightContainer->setMaximumWidth(w);
	m_ui->rightContainer->setMinimumWidth(w);
}

void ToolTemplate::openLeftContainerHelper(bool open)
{
	m_ui->leftContainer->toggleMenu(open);
}

void ToolTemplate::openRightContainerHelper(bool open)
{
	m_ui->rightContainer->toggleMenu(open);
}

void ToolTemplate::addWidgetToTopContainerHelper(QWidget *w, ToolTemplateAlignment a)
{
	auto lay1 = dynamic_cast<QHBoxLayout*>(topContainer()->layout());
	Q_ASSERT(lay1 != nullptr);

	auto idx = lay1->indexOf(m_ui->topContainerSpacer);
	int offset;
	if(a == TTA_LEFT) {
		lay1->insertWidget(idx,w);
	} else {
		lay1->insertWidget(-1,w);
	}
}

void ToolTemplate::addWidgetToTopContainerMenuControlHelper(QWidget *w, ToolTemplateAlignment a) {
	auto lay1 = dynamic_cast<QHBoxLayout*>(topContainerMenuControl()->layout());
	Q_ASSERT(lay1 != nullptr);
	auto idx = lay1->indexOf(m_ui->topContainerMenuControlSpacer);
	int offset;
	if(a == TTA_LEFT) {
		lay1->insertWidget(idx,w);
	} else {
		lay1->insertWidget(-1,w);
	}	
}

void ToolTemplate::addWidgetToBottomContainerHelper(QWidget *w, ToolTemplateAlignment a)
{
	auto lay1 = dynamic_cast<QHBoxLayout*>(bottomContainer()->layout());
	Q_ASSERT(lay1 != nullptr);
	auto idx = lay1->indexOf(m_ui->bottomContainerSpacer);
	int offset;
	if(a == TTA_LEFT) {
		lay1->insertWidget(idx,w);
	} else {
		lay1->insertWidget(-1,w);
	}

}

void ToolTemplate::addWidgetToCentralContainerHelper(QWidget *w)
{
	auto lay1 = dynamic_cast<QVBoxLayout*>(centralContainer()->layout());
	Q_ASSERT(lay1 != nullptr);

	lay1->addWidget(w);
}

void ToolTemplate::requestMenu(QString key)
{
	if(m_leftStack->contains(key)) {
		m_leftStack->show(key);
		return;
	}

	if(m_rightStack->contains(key)) {
		m_rightStack->show(key);
		return;
	}
}




