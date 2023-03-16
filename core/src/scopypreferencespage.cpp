#include "scopypreferencespage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>
#include "pluginbase/preferenceshelper.h"

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

ScopyPreferencesPage::~ScopyPreferencesPage() {

}

QWidget* ScopyPreferencesPage::buildGeneralPreferencesPage()
{
	QWidget *page = new QWidget(this);
	QVBoxLayout *lay = new QVBoxLayout(page);

	Preferences *p = Preferences::GetInstance();
	lay->addWidget(PreferencesHelper::addPreferenceCombo(p,"general_plot_target_fps", "Plot target FPS", {"15","20","30","60"}, this));
	lay->addWidget(PreferencesHelper::addPreferenceCheckBox(p,"general_show_plot_fps","Show plot FPS", this));

	lay->addSpacerItem(new QSpacerItem(40,40,QSizePolicy::Expanding,QSizePolicy::Expanding));
	return page;
}

#include "moc_scopypreferencespage.cpp"
