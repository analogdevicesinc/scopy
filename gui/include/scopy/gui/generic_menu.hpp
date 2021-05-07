#ifndef GENERICMENU_HPP
#define GENERICMENU_HPP

#include <QWidget>

class GenericMenu : public QWidget
{
	Q_OBJECT

public:
	explicit GenericMenu(QWidget* parent = nullptr);

	virtual void setMenuButton(bool toggled) = 0;

Q_SIGNALS:
	void enableBtnToggled(bool toggled);
};

#endif // GENERICMENU_HPP
