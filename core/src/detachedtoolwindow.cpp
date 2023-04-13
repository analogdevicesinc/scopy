#include "detachedtoolwindow.h"
#include <QHBoxLayout>
#include <pluginbase/preferences.h>
#include <QApplication>
#include <QIcon>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DETACHEDTOOL,"DetachedToolWindow")

using namespace scopy;
DetachedToolWindow::DetachedToolWindow(QWidget *parent, ToolMenuEntry *tme) : QWidget(parent) {

	this->tme = tme;

	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setSpacing(0);
	lay->setMargin(0);
	w = tme->tool();
	setWindowIcon(QApplication::windowIcon());
	setWindowTitle("Scopy - "+tme->pluginName()+" - " + tme->name());

	lay->addWidget(w);
	loadToolGeometry(tme,this);
	tme->tool()->show();
	show();
}

DetachedToolWindow::~DetachedToolWindow() {
	saveToolGeometry(tme,this);
	w->setParent(nullptr);
}

void DetachedToolWindow::closeEvent(QCloseEvent *event) {
	saveToolGeometry(tme,w);
	tme->setAttached(true);
}

void DetachedToolWindow::saveToolGeometry(ToolMenuEntry *tme, QWidget *w) {
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;
	if(w) {
		prefId = tme->id()+"_geometry";
		QRect geometry = w->geometry();
		p->set(prefId, geometry);
		qDebug(CAT_DETACHEDTOOL)<<"Saving " << prefId << " " << geometry;
	}
}

void DetachedToolWindow::loadToolGeometry(ToolMenuEntry *tme, QWidget *w) {
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;

	if(w && !tme->attached()) {
		prefId = tme->id()+"_geometry";
		QRect geometry = p->get(prefId).toRect();
		if(!geometry.isNull())
			w->setGeometry(geometry);

		qDebug(CAT_DETACHEDTOOL)<<"Loading " <<prefId<<tme->tool()->geometry();
	}
}
