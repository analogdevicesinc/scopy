#include "scopystatusbar.h"
#include "stylehelper.h"
#include <pluginbase/statusbarmanager.h>
#include <QLoggingCategory>
#include <QApplication>
#include <QTimer>
#include <QSizeGrip>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SCOPYSTATUSBAR, "ScopyStatusBar")

ScopyStatusBar::ScopyStatusBar(QWidget *parent)
	: MenuVAnim(parent)
{
	initUi();

	auto statusManager = StatusBarManager::GetInstance();
	connect(statusManager, &StatusBarManager::sendStatus, this, &ScopyStatusBar::displayStatusMessage);
	connect(statusManager, &StatusBarManager::clearDisplay, this, &ScopyStatusBar::clearStatusMessage);
}

void ScopyStatusBar::initUi()
{
	// general layout
	auto mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setAnimMin(0);
	setAnimMax(20);
	setMinimumHeight(20);
	setMaximumHeight(20);

	m_leftWidget = new QWidget(this);
	auto leftLayout = new QHBoxLayout(m_leftWidget);
	leftLayout->setContentsMargins(0, 0, 0, 0);
	m_leftWidget->setLayout(leftLayout);

	m_rightWidget = new QWidget(this);
	auto rightLayout = new QHBoxLayout(m_rightWidget);
	rightLayout->setContentsMargins(0, 3, 0, 3);
	m_rightWidget->setLayout(rightLayout);

	layout()->addWidget(m_leftWidget);
	layout()->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layout()->addWidget(m_rightWidget);

	// right layout
	StyleHelper::ScopyStatusBar(this, "ScopyStatusBar");

	hide();
}

void ScopyStatusBar::addToRight(QWidget *widget) { m_rightWidget->layout()->addWidget(widget); }

void ScopyStatusBar::addToLeft(QWidget *widget) { m_leftWidget->layout()->addWidget(widget); }

void ScopyStatusBar::displayStatusMessage(StatusMessage *statusMessage)
{
	m_message = statusMessage;
	addToLeft(statusMessage->getWidget());
	this->toggleMenu(true);
	this->show();
}

void ScopyStatusBar::clearStatusMessage()
{
	this->toggleMenu(false);
	delete m_message;
	m_message = nullptr;
}

#include "moc_scopystatusbar.cpp"
