#ifndef PATTERN_GENERATOR_MENU_HPP
#define PATTERN_GENERATOR_MENU_HPP

#include <QWidget>

namespace Ui {
class PatternGeneratorMenu;
}

namespace scopy {
namespace gui {

class PatternGeneratorMenu : public QWidget
{
	Q_OBJECT

public:
	explicit PatternGeneratorMenu(QWidget* parent = nullptr);
	~PatternGeneratorMenu();

private:
	Ui::PatternGeneratorMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // PATTERN_GENERATOR_MENU_HPP
