#ifndef LOGIC_ANALYZER_CHANNEL_MENU_HPP
#define LOGIC_ANALYZER_CHANNEL_MENU_HPP

#include <QWidget>

namespace Ui {
class LogicAnalyzerChannelMenu;
}

namespace scopy {
namespace gui {

class LogicAnalyzerChannelMenu : public QWidget
{
	Q_OBJECT

public:
	explicit LogicAnalyzerChannelMenu(QWidget* parent = nullptr);
	~LogicAnalyzerChannelMenu();

private:
	void initUi();

private:
	Ui::LogicAnalyzerChannelMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // LOGIC_ANALYZER_CHANNEL_MENU_HPP
