#include <QWidget>
#include "swiotconfigmenu.hpp"
#include <QMap>
#include <QObject>

using namespace adiscope;

SwiotConfigMenu::SwiotConfigMenu(QWidget* parent) :
	QWidget(parent)
      ,m_ui(new Ui::SwiotConfigMenu)
{
	m_ui->setupUi(this);
}

SwiotConfigMenu::~SwiotConfigMenu()
{
	delete m_ui;
}

void SwiotConfigMenu::addWidget(QWidget* widget)
{
	m_ui->verticalLayout->addWidget(widget);
}
