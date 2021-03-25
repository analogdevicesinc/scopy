#ifndef TOOLLAUNCHER_HPP
#define TOOLLAUNCHER_HPP

#include <QMainWindow>

#include <scopy/gui/connect_dialog.hpp>
#include <scopy/gui/scopy_about_dialog.hpp>
#include <scopy/gui/tool_menu.hpp>
#include <scopy/gui/user_notes.hpp>

namespace Ui {
class ToolLauncher;
}

namespace scopy {
namespace gui {

class ToolLauncher : public QMainWindow
{
	Q_OBJECT
public:
	explicit ToolLauncher(QWidget* parent = nullptr);

private:
	void swapMenu(QWidget* menu);

private:
	Ui::ToolLauncher* m_ui;

	ToolMenu* m_toolMenu;

	ScopyAboutDialog* m_about;

	QWidget* m_current;
	QSettings* m_settings;
	Preferences* m_prefPanel;
	UserNotes* m_notesPanel;

private Q_SLOTS:
	void onBtnHomeClicked();
};
} // namespace gui
} // namespace scopy

#endif // TOOLLAUNCHER_HPP
