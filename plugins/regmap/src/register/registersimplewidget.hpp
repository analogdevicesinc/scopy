#ifndef REGISTERSIMPLEWIDGET_HPP
#define REGISTERSIMPLEWIDGET_HPP

#include <QColor>
#include <QFrame>
#include <QWidget>
#include <qstring.h>

class QLabel;
class QHBoxLayout;

namespace scopy::regmap{
namespace gui {
class BitFieldSimpleWidget;

class RegisterSimpleWidget : public QFrame
{
    Q_OBJECT
public:
    explicit RegisterSimpleWidget(QString name,
                                  QString address,
                                  QString description,
                                  QString notes,
                                  int registerWidth,
                                  QVector<BitFieldSimpleWidget*> *bitFields,
                                  QWidget *parent = nullptr);

    ~RegisterSimpleWidget();


    void valueUpdated(uint32_t value);
    void setRegisterSelected(bool selected);

public Q_SLOTS:
    void checkPreferences();

private:
    QHBoxLayout *layout;
    QLabel *value;
    QVector<BitFieldSimpleWidget*> *bitFields;
    QString address;
    int registerWidth;
    QFrame *regBaseInfoWidget;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
Q_SIGNALS:
    void registerSelected(uint32_t address);
};
}
}
#endif // REGISTERSIMPLEWIDGET_HPP
