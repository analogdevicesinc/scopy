#ifndef NETWORK_ANALYZER_CHANNEL_MENU_HPP
#define NETWORK_ANALYZER_CHANNEL_MENU_HPP

#include <QStackedWidget>
#include <QWidget>

#include <scopy/gui/spinbox_a.hpp>
#include <scopy/gui/start_stop_range_widget.hpp>

namespace Ui {
class NetworkAnalyzerChannelMenu;
}

namespace scopy {
namespace gui {

class NetworkAnalyzerChannelMenu : public QWidget
{
	Q_OBJECT

public:
	explicit NetworkAnalyzerChannelMenu(QWidget* parent = nullptr);
	~NetworkAnalyzerChannelMenu();

private:
	void initUi();

	void setMinimumDistanceBetween(SpinBoxA* min, SpinBoxA* max, double distance);

private:
	Ui::NetworkAnalyzerChannelMenu* m_ui;

	QStackedWidget* m_sampleStackedWidget;
	StartStopRangeWidget* m_startStopRange;

	ScaleSpinButton* m_samplesCount;
	ScaleSpinButton* m_samplesPerDecadeCount;
	ScaleSpinButton* m_samplesStepSize;
	ScaleSpinButton* m_amplitude;
	PositionSpinButton* m_offset;
	PositionSpinButton* m_magMax;
	PositionSpinButton* m_magMin;
	PositionSpinButton* m_phaseMax;
	PositionSpinButton* m_phaseMin;
	PositionSpinButton* m_pushDelay;
	PositionSpinButton* m_captureDelay;

	bool m_wasChecked;
};
} // namespace gui
} // namespace scopy

#endif // NETWORK_ANALYZER_CHANNEL_MENU_HPP
