#ifndef PATTERN_GENERATOR_GENERAL_MENU_HPP
#define PATTERN_GENERATOR_GENERAL_MENU_HPP

#include <QWidget>

namespace Ui {
class PatternGeneratorGeneralMenu;
}

namespace scopy {
namespace gui {

class PatternGeneratorGeneralMenu : public QWidget
{
	Q_OBJECT

public:
	explicit PatternGeneratorGeneralMenu(QWidget* parent = nullptr);
	~PatternGeneratorGeneralMenu();

private:
	void initUi();

private:
	Ui::PatternGeneratorGeneralMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // PATTERN_GENERATOR_GENERAL_MENU_HPP
