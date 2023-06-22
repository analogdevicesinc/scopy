#ifndef REGMAPSTYLEHELPER_HPP
#define REGMAPSTYLEHELPER_HPP

#include <QMap>
#include <QObject>
#include <QPushButton>

namespace scopy::regmap {
class RegmapStyleHelper
{
protected:
    RegmapStyleHelper(QObject *parent = nullptr);
    ~RegmapStyleHelper();

public:
        // singleton
    RegmapStyleHelper(RegmapStyleHelper &other) = delete;
    void operator=(const RegmapStyleHelper &) = delete;
    static RegmapStyleHelper *GetInstance();


    static void initColorMap();
    static QString getColor(QString id);
    static void BlueButton(QPushButton *btn);
    static void BlueButton(QWidget *widget);
    static void PartialFrameWidget(QWidget *widget);
    static void FrameWidget(QWidget *widget);
    static void RegisterMapStyle(QWidget *widget);

private:
    QMap<QString,QString> colorMap;
    static RegmapStyleHelper * pinstance_;
};
}
#endif // REGMAPSTYLEHELPER_HPP
