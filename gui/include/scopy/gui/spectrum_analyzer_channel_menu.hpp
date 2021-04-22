#ifndef SPECTRUM_ANALYZER_CHANNEL_MENU_HPP
#define SPECTRUM_ANALYZER_CHANNEL_MENU_HPP

#include <QWidget>

#include <scopy/gui/fft_display_plot.hpp>

namespace Ui {
class SpectrumAnalyzerChannelMenu;
}

namespace scopy {
namespace gui {

class SpectrumAnalyzerChannelMenu : public QWidget
{
	Q_OBJECT

public:
	enum class FftWinType
	{
		FLAT_TOP = 0,
		RECTANGULAR = 1,
		TRIANGULAR = 2,
		HAMMING = 3,
		HANN = 4,
		BLACKMAN_HARRIS = 5,
		KAISER = 6,
	};

	explicit SpectrumAnalyzerChannelMenu(QWidget* parent = nullptr);
	~SpectrumAnalyzerChannelMenu();

private:
	void initUi();

private:
	Ui::SpectrumAnalyzerChannelMenu* m_ui;

	std::vector<std::pair<QString, FftDisplayPlot::AverageType>> m_avgTypes;
	std::vector<std::pair<QString, FftWinType>> m_winTypes;
};
} // namespace gui
} // namespace scopy

#endif // SPECTRUM_ANALYZER_CHANNEL_MENU_HPP
