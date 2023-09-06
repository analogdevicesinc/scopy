#ifndef REGISTERMAPINSTRUMENT_HPP
#define REGISTERMAPINSTRUMENT_HPP

#include <iio.h>
#include <QMap>
#include <QMap>
#include <QWidget>
#include <tool_view.hpp>
#include <tabinfo.hpp>
#include "scopy-regmapplugin_export.h"

class QComboBox;
class QVBoxLayout;
class QTabWidget;

namespace scopy::regmap {
class RegisterMapValues;
class DeviceRegisterMap;
class RegisterMapSettingsMenu;
class SearchBarWidget;

class SCOPY_REGMAPPLUGIN_EXPORT RegisterMapInstrument : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterMapInstrument(QWidget *parent = nullptr);
    ~RegisterMapInstrument();

    void addTab(struct iio_device *dev, QString title, QString xmlPath);
    void addTab(struct iio_device *dev, QString title);
    void addTab(QString filePath, QString title);
signals:

private:
    QVBoxLayout *layout;
    QWidget *mainWidget;
    scopy::gui::ToolView *toolView;

    RegisterMapValues* getRegisterMapValues(struct iio_device *dev);
    RegisterMapValues* getRegisterMapValues(QString filePath);

    QMap<QString , DeviceRegisterMap*> *tabs;
    QMap<QString , TabInfo*> *tabsInfo;
    bool first = true;
    QString activeRegisterMap;
    QComboBox *registerDeviceList;
    void toggleSettingsMenu(QString registerName, bool toggle);
    scopy::regmap::RegisterMapSettingsMenu *settings;
    SearchBarWidget *searchBarWidget;
    void generateDeviceRegisterMap(TabInfo *tabInfo);

private Q_SLOTS:
    void updateActiveRegisterMap(QString registerName);
    void toggleSearchBarVisible(bool visible);
};
}
#endif // REGISTERMAPINSTRUMENT_HPP
