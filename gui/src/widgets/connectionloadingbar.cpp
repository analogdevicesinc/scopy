#include "connectionloadingbar.h"
#include <QHBoxLayout>
#include <stylehelper.h>

using namespace scopy;

ConnectionLoadingBar::ConnectionLoadingBar(QWidget *parent)
	: QWidget{parent}
	, m_currentPluginName("")
	, m_pluginCount(0)
	, m_noPlugins(0)
{
	setupUi();
}

void ConnectionLoadingBar::setCurrentPlugin(QString pluginName)
{
	m_currentPluginName = pluginName;
	m_loadingLabel->setText(QString("Loading %1 plugin").arg(m_currentPluginName));
}

void ConnectionLoadingBar::addProgress(int progress)
{
	m_progressBar->setValue(m_progressBar->value() + progress);
	++m_pluginCount;
	m_pluginCountLabel->setText(QString("%1/%2").arg(m_pluginCount).arg(m_noPlugins));
}

void ConnectionLoadingBar::setProgressBarMaximum(int maximum)
{
	m_noPlugins = maximum;
	m_progressBar->setMaximum(m_noPlugins);
}

void ConnectionLoadingBar::setupUi()
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);

	m_loadingLabel = new QLabel("", this);
	m_pluginCountLabel = new QLabel("", this);
	m_progressBar = new QProgressBar(this);
	m_progressBar->setMinimum(0);
	m_progressBar->setTextVisible(false);
	m_progressBar->setMaximumHeight(10);
	m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_progressBar->setValue(0);
	QString style = QString("QProgressBar::chunk {background-color: %1;}").arg(StyleHelper::getColor("ScopyBlue"));
	setStyleSheet(style);

	layout()->addWidget(m_loadingLabel);
	layout()->addWidget(m_pluginCountLabel);
	layout()->addWidget(m_progressBar);
}

#include "moc_connectionloadingbar.cpp"
