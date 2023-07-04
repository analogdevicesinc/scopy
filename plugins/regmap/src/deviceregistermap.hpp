#ifndef DEVICEREGISTERMAP_HPP
#define DEVICEREGISTERMAP_HPP

#include <QMap>
#include <QObject>
#include <QWidget>
#include "scopy-regmapplugin_export.h"

class QVBoxLayout;

class QDockWidget;

namespace scopy::regmap{
class RegisterMapTemplate;
class RegisterModel;
class RegisterMapValues;

namespace gui {
class RegisterMapSettingsMenu;
class RegisterDetailedWidget;
class RegisterController;
class SearchBarWidget;
class RegisterMapTable;
}



class SCOPY_REGMAPPLUGIN_EXPORT DeviceRegisterMap : public QWidget
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
    QVBoxLayout *layout;
    RegisterMapTemplate *registerMapTemplate ;
    RegisterMapValues *registerMapValues;
    gui::RegisterController *registerController = nullptr;

    gui::RegisterMapTable *registerMapTableWidget = nullptr;
    QDockWidget *docRegisterMapTable = nullptr;

    gui::RegisterDetailedWidget *registerDetailedWidget = nullptr;
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
