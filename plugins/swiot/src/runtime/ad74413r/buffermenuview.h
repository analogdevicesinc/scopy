#ifndef SWIOTGENERICMENU_H
#define SWIOTGENERICMENU_H

#include <QWidget>
#include <gui/generic_menu.hpp>
#include "buffermenu.h"
#include "linked_button.hpp"
#include <QMap>
#include <QObject>

namespace scopy {
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
	void createHeaderWidget(const QString title);

	BufferMenu *getAdvMenu();

private:
	bool eventFilter(QObject *obj, QEvent *event);

	BufferMenu *m_swiotAdvMenu;
	scopy::gui::SubsectionSeparator *m_advanceSettingsSection;
	scopy::LinkedButton *m_btnInfoStatus;
};
}
}

#endif // SWIOTGENERICMENU_H
