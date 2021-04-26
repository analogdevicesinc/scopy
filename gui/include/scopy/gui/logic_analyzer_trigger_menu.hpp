#ifndef LOGIC_ANALYZER_TRIGGER_MENU_HPP
#define LOGIC_ANALYZER_TRIGGER_MENU_HPP

#include <QWidget>

namespace Ui {
class LogicAnalyzerTriggerMenu;
}

namespace scopy {
namespace gui {

class LogicAnalyzerTriggerMenu : public QWidget
{
	Q_OBJECT

public:
	explicit LogicAnalyzerTriggerMenu(QWidget* parent = nullptr);
	~LogicAnalyzerTriggerMenu();

private:
	void initUi();

private:
	Ui::LogicAnalyzerTriggerMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // LOGIC_ANALYZER_TRIGGER_MENU_HPP
