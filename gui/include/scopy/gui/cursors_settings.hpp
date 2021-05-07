#ifndef CURSORSSETTINGS_HPP
#define CURSORSSETTINGS_HPP

#include <scopy/gui/generic_menu.hpp>

namespace Ui {
class CursorsSettings;
};

namespace scopy {
namespace gui {

class CursorsSettings : public GenericMenu
{
	Q_OBJECT

public:
	explicit CursorsSettings(GenericMenu* parent = nullptr);
	~CursorsSettings();

	void setBtnNormalTrackVisible(bool visible);
	void setHorizontalVisible(bool visible);
	void setVerticalVisible(bool visible);
	void setCursorReadoutsVisible(bool visible);

	void setMenuButton(bool toggled) override;

private:
	void initUi();

private:
	Ui::CursorsSettings* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // CURSORSSETTINGS_HPP
