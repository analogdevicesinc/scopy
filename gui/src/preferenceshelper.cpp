/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "preferenceshelper.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QApplication>
#include <smallOnOffSwitch.h>
#include <toolbuttons.h>
#include <popupwidget.h>
#include <style.h>

using namespace scopy;

QWidget *PreferencesHelper::setupDescriptionButton(QString id, QString description, QObject *parent)
{
	InfoBtn *infoBtn = new InfoBtn();
	Style::setStyle(infoBtn, style::properties::button::squareIconButton, false);
	Style::setStyle(infoBtn, style::properties::button::smallSquareIconButton);
	Style::setStyle(infoBtn, style::properties::widget::basicBackground);

	parent->connect(infoBtn, &InfoBtn::clicked, parent, [parent, id, description](bool b) {
		QWidget *parentWidget = QApplication::activeWindow();
		PopupWidget *popup = new PopupWidget(parentWidget);
		popup->enableCenterOnParent(true);
		popup->setTitle("ID: " + id);
		popup->setDescription(description);
		popup->getExitBtn()->hide();
		popup->getContinueBtn()->hide();
		popup->enableCloseButton(true);
		popup->enableTintedOverlay(true);
		popup->show();
		popup->raise();
	});
	return infoBtn;
}

QWidget *PreferencesHelper::addPreferenceCheckBox(Preferences *p, QString id, QString title, QString description,
						  QObject *parent)
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->setMargin(0);
	widget->setLayout(layout);

	QWidget *infoBtn = setupDescriptionButton(id, description, parent);

	SmallOnOffSwitch *pref = new SmallOnOffSwitch();
	pref->setChecked(p->get(id).toBool());
	parent->connect(pref, &SmallOnOffSwitch::toggled, parent, [p, id](bool b) { p->set(id, b); });

	p->initDescription(id, title, description);

	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed);

	layout->addWidget(infoBtn);
	layout->addWidget(new QLabel(title, widget));
	layout->addSpacerItem(space);
	layout->addWidget(pref);
	widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	return widget;
}

QWidget *PreferencesHelper::addPreferenceEdit(Preferences *p, QString id, QString title, QString description,
					      QObject *parent)
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->setMargin(0);
	widget->setLayout(layout);

	QWidget *infoBtn = setupDescriptionButton(id, description, parent);

	QString pref1Val = p->get(id).toString();
	QLineEdit *pref = new QLineEdit();
	pref->setText(pref1Val);
	parent->connect(pref, &QLineEdit::textChanged, parent, [p, id](QString b) { p->set(id, b); });

	QLabel *label = new QLabel(title);
	p->initDescription(id, title, description);

	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Preferred);

	layout->addWidget(infoBtn);
	layout->addWidget(label, 1);
	layout->addSpacerItem(space);
	layout->addWidget(pref, 1);
	return widget;
}

QWidget *PreferencesHelper::addPreferenceEditValidation(Preferences *p, QString id, QString title, QString description,
							std::function<bool(const QString &)> validator, QObject *parent)
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->setMargin(0);
	widget->setLayout(layout);

	QWidget *infoBtn = setupDescriptionButton(id, description, parent);

	QString pref1Val = p->get(id).toString();
	QLineEdit *pref = new QLineEdit();
	pref->setText(pref1Val);
	parent->connect(pref, &QLineEdit::returnPressed, parent, [p, id, validator, pref]() {
		QString prefText = pref->text();
		bool valid = false;
		if(validator) {
			valid = validator(prefText);
		}

		if(valid) {
			p->set(id, prefText);
		} else {
			pref->setText(p->get(id).toString());
		}
	});

	QLabel *label = new QLabel(title);
	p->initDescription(id, title, description);

	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Preferred);

	layout->addWidget(infoBtn);
	layout->addWidget(label, 1);
	layout->addSpacerItem(space);
	layout->addWidget(pref, 1);
	return widget;
}

QWidget *PreferencesHelper::addPreferenceComboList(Preferences *p, QString id, QString title, QString description,
						   QList<QPair<QString, QVariant>> options, QObject *parent)
{
	QWidget *w = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setMargin(0);
	lay->setSpacing(0);
	lay->setMargin(0);
	w->setLayout(lay);

	QWidget *infoBtn = setupDescriptionButton(id, description, parent);

	QLabel *lab = new QLabel(title);
	p->initDescription(id, title, description);
	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Preferred);
	QString pref1Val;

	QComboBox *pref = new QComboBox();
	for(const auto &option : options) {
		pref->addItem(option.first, option.second);
		if(option.second == p->get(id)) {
			pref1Val = option.first;
		}
	}

	pref->setCurrentText(pref1Val);
	lay->addWidget(infoBtn);
	lay->addWidget(lab, 1);
	lay->addSpacerItem(space);
	lay->addWidget(pref, 1);

	parent->connect(pref, qOverload<int>(&QComboBox::currentIndexChanged), parent, [pref, p, id](int idx) {
		auto data = pref->itemData(idx);
		p->set(id, data);
	});
	return w;
}

QWidget *PreferencesHelper::addPreferenceCombo(Preferences *p, QString id, QString title, QString description,
					       QStringList options, QObject *parent)
{
	QWidget *w = new QWidget();
	QHBoxLayout *lay = new QHBoxLayout();
	lay->setSpacing(0);
	lay->setMargin(0);

	QWidget *infoBtn = setupDescriptionButton(id, description, parent);

	w->setLayout(lay);
	QLabel *lab = new QLabel(title);
	p->initDescription(id, title, description);
	QSpacerItem *space = new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Preferred);
	QString pref1Val = p->get(id).toString();
	QComboBox *pref = new QComboBox();
	pref->addItems(options);
	pref->setCurrentText(pref1Val);
	lay->addWidget(infoBtn);
	lay->addWidget(lab, 1);
	lay->addSpacerItem(space);
	lay->addWidget(pref, 1);

	parent->connect(pref, &QComboBox::currentTextChanged, parent, [p, id](QString b) { p->set(id, b); });
	return w;
}
