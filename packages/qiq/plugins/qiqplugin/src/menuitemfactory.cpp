#include "menuitemfactory.h"
#include <QComboBox>
#include <QCheckBox>
#include <QVariant>
#include <menucombo.h>
#include <menuonoffswitch.h>
#include "gui/widgets/menuspinbox.h"
#include "gui/style.h"

using namespace scopy::qiqplugin;
using namespace scopy::gui;

MenuItemFactory::MenuItemFactory(QObject *parent)
	: QObject(parent)
{
	connect(this, &MenuItemFactory::itemParamChanged, this, &MenuItemFactory::onItemParamChanged);
}

MenuItemFactory::~MenuItemFactory() {}

QWidget *MenuItemFactory::createWidget(const QString &field, const QVariantMap &params)
{
	if(params.contains("min") && params.contains("max") && params.contains("step")) {
		return createSpinBox(field, params);
	} else if(params.contains("available_values")) {
		return createCombo(field, params);
	} else if(params["type"].toString() == "bool") {
		return createCheckBox(field, params);
	} else {
		return createLineEdit(field, params);
	}

	return nullptr;
}

QWidget *MenuItemFactory::createSpinBox(const QString &field, const QVariantMap &params)
{
	double defaultVal = params.value("default", 0).toDouble();
	MenuSpinbox *spinBox = new MenuSpinbox(field, defaultVal, "", params.value("min", 0).toInt(),
					       params.value("max", 100).toInt());
	spinBox->setIncrementMode(gui::MenuSpinbox::IS_FIXED); //?

	connect(spinBox, &MenuSpinbox::valueChanged, this,
		[this, field](double value) { Q_EMIT itemParamChanged(field, value); });

	return spinBox;
}

QWidget *MenuItemFactory::createLineEdit(const QString &field, const QVariantMap &params)
{
	QWidget *w = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(w);
	lay->setMargin(0);

	QLabel *label = new QLabel(field, w);
	Style::setStyle(label, style::properties::label::menuSmall);

	QLineEdit *lineEdit = new QLineEdit(w);
	lineEdit->setText(params.value("default", "0").toString());

	lay->addWidget(label);
	lay->addWidget(lineEdit);

	connect(lineEdit, &QLineEdit::textChanged, this,
		[this, field](const QString &text) { Q_EMIT itemParamChanged(field, text); });

	return w;
}

QWidget *MenuItemFactory::createCombo(const QString &field, const QVariantMap &params)
{
	MenuCombo *combo = new MenuCombo(field);
	QStringList items = params.value("available_values").toStringList();
	combo->combo()->addItems(items);
	if(params.contains("default")) {
		combo->combo()->setCurrentText(params.value("default").toString());
	}
	connect(combo->combo(), &QComboBox::currentTextChanged, this,
		[this, field](const QString &text) { Q_EMIT itemParamChanged(field, text); });

	return combo;
}

QWidget *MenuItemFactory::createCheckBox(const QString &field, const QVariantMap &params)
{
	MenuOnOffSwitch *checkBox = new MenuOnOffSwitch(field);
	checkBox->onOffswitch()->setChecked(params.value("value", false).toBool());
	connect(checkBox->onOffswitch(), &QCheckBox::toggled, this,
		[this, field](bool checked) { Q_EMIT itemParamChanged(field, checked); });

	return checkBox;
}

QVariantMap MenuItemFactory::toVariantMap() { return m_currentParams; }

void MenuItemFactory::onItemParamChanged(const QString &field, const QVariant &value)
{
	m_currentParams[field] = value;
}
