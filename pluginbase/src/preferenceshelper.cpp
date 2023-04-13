#include "preferenceshelper.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace scopy;
QCheckBox* PreferencesHelper::addPreferenceCheckBox(Preferences *p, QString id, QString description, QObject *parent) {
	bool pref1Val = p->get(id).toBool();
	QCheckBox *pref = new QCheckBox(description);
	pref->setChecked(pref1Val);
	parent->connect(pref,&QCheckBox::toggled,parent,[p,id](bool b) { p->set(id,b);});
	return pref;
}

QLineEdit* PreferencesHelper::addPreferenceEdit(Preferences *p, QString id, QString description, QObject *parent) {
	QString pref1Val = p->get(id).toString();
	QLineEdit *pref = new QLineEdit(description);
	pref->setText(pref1Val);
	parent->connect(pref,&QLineEdit::textChanged,parent,[p,id](QString b) { p->set(id,b);});
	return pref;
}

QWidget* PreferencesHelper::addPreferenceCombo(Preferences *p, QString id, QString description, QStringList options, QObject *parent) {
	QWidget *w = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setSpacing(0);
	lay->setMargin(0);
	w->setLayout(lay);
	QLabel *lab = new QLabel(description);
	QSpacerItem *space = new QSpacerItem(20,20,QSizePolicy::Preferred,QSizePolicy::Preferred);
	QString pref1Val = p->get(id).toString();
	QComboBox *pref = new QComboBox();
	pref->addItems(options);
	pref->setCurrentText(pref1Val);
	lay->addWidget(lab,1);
	lay->addSpacerItem(space);
	lay->addWidget(pref,1);

	parent->connect(pref,&QComboBox::currentTextChanged,parent,[p,id](QString b) { p->set(id,b);});
	return w;
}
