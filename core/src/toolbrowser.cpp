#include "toolbrowser.h"

#include "ui_toolbrowser.h"

#include <QDebug>
#include <style.h>

#include <gui/utils.h>

using namespace scopy;

ToolBrowser::ToolBrowser(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ToolBrowser)
	, m_collapsed(false)
{
	ui->setupUi(this);

	ToolMenu *tm = ui->wToolMenu;

	tm->getButtonGroup()->addButton(ui->btnPreferences);
	tm->getButtonGroup()->addButton(ui->btnAbout);

	ToolMenuItem *homeTmi = tm->createTool("home", "Home", ":/gui/icons/home.svg");
	homeTmi->setSeparator(true, true);
	homeTmi->getToolRunBtn()->setVisible(false);
	homeTmi->setEnabled(true);
	ui->homePlaceholder->layout()->addWidget(homeTmi);
	Util::retainWidgetSizeWhenHidden(ui->logo);
	homeTmi->setDraggable(false);

	ui->btnLoad->setIcon(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
					      "/icons/load.svg"));
	ui->btnSave->setIcon(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
					      "/icons/save.svg"));
	ui->btnAbout->setIcon(Style::getPixmap(":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
					       "/icons/info.svg"));
	ui->btnPreferences->setIcon(Style::getPixmap(
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/gear_wheel.svg"));
	ui->btnCollapseMini->setIcon(Style::getPixmap(
		":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) + "/icons/menu.svg"));

	connect(ui->btnCollapse, &QPushButton::clicked, this, &ToolBrowser::toggleCollapse);
	connect(ui->btnCollapseMini, &QPushButton::clicked, this, &ToolBrowser::toggleCollapse);
	connect(ui->btnPreferences, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("preferences"); });
	connect(ui->btnAbout, &QPushButton::clicked, this, [=]() { Q_EMIT requestTool("about"); });

	connect(ui->btnSave, &QPushButton::clicked, this, [=]() { Q_EMIT requestSave(); });
	connect(ui->btnLoad, &QPushButton::clicked, this, [=]() { Q_EMIT requestLoad(); });

	connect(tm, SIGNAL(requestToolSelect(QString)), this, SIGNAL(requestTool(QString)));
	setStyleSheet("QWidget { background-color: " + Style::getAttribute(json::theme::background_primary) + "}");
}

ToolMenu *ToolBrowser::getToolMenu() { return ui->wToolMenu; }

void ToolBrowser::hideMenuText(bool collapsed)
{
	ToolMenu *tm = ui->wToolMenu;

	if(collapsed) {
		setMinimumWidth(50);
		ui->btnLoad->setText("");
		ui->btnSave->setText("");
		ui->btnAbout->setText("");
		ui->btnPreferences->setText("");

	} else {
		setMinimumWidth(200);
		ui->btnLoad->setText(tr("Load"));
		ui->btnSave->setText(tr("Save"));
		ui->btnAbout->setText(tr("About"));
		ui->btnPreferences->setText(tr("Preferences"));
	}
	ui->btnCollapse->setVisible(!collapsed);
	ui->logo->setVisible(!collapsed);
	tm->hideMenuText(collapsed);
}

void ToolBrowser::toggleCollapse()
{
	m_collapsed = !m_collapsed;
	hideMenuText(m_collapsed);
	Q_EMIT collapsed(m_collapsed);
}

ToolBrowser::~ToolBrowser() { delete ui; }

// TEST
/*
 ts->detachTool("home");
 tm->addTool("home1","Home12","");
 tm->addTool("home2","Home2","");
 tm->addTool("home3","Home3","");
 tm->getToolMenuItemFor("home1")->setToolEnabled(true);
 tm->getToolMenuItemFor("home2")->setToolEnabled(true);
 tm->getToolMenuItemFor("home3")->setToolEnabled(true);

 ts->addTool("home1", new QLabel("home1"));
 ts->addTool("home2", new QLabel("home2"));
 ts->addTool("home3", new QLabel("home3"));
*/

#include "moc_toolbrowser.cpp"
