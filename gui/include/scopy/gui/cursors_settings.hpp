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

private:
	Ui::CursorsSettings* m_ui;

public:
	void setCursorReadoutsVisible(bool visible);
};
} // namespace gui
} // namespace scopy

#endif // CURSORSSETTINGS_HPP
