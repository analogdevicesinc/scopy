#ifndef SPECTRUM_ANALYZER_SWEEP_MENU_HPP
#define SPECTRUM_ANALYZER_SWEEP_MENU_HPP

#include <scopy/gui/FftDisplayPlot.h>
#include <scopy/gui/generic_menu.hpp>
#include <scopy/gui/spinbox_a.hpp>
#include <scopy/gui/start_stop_range_widget.hpp>

namespace Ui {
class SpectrumAnalyzerSweepMenu;
}

namespace scopy {
namespace gui {

class SpectrumAnalyzerSweepMenu : public GenericMenu
{
	Q_OBJECT

public:
	explicit SpectrumAnalyzerSweepMenu(GenericMenu* parent = nullptr);
	~SpectrumAnalyzerSweepMenu();

private:
	void initUi();

private:
	Ui::SpectrumAnalyzerSweepMenu* m_ui;

	StartStopRangeWidget* m_startStopRange;
	MetricPrefixFormatter m_freqFormatter;

	ScaleSpinButton* m_topScale;
	ScaleSpinButton* m_bottomScale;
	PositionSpinButton* m_unitPerDiv;
	PositionSpinButton* m_top;
	PositionSpinButton* m_bottom;

	std::vector<std::pair<QString, FftDisplayPlot::MagnitudeType>> m_magTypes;
};
} // namespace gui
} // namespace scopy

#endif // SPECTRUM_ANALYZER_SWEEP_MENU_HPP
