#include "preferenceshelper.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace scopy;
QCheckBox *PreferencesHelper::addPreferenceCheckBox(Preferences *p, QString id, QString description, QObject *parent)
{
	bool pref1Val = p->get(id).toBool();
	QCheckBox *pref = new QCheckBox(description);
	pref->setChecked(pref1Val);
	parent->connect(pref, &QCheckBox::toggled, parent, [p, id](bool b) { p->set(id, b); });
	return pref;
}

QWidget *PreferencesHelper::addPreferenceEdit(Preferences *p, QString id, QString description, QObject *parent)
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	widget->setLayout(layout);

	QString pref1Val = p->get(id).toString();
	QLineEdit *pref = new QLineEdit();
	pref->setMaximumWidth(200);
	pref->setText(pref1Val);
	parent->connect(pref, &QLineEdit::textChanged, parent, [p, id](QString b) { p->set(id, b); });

	QLabel *label = new QLabel(description);
	label->setStyleSheet("font-weight: bold");

	layout->addWidget(label);
	layout->addWidget(pref);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	return widget;
}

QWidget *PreferencesHelper::addPreferenceComboList(Preferences *p, QString id, QString description,
						   QList<QPair<QString, QVariant>> options, QObject *parent)
{
	QWidget *w = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setMargin(0);
	w->setLayout(lay);
	QLabel *lab = new QLabel(description);
	lab->setStyleSheet("font-weight: bold;");
	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Preferred);
	QString pref1Val;

	QComboBox *pref = new QComboBox();
	for(auto option : options) {
		pref->addItem(option.first, option.second);
		if(option.second == p->get(id)) {
			pref1Val = option.first;
		}
	}

	pref->setCurrentText(pref1Val);
	lay->addWidget(lab, 1);
	lay->addSpacerItem(space);
	lay->addWidget(pref, 1);

	parent->connect(pref, qOverload<int>(&QComboBox::currentIndexChanged), parent, [pref, p, id](int idx) {
		auto data = pref->itemData(idx);
		p->set(id, data);
	});
	return w;
}

QWidget *PreferencesHelper::addPreferenceCombo(Preferences *p, QString id, QString description, QStringList options,
					       QObject *parent)
{
	QWidget *w = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setSpacing(0);
	lay->setMargin(0);
	w->setLayout(lay);
	QLabel *lab = new QLabel(description);
	lab->setStyleSheet("font-weight: bold;");
	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Preferred);
	QString pref1Val = p->get(id).toString();
	QComboBox *pref = new QComboBox();
	pref->addItems(options);
	pref->setCurrentText(pref1Val);
	lay->addWidget(lab, 1);
	lay->addSpacerItem(space);
	lay->addWidget(pref, 1);

	parent->connect(pref, &QComboBox::currentTextChanged, parent, [p, id](QString b) { p->set(id, b); });
	return w;
}
