#ifndef PATTERN_GENERATOR_CHANNEL_MENU_HPP
#define PATTERN_GENERATOR_CHANNEL_MENU_HPP

#include <QWidget>

namespace Ui {
class PatternGeneratorChannelMenu;
}

namespace scopy {
namespace gui {

class PatternGeneratorChannelMenu : public QWidget
{
	Q_OBJECT

public:
	explicit PatternGeneratorChannelMenu(QWidget* parent = nullptr);
	~PatternGeneratorChannelMenu();

private:
	void initUi();

private:
	Ui::PatternGeneratorChannelMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // PATTERN_GENERATOR_CHANNEL_MENU_HPP
