#ifndef GENERICMENU_HPP
#define GENERICMENU_HPP

#include <QWidget>

#include <scopy/gui/base_menu.hpp>
#include <scopy/gui/menu_header.hpp>
#include <scopy/gui/subsection_separator.hpp>

namespace scopy {
namespace gui {

class GenericMenu : public QWidget
{
	Q_OBJECT

public:
	explicit GenericMenu(QWidget* parent = nullptr);
	~GenericMenu();

	virtual void setMenuButton(bool toggled){};

	void initInteractiveMenu();
	void setMenuHeader(const QString& title, const QColor* lineColor, bool hasEnableBtn);
	void insertSection(SubsectionSeparator* section);

Q_SIGNALS:
	void enableBtnToggled(bool toggled);

private:
	MenuHeader* m_menuHeader;
	BaseMenu* m_menu;

	int m_lastOpenPosition;
};
} // namespace gui
} // namespace scopy

#endif // GENERICMENU_HPP
