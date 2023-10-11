#include "scopystatusbar.h"
#include "stylehelper.h"
#include <pluginbase/statusmanager.h>
#include <QLoggingCategory>
#include <QApplication>
#include <QTimer>
#include <QSizeGrip>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SCOPYSTATUSBAR, "ScopyStatusBar")

ScopyStatusBar::ScopyStatusBar(QWidget *parent)
	: QStatusBar(parent)
{
	initUi();
	initHistory();

	auto statusManager = StatusManager::GetInstance();
	connect(statusManager, &StatusManager::sendStatus, this, &ScopyStatusBar::processStatus);

	connect(statusManager, &StatusManager::announceStatusAvailable, this, &ScopyStatusBar::shouldDisplayNewStatus);
	connect(statusManager, &StatusManager::sendUrgentMessage, this, &ScopyStatusBar::receiveUrgentMessage);
	connect(this, &ScopyStatusBar::messageChanged, this, &ScopyStatusBar::shouldDisplayNewStatus);

	statusManager->requestStatus(); // initially, the status bar should tell StatusManager that messages are enabled
}

void ScopyStatusBar::initUi()
{
	StyleHelper::ScopyStatusBar(this, "ScopyStatusBar");

	auto *sizeGrip = this->findChild<QSizeGrip *>();
	if(sizeGrip) {
		// hackish, but hide() and setVisible(false) do not work here
		sizeGrip->setStyleSheet("width: 0; height: 0;");
	}

	m_historyButton = new QPushButton(this);
	m_historyButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_historyButton->setFixedSize(20, 20);
	m_historyButton->setStyleSheet("background-color: #272730");
	m_historyButton->setIcon(QIcon(":/gui/icons/scopy-default/icons/tool_calibration.svg"));
	m_historyButton->setIconSize(m_historyButton->size());
	m_historyButton->setCheckable(true);
	connect(m_historyButton, &QPushButton::toggled, this, &ScopyStatusBar::showHistory);

	this->addPermanentWidget(m_historyButton);
}

void ScopyStatusBar::initHistory()
{
	m_historyList = new QListWidget(this);
	StyleHelper::ScopyHistoryList(m_historyList, "HistoryList");

	m_hoverWidget = new HoverWidget(m_historyList, m_historyButton, parentWidget());
	m_hoverWidget->setObjectName("statusHover");
	m_hoverWidget->setAnchorPos(HP_TOP);
	m_hoverWidget->setContentPos(HP_TOPLEFT);
	m_hoverWidget->setAnchorOffset(QPoint(0, -20));
}

void ScopyStatusBar::shouldDisplayNewStatus()
{
	if(this->currentMessage() == "") {
		auto statusManager = StatusManager::GetInstance();
		statusManager->requestStatus();
	}
}

void ScopyStatusBar::processStatus(QVariant status, int ms)
{
	if(status.canConvert<QString>()) {
		QString statusString = status.toString();
		this->showMessage(statusString, ms);

		m_historyList->addItem(statusString);
		if(m_historyList->count() > 10) {
			m_historyList->takeItem(0); // see if item needs to be deleted
		}
	} else if(status.canConvert<QWidget *>()) {
		auto *statusWidget = qvariant_cast<QWidget *>(status);
		auto *time = new QTimer(this);
		time->start(ms);
		connect(time, &QTimer::timeout, this, [this, time, statusWidget]() {
			this->removeWidget(statusWidget);
			delete time;
		});
		this->addWidget(statusWidget);

		//		auto* listItem = new QListWidgetItem();
		//		listItem->setData(Qt::UserRole, QVariant::fromValue(statusWidget));
		//		if (m_historyList->count() > 10) {
		//			m_historyList->takeItem(0); // see if item needs to be deleted
		//		}
		// TODO: find workaround to display items in history as well
	}
}

void ScopyStatusBar::showHistory(bool checked)
{
	if(checked) {
		m_hoverWidget->raise();
	}
	m_hoverWidget->setVisible(checked);
}

void ScopyStatusBar::receiveUrgentMessage(QString message, int ms)
{
	// the current message will be erased in favor of the urgent message
	this->showMessage(message, ms);
}

#include "moc_scopystatusbar.cpp"
