#ifndef REGISTERMAPSETTINGSMENU_HPP
#define REGISTERMAPSETTINGSMENU_HPP

#include <QObject>
#include <QWidget>
#include <generic_menu.hpp>

namespace  scopy {
namespace regmap {
namespace gui {

class RegisterMapSettingsMenu : public ::scopy::gui::GenericMenu
{
    Q_OBJECT
public:
    explicit RegisterMapSettingsMenu(QWidget *parent = nullptr);

Q_SIGNALS:
    void autoreadToggled(bool toggled);
    void requestRead(int address);
    void requestWrite(uint32_t address, uint32_t value);
    void requestRegisterDump(QString path);
};
}
}
}
#endif // REGISTERMAPSETTINGSMENU_HPP
