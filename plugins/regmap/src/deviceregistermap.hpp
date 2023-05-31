#ifndef DEVICEREGISTERMAP_HPP
#define DEVICEREGISTERMAP_HPP

#include <QMap>
#include <QObject>
#include <QWidget>
#include "scopyregmap_export.h"

class RegisterMapTable;
class QVBoxLayout;
class RegisterModel;
class RegisterDetailedWidget;
class RegisterController;
class RegisterMapValues;
class RegisterMapTemplate;
class SearchBarWidget;


class QMainWindow;

class QDockWidget;
namespace scopy::regmap{
namespace gui {
class RegisterMapSettingsMenu;
}
class SCOPYREGMAP_EXPORT DeviceRegisterMap : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate = nullptr, RegisterMapValues *registerMapValues = nullptr,  QWidget *parent = nullptr);
    ~DeviceRegisterMap();

    void registerChanged(RegisterModel *regModel);
    void toggleAutoread(bool toggled);
    void applyFilters(QString filter);
    bool hasTemplate();

private:

    bool autoread = false;
    QVBoxLayout *deviceRegisterMapLayout;
    QMainWindow *mainWindow;
    RegisterMapTemplate *registerMapTemplate ;
    RegisterMapValues *registerMapValues;
    RegisterController *registerController = nullptr;

    RegisterMapTable *registerMapTableWidget = nullptr;
    QDockWidget *docRegisterMapTable = nullptr;

    RegisterDetailedWidget *registerDetailedWidget = nullptr;
    QDockWidget *dockRegisterDetailedWidget = nullptr;
    gui::RegisterMapSettingsMenu *settings;
    void initSettings();
    int selectedRegister;
Q_SIGNALS:
    void requestRead(uint32_t address);
    void requestWrite(uint32_t address, uint32_t value);
    void requestRegisterDump(QString path);


};
}
#endif // DEVICEREGISTERMAP_HPP
