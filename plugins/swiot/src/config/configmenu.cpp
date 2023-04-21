#include <QWidget>
#include "configmenu.h"
#include <QMap>
#include <QObject>

using namespace scopy::swiot;

ConfigMenu::ConfigMenu(QWidget* parent) :
	QWidget(parent)
      ,m_ui(new Ui::ConfigMenu)
{
	m_ui->setupUi(this);
}

ConfigMenu::~ConfigMenu()
{
	delete m_ui;
}

void ConfigMenu::addWidget(QWidget* widget)
{
	m_ui->verticalLayout->addWidget(widget);
}
