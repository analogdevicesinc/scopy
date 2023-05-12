#include "scopypreferencespage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include <QDesktopServices>
#include <QUrl>
#include "pluginbase/preferenceshelper.h"
#include "application_restarter.h"
#include <QDir>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PREFERENCESPAGE, "ScopyPreferencesPage");

using namespace scopy;
ScopyPreferencesPage::ScopyPreferencesPage(QWidget *parent) : QTabWidget(parent)
{
	setTabPosition(TabPosition::East);
	addHorizontalTab(buildGeneralPreferencesPage(),"General");
}

void ScopyPreferencesPage::addHorizontalTab(QWidget *w, QString text) {
	// Hackish - so we don't override paint event
	addTab(w, "");
	QLabel *lbl1 = new QLabel();
	lbl1->setText(text);
	QTabBar *tabbar = tabBar();
	tabbar->setTabButton(tabbar->count() - 1, QTabBar::RightSide, lbl1);

}

void ScopyPreferencesPage::showRestartWidget()
{
	restartWidget->show();
}

ScopyPreferencesPage::~ScopyPreferencesPage() {

}

QWidget* ScopyPreferencesPage::createRestartWidget() {
	QWidget *w = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setSpacing(0);
	lay->setMargin(0);
	w->setLayout(lay);
	QLabel *lab = new QLabel("An application restart is required for these settings to take effect. ");
	QSpacerItem *space = new QSpacerItem(20,20,QSizePolicy::Preferred,QSizePolicy::Preferred);
	QPushButton* btn = new QPushButton("Restart");
	btn->setProperty("blue_button",true);
	btn->setStyleSheet("width:80;height:20");

	lay->addWidget(lab,3);
	lay->addSpacerItem(space);
	lay->addWidget(btn,1);

	w->connect(btn,&QPushButton::clicked, btn, [](){ApplicationRestarter::triggerRestart();});
	w->hide();
	return w;
}

QWidget* ScopyPreferencesPage::buildSaveSessionPreference() {
	Preferences *p = Preferences::GetInstance();
	QWidget *w = new QWidget(this);
	QHBoxLayout *lay = new QHBoxLayout(w);
	lay->setMargin(0);

	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_save_session", "Save/Load Scopy session", this));
	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding,QSizePolicy::Fixed));
	lay->addWidget(new QLabel("Settings files location ",this));
	QPushButton *navigateBtn = new QPushButton("Open",this);
	navigateBtn->setProperty("blue_button",true);
	navigateBtn->setStyleSheet("width:80;height:20");
	connect(navigateBtn,&QPushButton::clicked,this,[=]() {QDesktopServices::openUrl(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)); });
	lay->addWidget(navigateBtn);
	return w;
}

QWidget* ScopyPreferencesPage::buildResetScopyDefaultButton() {
	Preferences *p = Preferences::GetInstance();
	QWidget *w = new QWidget(this);
	QHBoxLayout *lay = new QHBoxLayout(w);

	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(new QLabel("Reset to settings and plugins to default"));
	QPushButton	*resetBtn = new QPushButton("Reset",this);
	resetBtn->setProperty("blue_button",true);
	resetBtn->setStyleSheet("width:80;height:20");
	connect(resetBtn,&QPushButton::clicked,this,[=]() {
		QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
		QDir loc(dir);
		QFileInfoList plugins = loc.entryInfoList(QDir::Files);
		QStringList settingsFiles;

		for(const QFileInfo &p : plugins) {
			if(p.suffix() == "ini")
				settingsFiles.append(p.absoluteFilePath());
		}
		qInfo(CAT_PREFERENCESPAGE)<<settingsFiles;
		for(auto &&file : settingsFiles) {
			QFile(file).rename(file+".bak");
		}
		p->clear();
		showRestartWidget();
	}	);
	lay->addWidget(resetBtn);
	return w;
}

QWidget* ScopyPreferencesPage::buildGeneralPreferencesPage()
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);
	lay->setSpacing(10);
	QHBoxLayout *lay3 = new QHBoxLayout(page);

	Preferences *p = Preferences::GetInstance();

	lay->addWidget(buildSaveSessionPreference());

	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_save_attached", "Save/Load tool attached state", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_doubleclick_attach", "Doubleclick to attach/detach tool", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_use_opengl", "Enable OpenGL plotting", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_use_animations", "Enable menu animations", this));
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_theme", "Theme", {"dark","light"}, this));
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_language", "Language",{"english","romanian"}, this));

	lay->addWidget(new QLabel("--- Debug preferences --- "));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_show_plot_fps","Show plot FPS", this));
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_plot_target_fps", "Plot target FPS", {"15","20","30","60"}, this));

	lay->addWidget(buildResetScopyDefaultButton());


	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding,QSizePolicy::Expanding));
	restartWidget = createRestartWidget();
	lay->addWidget(restartWidget);
	return page;
}

#include "moc_scopypreferencespage.cpp"
