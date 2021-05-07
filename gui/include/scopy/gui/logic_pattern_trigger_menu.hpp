#ifndef LOGIC_PATTERN_TRIGGER_MENU_HPP
#define LOGIC_PATTERN_TRIGGER_MENU_HPP

#include <scopy/gui/generic_menu.hpp>

namespace Ui {
class TriggerMenu;
}

namespace scopy {
namespace gui {

class TriggerMenu : public GenericMenu
{
	Q_OBJECT

public:
	explicit TriggerMenu(GenericMenu* parent = nullptr);
	~TriggerMenu();

	void setMenuButton(bool toggled) override;

private:
	void initUi();

private:
	Ui::TriggerMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // LOGIC_PATTERN_TRIGGER_MENU_HPP
