#ifndef SWIOTGENERICMENU_H
#define SWIOTGENERICMENU_H

#include <QWidget>
#include "src/refactoring/maingui/generic_menu.hpp"
#include "src/ad74413r/buffermenu.hpp"
#include <QMap>
#include <QObject>

namespace adiscope {
class CustomSwitch;
class GenericMenu;

namespace swiot {
class BufferMenuView : public gui::GenericMenu {
	Q_OBJECT
public:
	explicit BufferMenuView(QWidget *parent = nullptr);

	~BufferMenuView();

	void init(QString title, QString function, QColor *color);

	void initAdvMenu(QMap<QString, QStringList> values);

	BufferMenu *getAdvMenu();

private:
	BufferMenu *m_swiotAdvMenu;
	adiscope::gui::SubsectionSeparator *m_advanceSettingsSection;
};
}
}

#endif // SWIOTGENERICMENU_H
