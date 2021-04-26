#ifndef LOGIC_ANALYZER_GENERAL_MENU_HPP
#define LOGIC_ANALYZER_GENERAL_MENU_HPP

#include <QWidget>

#include <scopy/gui/osc_export_settings.hpp>
#include <scopy/gui/spinbox_a.hpp>

namespace Ui {
class LogicAnalyzerGeneralMenu;
}

namespace scopy {
namespace gui {

class LogicAnalyzerGeneralMenu : public QWidget
{
	Q_OBJECT

public:
	explicit LogicAnalyzerGeneralMenu(QWidget* parent = nullptr);
	~LogicAnalyzerGeneralMenu();

private:
	void initUi();

private:
	Ui::LogicAnalyzerGeneralMenu* m_ui;

	ExportSettings* m_exportSettings;

	ScaleSpinButton* m_sampleRateButton;
	ScaleSpinButton* m_bufferSizeButton;
	PositionSpinButton* m_timePositionButton;

	int m_nbChannels;
};
} // namespace gui
} // namespace scopy

#endif // LOGIC_ANALYZER_GENERAL_MENU_HPP
