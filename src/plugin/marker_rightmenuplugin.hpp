#ifndef MARKER_RIGHTMENUPLUGIN_HPP
#define MARKER_RIGHTMENUPLUGIN_HPP

#include "plugin/irightmenuplugin.h"
#include <FftDisplayPlot.h>
#include <marker_table.hpp>
#include <ui_marker_menu.h>
#include <ui_marker_table.h>
#include <gui/db_click_buttons.hpp>
#include <gui/generic_menu.hpp>
#include <gui/spinbox_a.hpp>

namespace adiscope {
namespace gui {

class MarkerRightMenuPlugin : public IRightMenuPlugin
{
private:
	FftDisplayPlot *fft_plot;
	Ui::MarkerSettings *m_ui;
	MarkerTable *markerTable;
	CustomMenuButton *menuBtn;
	MetricPrefixFormatter freq_formatter;
	PositionSpinButton *marker_freq_pos;
	DbClickButtons *marker_selector;
	ChannelManager *chManager;
	std::vector<QString> markerTypes;
	QDockWidget *markerTableDocker;
	QLabel *lbl_crtMarkerReading;

public:
	MarkerRightMenuPlugin(QWidget *parent, ToolView* toolView, bool dockable, ChannelManager *chManager = nullptr, FftDisplayPlot *fft_plot = nullptr);
	~MarkerRightMenuPlugin();

	void enableMarkerTable(bool en);

private:
	void init() override;
	void connectSignals();
	void initMenu();
	void initTable();
	void initMarkers();
	void setCurrentMarkerLabelData(int chIdx, int mkIdx);
	void updateWidgetsRelatedToMarker(int mrk_id);

public Q_SLOTS:
	void onMarkerFreqPosChanged(double freq);
	void updateMarkerMenu(unsigned int id);
	void updateCrtMrkLblVisibility();
	void updateMrkFreqPosSpinBtnValue();
	void setMarkerEnabled(int ch_idx, int mrk_idx, bool en);
	void onPlotMarkerSelected(uint chIdx, uint mkIdx);
	void onPlotNewMarkerData();
	void onMarkerSelected(int id);
	void onMarkerToggled(int id, bool on);
	void menuToggled(bool toggled);
	void on_btnLeftPeak_clicked();
	void on_btnRightPeak_clicked();
	void on_btnMaxPeak_clicked();
	void on_btnDnAmplPeak_clicked();
	void on_btnUpAmplPeak_clicked();
};
}
}

#endif // MARKER_RIGHTMENUPLUGIN_HPP
