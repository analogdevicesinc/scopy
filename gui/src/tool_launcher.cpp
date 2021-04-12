#include "dynamic_widget.hpp"
#include "tool.hpp"

#include "ui_tool_launcher.h"

#include <scopy/gui/tool_launcher.hpp>

using namespace scopy::gui;

ToolLauncher::ToolLauncher(QWidget* parent)
	: QMainWindow(parent)
	, m_ui(new Ui::ToolLauncher)
	, m_toolMenu(nullptr)
	, m_about(nullptr)
	, m_current(nullptr)
{
	m_ui->setupUi(this);

	m_prefPanel = new Preferences(this);
	m_prefPanel->setVisible(false);

	m_notesPanel = new UserNotes(this);
	m_notesPanel->setVisible(false);

	connect(m_ui->prefBtn, &QPushButton::clicked, [=]() { swapMenu(static_cast<QWidget*>(m_prefPanel)); });
	connect(m_ui->btnNotes, &QPushButton::clicked, [=]() { swapMenu(static_cast<QWidget*>(m_notesPanel)); });
	connect(m_ui->btnHome, &QPushButton::clicked, [=]() { swapMenu(static_cast<QWidget*>(m_ui->widgetHome)); });

	m_about = new ScopyAboutDialog(this);
	connect(m_ui->btnAbout, &QPushButton::clicked, [=]() {
		if (!m_about)
			m_about = new ScopyAboutDialog(this);
		m_about->setModal(false);
		m_about->show();
		m_about->raise();
		m_about->activateWindow();
	});

	connect(m_ui->menuAnimMenu, &MenuAnim::finished, [=](bool opened) {
		m_ui->saveLbl->setVisible(opened);
		m_ui->loadLbl->setVisible(opened);
		m_ui->prefBtn->setText(opened ? tr("Preferences") : "");
		//		m_toolMenu->hideMenuText(!opened);
	});

	connect(m_ui->btnHome, &QPushButton::clicked, [=]() { swapMenu(static_cast<QWidget*>(m_ui->widgetHome)); });

	m_ui->saveBtn->parentWidget()->setEnabled(false);
	m_ui->loadBtn->parentWidget()->setEnabled(true);

	m_ui->btnHome->toggle();

	// TO DO: Remove temporary spaces
	// set home icon
	m_ui->btnHome->setText("   Home");
	m_ui->btnHome->setIcon(QIcon::fromTheme("house"));
	m_ui->btnHome->setIconSize(QSize(32, 32));

	m_current = m_ui->widgetHome;

	// Visualize all connected uris
	m_boardDetector = new scopy::core::ContextEnumerator();
	connect(m_boardDetector, &scopy::core::ContextEnumerator::printData, this,
		&scopy::gui::ToolLauncher::printUris);
}

void ToolLauncher::swapMenu(QWidget* menu)
{
	Tool* tl = dynamic_cast<Tool*>(menu);

	if (m_current) {
		m_current->setVisible(false);
		m_ui->hLayoutCentral->removeWidget(m_current);
	}

	m_current = menu;

	m_ui->hLayoutCentral->addWidget(m_current);
	m_current->setVisible(true);
}

void ToolLauncher::setTestLbl(const QString& text) { m_ui->lblTestDevFound->setText(text); }

void ToolLauncher::printUris(const QStringList& uris)
{
	QString text = std::accumulate(uris.cbegin(), uris.cend(), QString{});
	this->setTestLbl(text);
	m_boardDetector->start();
}

void ToolLauncher::onBtnHomeClicked() { swapMenu(m_ui->widgetHome); }
