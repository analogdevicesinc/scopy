#include "bitfieldsimplewidget.hpp"

#include "dynamicWidget.h"

#include <QBoxLayout>
#include <QStyle>
#include <QVariant>
#include <qlabel.h>

#include <pluginbase/preferences.h>
#include <regmapstylehelper.hpp>
#include <src/utils.hpp>
#include <utils.h>

using namespace scopy;
using namespace regmap;

BitFieldSimpleWidget::BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width,
					   QString notes, int regOffset, int streach, QWidget *parent)
	: width(width)
	, description(description)
	, streach(streach)
	, QFrame{parent}
{

	mainFrame = new QFrame;
	RegmapStyleHelper::grayBackgroundHoverWidget(mainFrame, "");

	setMinimumWidth(60);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	layout = new QHBoxLayout();

	QVBoxLayout *rightLayout = new QVBoxLayout();
	rightLayout->setAlignment(Qt::AlignRight);

	value = new QLabel("N/R");
	value->setAlignment(Qt::AlignRight);
	value->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	value->setMinimumWidth(25);
	QLabel *bitfieldWidth = new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset));
	bitfieldWidth->setAlignment(Qt::AlignRight);
	rightLayout->addWidget(bitfieldWidth);
	rightLayout->addWidget(value);

	QVBoxLayout *leftLayout = new QVBoxLayout();
	leftLayout->setAlignment(Qt::AlignTop);
	QLabel *descriptionLabel = new QLabel(name);
	descriptionLabel->setWordWrap(true);
	descriptionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	leftLayout->addWidget(descriptionLabel);

	layout->addLayout(leftLayout);
	layout->addLayout(rightLayout);

	QString toolTip = "Name : " + name + "\n" + QString::number(regOffset + width - 1) + ":" +
		QString::number(regOffset) + "\n" + "Description : " + description + "\n" + "Notes : " + notes + "\n" +
		"Default Value : " + scopy::regmap::Utils::convertToHexa(defaultValue, width);

	setToolTip(toolTip);

	mainFrame->setLayout(layout);
	QVBoxLayout *mainFrameLayout = new QVBoxLayout();
	mainFrameLayout->setMargin(0);
	mainFrameLayout->setSpacing(0);
	mainFrameLayout->addWidget(mainFrame);
	setLayout(mainFrameLayout);
}

BitFieldSimpleWidget::~BitFieldSimpleWidget()
{
	delete value;
	delete layout;
}

void BitFieldSimpleWidget::updateValue(QString newValue)
{
	value->setText(newValue);
	checkPreferences();
}

int BitFieldSimpleWidget::getWidth() const { return width; }

QString BitFieldSimpleWidget::getDescription() const { return description; }

int BitFieldSimpleWidget::getStreach() const { return streach; }

void BitFieldSimpleWidget::checkPreferences()
{
	scopy::Preferences *p = scopy::Preferences::GetInstance();
	QString background = p->get("regmap_color_by_value").toString();

	if(background.contains("Bitfield background")) {
		bool ok;
		mainFrame->setStyleSheet(
			QString("background-color: " + Util::getColors().at(value->text().toInt(&ok, 16) % 16)));
	}

	if(background.contains("Bitfield text")) {
		bool ok;
		value->setStyleSheet(QString("color: " + Util::getColors().at(value->text().toInt(&ok, 16) % 16)));
	}
}

void BitFieldSimpleWidget::setSelected(bool selected) { scopy::setDynamicProperty(mainFrame, "is_selected", selected); }
