#ifndef TESTMENU_HPP
#define TESTMENU_HPP

#include <QWidget>

#include <scopy/gui/generic_menu.hpp>

namespace scopy {
namespace gui {

class TestMenu : public GenericMenu
{
	Q_OBJECT
public:
	TestMenu(QWidget* parent = nullptr);
};
} // namespace gui
} // namespace scopy

#endif // TESTMENU_HPP
