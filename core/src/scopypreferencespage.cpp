#include "scopypreferencespage.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabBar>

using namespace adiscope;
ScopyPreferencesPage::ScopyPreferencesPage(QWidget *parent) : QTabWidget(parent)
{
	setTabPosition(TabPosition::East);
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

#include "moc_scopypreferencespage.cpp"
