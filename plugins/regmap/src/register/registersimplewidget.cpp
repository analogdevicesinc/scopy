#include "registersimplewidget.hpp"

#include "bitfield/bitfieldsimplewidget.hpp"
#include "bitfield/bitfieldmodel.hpp"
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

RegisterSimpleWidget::RegisterSimpleWidget(RegisterModel *registerModel, QVector<BitFieldSimpleWidget *> *bitFields,
					   int bitsPerRow, QWidget *parent)
	: registerModel(registerModel)
	, bitFields(bitFields)
{
	installEventFilter(this);

	setMinimumWidth(10);
	int height = 0;
	if(bitsPerRow >= registerModel->getWidth()) {
		height = registerModel->getWidth() / bitsPerRow;
	} else {
		height = registerModel->getWidth() / 8;
	}

	setFixedHeight(60 * height);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(2);
	layout->setSpacing(4);
	setLayout(layout);

	regBaseInfoWidget = new QFrame();

	QHBoxLayout *regBaseInfo = new QHBoxLayout();
	QVBoxLayout *rightLayout = new QVBoxLayout();
	rightLayout->setAlignment(Qt::AlignRight);

	QLabel *registerAddressLable =
		new QLabel(Utils::convertToHexa(registerModel->getAddress(), registerModel->getWidth()));
	registerAddressLable->setAlignment(Qt::AlignRight);
	rightLayout->addWidget(registerAddressLable);
	value = new QLabel("N/R");
	value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	value->setAlignment(Qt::AlignBottom | Qt::AlignRight);
	rightLayout->addWidget(value, Qt::AlignRight);

	QVBoxLayout *leftLayout = new QVBoxLayout();
	leftLayout->setAlignment(Qt::AlignTop);
	registerNameLabel = new QLabel(registerModel->getName());
	registerNameLabel->setWordWrap(true);
	leftLayout->addWidget(registerNameLabel);

	regBaseInfo->addLayout(leftLayout);
	regBaseInfo->addLayout(rightLayout);

	regBaseInfoWidget->setLayout(regBaseInfo);
	// to make sure table proportions are good we use a fixed size for this
	regBaseInfoWidget->setFixedWidth(180);
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
		if(col > (bitsPerRow - 1)) {
			row++;
			col = 0;
		}
		--bits;
	}

	for(int i = 0; i < bitFieldsWidgetLayout->columnCount(); i++) {
		bitFieldsWidgetLayout->setColumnStretch(i, 1);
	}

	layout->addLayout(bitFieldsWidgetLayout, 8);

	QString toolTip = "Name : " + registerModel->getName() + "\n" +
		"Address : " + Utils::convertToHexa(registerModel->getAddress(), registerModel->getWidth()) + "\n" +
		"Description : " + registerModel->getDescription() + "\n" + "Notes : " + registerModel->getNotes() +
		"\n";

	setToolTip(toolTip);

	Preferences *p = Preferences::GetInstance();
	QObject::connect(p, &Preferences::preferenceChanged, this, [=](QString id, QVariant var) {
		if(id.contains("regmap")) {
			applyStyle();
		}
	});
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
	int j = 0;
	for(int i = 0; i < registerModel->getBitFields()->length(); ++i) {

		int width = registerModel->getBitFields()->at(i)->getWidth();
		uint32_t bfMask = Utils::getBitMask(regOffset, width);
		uint32_t bfVal = (bfMask & value) >> regOffset;

		QString bitFieldValue = Utils::convertToHexa(bfVal, bitFields->at(i)->getWidth());

		regOffset += width;

		// some bitfileds will be on multiple rows
		while(bitFields->length() > j &&
		      bitFields->at(j)->getName() == registerModel->getBitFields()->at(i)->getName()) {
			bitFields->at(j)->blockSignals(true);

			bitFields->at(j)->updateValue(bitFieldValue);

			bitFields->at(j)->blockSignals(false);
			j++;
		}
	}
	this->value->setText(Utils::convertToHexa(value, registerModel->getWidth()));
	applyStyle();
}

void RegisterSimpleWidget::setRegisterSelected(bool selected)
{
	scopy::setDynamicProperty(regBaseInfoWidget, "is_selected", selected);
	for(int i = 0; i < bitFields->length(); ++i) {
		bitFields->at(i)->setSelected(selected);
	}
}

void RegisterSimpleWidget::applyStyle() { RegmapStyleHelper::RegisterSimpleWidgetStyle(this); }

RegisterModel *RegisterSimpleWidget::getRegisterModel() const { return registerModel; }

bool RegisterSimpleWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress) {
		Q_EMIT registerSelected(registerModel->getAddress());
	}

	return QWidget::eventFilter(object, event);
}
