#ifndef LOGICAN_ALYZER_MENU_HPP
#define LOGICAN_ALYZER_MENU_HPP

#include <QWidget>

namespace Ui {
class LogicAnalyzerMenu;
}

namespace scopy {
namespace gui {

class ScaleSpinButton;
class PositionSpinButton;

class LogicAnalyzerMenu : public QWidget
{
	Q_OBJECT

public:
	explicit LogicAnalyzerMenu(QWidget* parent = nullptr);
	~LogicAnalyzerMenu();

private:
	Ui::LogicAnalyzerMenu* m_ui;

	ScaleSpinButton* m_sampleRateButton;
	ScaleSpinButton* m_bufferSizeButton;
	PositionSpinButton* m_timePositionButton;

	int m_nbChannels;
};
} // namespace gui
} // namespace scopy

#endif // LOGICAN_ALYZER_MENU_HPP
