#include "scopypreferencespage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include "pluginbase/preferenceshelper.h"
#include "application_restarter.h"

using namespace adiscope;
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
	lay->addWidget(lab,1);
	lay->addSpacerItem(space);
	lay->addWidget(btn,1);

	w->connect(btn,&QPushButton::clicked, btn, [](){ApplicationRestarter::triggerRestart();});
	w->hide();
	return w;
}

QWidget* ScopyPreferencesPage::buildGeneralPreferencesPage()
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);

	Preferences *p = Preferences::GetInstance();
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_save_session", "Save/Load Scopy session", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_save_attached", "Save/Load tool attached state", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_doubleclick_attach", "Doubleclick to attach/detach tool", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_use_opengl", "Enable OpenGL plotting", this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_use_animations", "Enable menu animations", this));
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_theme", "Theme", {"dark","light"}, this));
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_language", "Language",{"english","romanian"}, this));

	lay->addWidget(new QLabel("--- Debug preferences --- "));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_show_plot_fps","Show plot FPS", this));
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_plot_target_fps", "Plot target FPS", {"15","20","30","60"}, this));

	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding,QSizePolicy::Expanding));
	restartWidget = createRestartWidget();
	lay->addWidget(restartWidget);
	return page;
}

#include "moc_scopypreferencespage.cpp"
