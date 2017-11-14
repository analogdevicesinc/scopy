#ifndef PREFERENCE_MENU_H
#define PREFERENCE_MENU_H

#include <QWidget>
#include "preference_option.h"

namespace Ui {
class PreferenceMenu;
}

namespace adiscope {
class PreferenceMenu : public QWidget
{
	Q_OBJECT

public:
	explicit PreferenceMenu(QWidget *parent = 0);
	~PreferenceMenu();

	void insertOption(PreferenceOption *option, const char *slot);
	void load();

private:
	Ui::PreferenceMenu *p_ui;
};
}

#endif // PREFERENCE_MENU_H
