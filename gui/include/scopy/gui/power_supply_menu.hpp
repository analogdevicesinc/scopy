#ifndef POWER_SUPPLY_MENU_HPP
#define POWER_SUPPLY_MENU_HPP

#include <QWidget>

namespace Ui {
class PowerSupplyMenu;
}

namespace scopy {
namespace gui {

class PositionSpinButton;

class PowerSupplyMenu : public QWidget
{
	Q_OBJECT

public:
	explicit PowerSupplyMenu(QWidget* parent = nullptr);
	~PowerSupplyMenu();

private:
	void initUi();

private:
	Ui::PowerSupplyMenu* m_ui;

	PositionSpinButton* m_valuePos;
	PositionSpinButton* m_valueNeg;
};
} // namespace gui
} // namespace scopy

#endif // POWER_SUPPLY_MENU_HPP
