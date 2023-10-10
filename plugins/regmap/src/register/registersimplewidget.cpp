#include "registersimplewidget.hpp"

#include "bitfield/bitfieldsimplewidget.hpp"
#include "dynamicWidget.h"

#include <QLabel>
#include <qboxlayout.h>
#include <qcoreevent.h>
#include <qdebug.h>

#include <pluginbase/preferences.h>
#include <regmapstylehelper.hpp>
#include <src/utils.hpp>
#include <utils.h>

using namespace scopy;
using namespace regmap;

RegisterSimpleWidget::RegisterSimpleWidget(QString name, QString address, QString description, QString notes,
					   int registerWidth, QVector<BitFieldSimpleWidget *> *bitFields,
					   QWidget *parent)
	: bitFields(bitFields)
	, registerWidth(registerWidth)
	, address(address)
{
	installEventFilter(this);

	setMinimumWidth(10);
	setFixedHeight(60 * (registerWidth / 8));
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	bool ok;
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(2);
	layout->setSpacing(4);
	setLayout(layout);

	regBaseInfoWidget = new QFrame();

	QHBoxLayout *regBaseInfo = new QHBoxLayout();
	QVBoxLayout *rightLayout = new QVBoxLayout();
	rightLayout->setAlignment(Qt::AlignRight);

	QLabel *registerAddressLable =
		new QLabel(scopy::regmap::Utils::convertToHexa(address.toInt(&ok, 16), registerWidth));
	registerAddressLable->setAlignment(Qt::AlignRight);
	rightLayout->addWidget(registerAddressLable);
	value = new QLabel("N/R");
	rightLayout->addWidget(value);

	QVBoxLayout *leftLayout = new QVBoxLayout();
	leftLayout->setAlignment(Qt::AlignTop);
	registerNameLabel = new QLabel(name);
	registerNameLabel->setWordWrap(true);
	registerNameLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	leftLayout->addWidget(registerNameLabel);

	regBaseInfo->addLayout(leftLayout);
	regBaseInfo->addLayout(rightLayout);

	regBaseInfoWidget->setLayout(regBaseInfo);
	// to make sure table proportions are good we use a fixed size for this
	regBaseInfoWidget->setFixedWidth(130);
	layout->addWidget(regBaseInfoWidget, 1);

	// add bitfield widgets
	QGridLayout *bitFieldsWidgetLayout = new QGridLayout();
	bitFieldsWidgetLayout->setSpacing(0);

	int bits = bitFields->length() - 1;
	int row = 0;
	int col = 0;
	while(bits >= 0) {
		int streach = bitFields->at(bits)->getStreach();
		bitFieldsWidgetLayout->addWidget(bitFields->at(bits), row, col, 1, streach);
		col += streach;
		if(col > scopy::regmap::Utils::getBitsPerRow()) {
			row++;
			col = 0;
		}
		--bits;
	}

	for(int i = 0; i < bitFieldsWidgetLayout->columnCount(); i++) {
		bitFieldsWidgetLayout->setColumnStretch(i, 1);
	}

	layout->addLayout(bitFieldsWidgetLayout, 8);

	QString toolTip = "Name : " + name + "\n" +
		"Address : " + scopy::regmap::Utils::convertToHexa(address.toInt(&ok, 16), registerWidth) + "\n" +
		"Description : " + description + "\n" + "Notes : " + notes + "\n";

	setToolTip(toolTip);
}

RegisterSimpleWidget::~RegisterSimpleWidget()
{
	delete registerNameLabel;
	delete value;
	delete regBaseInfoWidget;
}

void RegisterSimpleWidget::valueUpdated(uint32_t value)
{
	int regOffset = 0;
	for(int i = 0; i < bitFields->length(); ++i) {
		bitFields->at(i)->blockSignals(true);

		int width = bitFields->at(i)->getWidth();
		int bfVal = (((1 << (regOffset + width)) - 1) & value) >> regOffset;
		QString bitFieldValue = scopy::regmap::Utils::convertToHexa(bfVal, bitFields->at(i)->getWidth());
		bitFields->at(i)->updateValue(bitFieldValue);
		regOffset += width;

		bitFields->at(i)->blockSignals(false);
	}
	this->value->setText(scopy::regmap::Utils::convertToHexa(value, registerWidth));
	checkPreferences();
}

void RegisterSimpleWidget::setRegisterSelected(bool selected)
{
	scopy::setDynamicProperty(regBaseInfoWidget, "is_selected", selected);
	for(int i = 0; i < bitFields->length(); ++i) {
		bitFields->at(i)->setSelected(selected);
	}
}

void RegisterSimpleWidget::checkPreferences()
{
	scopy::Preferences *p = scopy::Preferences::GetInstance();
	QString background = p->get("regmap_color_by_value").toString();

	if(background.contains("Register background")) {
		bool ok;
		regBaseInfoWidget->setStyleSheet(
			QString("background-color: " + Util::getColors().at(value->text().toInt(&ok, 16) % 16)));
	}

	if(background.contains("Register text")) {
		bool ok;
		value->setStyleSheet(QString("color: " + Util::getColors().at(value->text().toInt(&ok, 16) % 16)));
	}
}

bool RegisterSimpleWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress) {
		bool ok;
		Q_EMIT registerSelected(address.toInt(&ok, 16));
	}

	return QWidget::eventFilter(object, event);
}
