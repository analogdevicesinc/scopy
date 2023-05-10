#include "markermenupicker.h"
#include "ui_markermenupicker.h"

MarkerMenuPicker::MarkerMenuPicker(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MarkerMenuPicker)
{
	ui->setupUi(this);
}

MarkerMenuPicker::~MarkerMenuPicker()
{
	delete ui;
}
