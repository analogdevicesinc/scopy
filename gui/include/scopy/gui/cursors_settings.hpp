#ifndef CURSORSSETTINGS_HPP
#define CURSORSSETTINGS_HPP

#include <QWidget>

namespace Ui {
class CursorsSettings;
};

namespace scopy {
namespace gui {

class CursorsSettings : public QWidget
{
	Q_OBJECT

public:
	explicit CursorsSettings(QWidget* parent = nullptr);
	~CursorsSettings();

	void setBtnNormalTrackVisible(bool visible);
	void setHorizontalVisible(bool visible);
	void setVerticalVisible(bool visible);
	void setCursorReadoutsVisible(bool visible);

private:
	void initUi();

private:
	Ui::CursorsSettings* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // CURSORSSETTINGS_HPP
