#ifndef REGISTERMAPINSTRUMENT_HPP
#define REGISTERMAPINSTRUMENT_HPP

#include <QMap>
#include <QMap>
#include <QWidget>
#include <tool_view.hpp>
#include "scopyregmap_export.h"

namespace scopy { namespace regmap { class DeviceRegisterMap; } }
class RegisterMapValues;
class QVBoxLayout;
class QTabWidget;

class SCOPYREGMAP_EXPORT RegisterMapInstrument : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterMapInstrument(QWidget *parent = nullptr);
    ~RegisterMapInstrument();
    void addTab(QWidget *widget, QString title);
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

    QMap<QString , scopy::regmap::DeviceRegisterMap*> *tabs;
    bool first = false;
};
#endif // REGISTERMAPINSTRUMENT_HPP
