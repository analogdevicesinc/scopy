#include "bitfieldsimplewidget.hpp"

#include <qboxlayout.h>
#include <qlabel.h>


BitFieldSimpleWidget::BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width, QString notes, int regOffset, int streach, QWidget *parent):
	width(width),
	description(description),
	streach(streach),
	QWidget{parent}
{

	setMinimumWidth(10);
	setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	//	setStyleSheet("border: 1px solid black");

	layout = new QVBoxLayout();

	layout->addWidget(new QLabel(name));
	layout->addWidget(new QLabel(QString::number(regOffset + width - 1) + ":" + QString::number(regOffset)));
	layout->addWidget(new QLabel(description));
	layout->addWidget(new QLabel("Default Value: " + QString::number(defaultValue)));

	QString toolTip = "Name : " + name + "\n"
			+ QString::number(regOffset + width - 1) + ":" + QString::number(regOffset) + "\n"
			+ "Description : " + description + "\n"
			+ "Notes : " + notes + "\n"
			+ "Default Value : " + QString::number(defaultValue);

	setToolTip(toolTip);

	value = new QLabel("Not Read");
	layout->addWidget(value);

	setLayout(layout);
}

BitFieldSimpleWidget::~BitFieldSimpleWidget()
{
	delete value;
	delete layout;
}

void BitFieldSimpleWidget::updateValue(QString newValue)
{
	value->setText(newValue);
}

int BitFieldSimpleWidget::getWidth() const
{
	return width;
}

QString BitFieldSimpleWidget::getDescription() const
{
	return description;
}

int BitFieldSimpleWidget::getStreach() const
{
	return streach;
}
