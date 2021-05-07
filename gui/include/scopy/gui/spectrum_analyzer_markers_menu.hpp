#ifndef SPECTRUM_ANALYZER_MARKERS_MENU_HPP
#define SPECTRUM_ANALYZER_MARKERS_MENU_HPP

#include <scopy/gui/db_click_buttons.hpp>
#include <scopy/gui/fft_display_plot.hpp>
#include <scopy/gui/generic_menu.hpp>
#include <scopy/gui/spinbox_a.hpp>

namespace Ui {
class SpectrumAnalyzerMarkersMenu;
}

namespace scopy {
namespace gui {

class SpectrumAnalyzerMarkersMenu : public GenericMenu
{
	Q_OBJECT

public:
	explicit SpectrumAnalyzerMarkersMenu(GenericMenu* parent = nullptr);
	~SpectrumAnalyzerMarkersMenu();

	void setMenuButton(bool toggled) override{};

private:
	void initUi();

private:
	Ui::SpectrumAnalyzerMarkersMenu* m_ui;

	PositionSpinButton* m_markerFreqPos;

	DbClickButtons* m_markerSelector;
	std::vector<std::pair<QString, FftDisplayPlot::MarkerType>> m_markerTypes;
};
} // namespace gui
} // namespace scopy

#endif // SPECTRUM_ANALYZER_MARKERS_MENU_HPP
