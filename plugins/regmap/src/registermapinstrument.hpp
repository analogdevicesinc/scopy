#ifndef REGISTERMAPINSTRUMENT_HPP
#define REGISTERMAPINSTRUMENT_HPP

#include <QMap>
#include <QMap>
#include <QWidget>
#include <tool_view.hpp>
#include "scopyregmap_export.h"


class QComboBox;

class SearchBarWidget;
namespace scopy { namespace regmap {

namespace gui { class RegisterMapSettingsMenu; }
 class DeviceRegisterMap; } }
class RegisterMapValues;
class QVBoxLayout;
class QTabWidget;

class SCOPYREGMAP_EXPORT RegisterMapInstrument : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterMapInstrument(QWidget *parent = nullptr);
    ~RegisterMapInstrument();
    void addTab(struct iio_device *dev, QString title);
    void addTab(struct iio_device *dev, QString title, QString xmlPath);
    void addTab(QString filePath, QString title);
signals:

private:
    QVBoxLayout *layout;
    QWidget *mainWidget;
    scopy::gui::ToolView *toolView;
    scopy::gui::ChannelManager *channelManager;

    RegisterMapValues* getRegisterMapValues(struct iio_device *dev);
    RegisterMapValues* getRegisterMapValues(QString filePath);

    QMap<QString , scopy::regmap::DeviceRegisterMap*> *tabs;
    bool first = false;
    QString activeRegisterMap;
    QComboBox *registerDeviceList;
    void toggleSettingsMenu(QString registerName, bool toggle);
    scopy::regmap::gui::RegisterMapSettingsMenu *settings;
    SearchBarWidget *searchBarWidget;

private Q_SLOTS:
    void updateActiveRegisterMap(QString registerName);
    void toggleSearchBarVisible(bool visible);
};
#endif // REGISTERMAPINSTRUMENT_HPP
