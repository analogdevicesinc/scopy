#ifndef SWIOTGENERICMENU_H
#define SWIOTGENERICMENU_H

#include <QWidget>
#include "gui/generic_menu.hpp"
#include "swiotadvmenu.hpp"
#include <QMap>
#include <QObject>

namespace adiscope {
class CustomSwitch;

namespace gui {
class GenericMenu;
class SwiotAdvMenu;

class SwiotGenericMenu : public GenericMenu
{
	Q_OBJECT
public:
	explicit SwiotGenericMenu(QWidget *parent = nullptr);
	~SwiotGenericMenu();

	void init(QString title, QString function, QColor* color);
	void initAdvMenu(QMap<QString, QStringList> values);
	void connectMenuToOsc();

	SwiotAdvMenu* getAdvMenu();

private:
	SwiotAdvMenu *m_swiotAdvMenu;
	SubsectionSeparator *m_advanceSettingsSection;


};
}
}

#endif // SWIOTGENERICMENU_H
