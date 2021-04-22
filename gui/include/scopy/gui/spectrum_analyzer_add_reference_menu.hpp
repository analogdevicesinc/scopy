#ifndef SPECTRUM_ANALYZER_ADD_REFERENCE_MENU_HPP
#define SPECTRUM_ANALYZER_ADD_REFERENCE_MENU_HPP

#include <QWidget>

namespace Ui {
class SpectrumAnalyzerAddReferenceMenu;
}

namespace scopy {
namespace gui {

class SpectrumAnalyzerAddReferenceMenu : public QWidget
{
	Q_OBJECT

public:
	explicit SpectrumAnalyzerAddReferenceMenu(QWidget* parent = nullptr);
	~SpectrumAnalyzerAddReferenceMenu();

private:
	Ui::SpectrumAnalyzerAddReferenceMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // SPECTRUM_ANALYZER_ADD_REFERENCE_MENU_HPP
