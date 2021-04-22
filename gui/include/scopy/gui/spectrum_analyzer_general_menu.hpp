#ifndef SPECTRUM_ANALYZER_GENERAL_MENU_HPP
#define SPECTRUM_ANALYZER_GENERAL_MENU_HPP

#include <QWidget>

namespace Ui {
class SpectrumAnalyzerGeneralMenu;
}

namespace scopy {
namespace gui {

class SpectrumAnalyzerGeneralMenu : public QWidget
{
	Q_OBJECT

public:
	explicit SpectrumAnalyzerGeneralMenu(QWidget* parent = nullptr);
	~SpectrumAnalyzerGeneralMenu();

private Q_SLOTS:
	void btnExportClicked();

private:
	Ui::SpectrumAnalyzerGeneralMenu* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // SPECTRUM_ANALYZER_GENERAL_MENU_HPP
