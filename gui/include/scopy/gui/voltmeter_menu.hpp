#ifndef VOLTMETER_MENU_HPP
#define VOLTMETER_MENU_HPP

#include <QWidget>

namespace Ui {
class VoltmeterMenu;
}

namespace scopy {
namespace gui {

class PositionSpinButton;

class VoltmeterMenu : public QWidget
{
	Q_OBJECT

public:
	explicit VoltmeterMenu(QWidget* parent = nullptr);
	~VoltmeterMenu();

private:
	void initUi();
	void enableDataLogging(bool en);

private:
	Ui::VoltmeterMenu* m_ui;

	PositionSpinButton* m_dataLoggingTimer;
};
} // namespace gui
} // namespace scopy

#endif // VOLTMETER_MENU_HPP
