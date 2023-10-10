#ifndef GENERICMENU_HPP
#define GENERICMENU_HPP

#include "basemenu.h"
#include "menu_header.hpp"
#include "scopy-gui_export.h"
#include "subsection_separator.hpp"

#include <QWidget>

namespace scopy {
namespace gui {

class SCOPY_GUI_EXPORT GenericMenu : public QWidget
{
	Q_OBJECT

public:
	explicit GenericMenu(QWidget *parent = nullptr);
	~GenericMenu();

	virtual void setMenuButton(bool toggled){};

	void initInteractiveMenu();
	void setMenuHeader(const QString &title, const QColor *lineColor, bool hasEnableBtn);
	void insertSection(SubsectionSeparator *section);
	void addNewHeaderWidget(QWidget *widget);

	void setMenuWidget(QWidget *widget);
	void hideEvent(QHideEvent *) override;
	void showEvent(QShowEvent *) override;

Q_SIGNALS:
	void enableBtnToggled(bool toggled);
	void menuVisibilityChanged(bool toggled);

private:
	MenuHeader *m_menuHeader;
	BaseMenu *m_menu;

	int m_lastOpenPosition;
};
} // namespace gui
} // namespace scopy

#endif // GENERICMENU_HPP
