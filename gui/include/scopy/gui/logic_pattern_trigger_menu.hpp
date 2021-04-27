#ifndef LOGIC_PATTERN_TRIGGER_MENU_HPP
#define LOGIC_PATTERN_TRIGGER_MENU_HPP

#include <QWidget>

namespace Ui {
class TriggerMenu;
}

namespace scopy {
namespace gui {

class TriggerMenu : public QWidget
{
	Q_OBJECT

public:
	explicit TriggerMenu(QWidget* parent = nullptr);
	~TriggerMenu();

private:
	void initUi();

private:
	Ui::TriggerMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // LOGIC_PATTERN_TRIGGER_MENU_HPP
