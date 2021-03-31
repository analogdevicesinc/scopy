#ifndef SPECTRUM_ANALYZER_MENU_HPP
#define SPECTRUM_ANALYZER_MENU_HPP

#include <QWidget>

namespace Ui {
class SpectrumAnalyzerMenu;
}

namespace scopy {
namespace gui {

class SpectrumAnalyzerMenu : public QWidget
{
	Q_OBJECT

public:
	explicit SpectrumAnalyzerMenu(QWidget* parent = nullptr);
	~SpectrumAnalyzerMenu();

private:
	Ui::SpectrumAnalyzerMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // SPECTRUM_ANALYZER_MENU_HPP
