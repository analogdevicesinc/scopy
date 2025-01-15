#ifndef ADMTSTYLEHELPER_H
#define ADMTSTYLEHELPER_H

#include "scopy-admt_export.h"
#include "stylehelper.h"

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QIcon>
#include <QMap>
#include <QString>

#include <plotwidget.h>

namespace scopy {
namespace admt{
class SCOPY_ADMT_EXPORT ADMTStyleHelper : public QObject
{
    Q_OBJECT
protected:
    ADMTStyleHelper(QObject *parent = nullptr);
    ~ADMTStyleHelper();
public:
    // singleton
    ADMTStyleHelper(ADMTStyleHelper &other) = delete;
    void operator=(const ADMTStyleHelper &) = delete;
    static ADMTStyleHelper *GetInstance();
public:
    static void initColorMap();
    static QString getColor(QString id);
    static void TopContainerButtonStyle(QPushButton *btn, QString objectName = "");
    static void PlotWidgetStyle(PlotWidget *widget, QString objectName = "");
    static void ComboBoxStyle(QComboBox *widget, QString objectName = "");
    static void LineEditStyle(QLineEdit *widget, QString objectName = "");
    static void ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName = "");
    static void StartButtonStyle(QPushButton *btn, QString objectName = "");
    static void TabWidgetStyle(QTabWidget *widget, const QString& styleHelperColor = "ScopyBlue", QString objectName = "");
    static void TextStyle(QWidget *widget, const QString& styleHelperColor, bool isBold = false, QString objectName = "");
    static void MenuSmallLabel(QLabel *label, QString objectName = "");
    static void LineStyle(QFrame *line, QString objectName = "");
    static void UIBackgroundStyle(QWidget *widget, QString objectName = "");
    static void GraphChannelStyle(QWidget *widget, QLayout *layout, QString objectName = "");
    static void CalculatedCoeffWidgetRowStyle(QWidget *widget, QHBoxLayout *layout, QLabel *hLabel, QLabel *hMagLabel, QLabel *hPhaseLabel, QString objectName = "");
private:
    QMap<QString, QString> colorMap;
    static ADMTStyleHelper *pinstance_;
};
} // namespace admt
} // namespace scopy

#endif // ADMTSTYLEHELPER_H