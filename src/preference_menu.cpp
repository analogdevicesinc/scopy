#include "preference_menu.h"
#include "ui_preference_menu.h"
#include <QDebug>

using namespace adiscope;

PreferenceMenu::PreferenceMenu(QWidget *parent) :
	QWidget(parent),
	p_ui(new Ui::PreferenceMenu)
{
	p_ui->setupUi(this);
	if (!parent)
		qDebug() << "This widget can't process the preferences of a null parent!";
}

PreferenceMenu::~PreferenceMenu()
{
	delete p_ui;
}

void PreferenceMenu::insertOption(PreferenceOption *option, const char *slot)
{
	if (parentWidget()) {
		p_ui->option_container->insertWidget(0, option);
		connect(option, SIGNAL(enabled(bool)), parentWidget(), slot);

	}
	else
		qDebug() << "connection to : " << slot << " could not be established!";
}

void PreferenceMenu::load()
{
	for (const auto &option : findChildren<PreferenceOption*>()) {
		option->load();
	}

}
