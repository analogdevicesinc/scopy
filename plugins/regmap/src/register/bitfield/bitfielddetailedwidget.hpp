#ifndef BitFieldDetailedWidget_HPP
#define BitFieldDetailedWidget_HPP

#include "bitfieldoption.hpp"

#include <QWidget>
#include <QFrame>

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QVBoxLayout;

class QHBoxLayout;
namespace scopy::regmap{

class BitFieldDetailedWidget : public QFrame
{
    friend class RegmapStyleHelper;

	Q_OBJECT
public:
    explicit BitFieldDetailedWidget(QString name,
                                    QString access,
                                    int defaultValue,
                                    QString description,
                                    int width,
                                    QString notes,
                                    int regOffset,
                                    QVector<BitFieldOption*> *options,
                                    QWidget *parent = nullptr);
    ~BitFieldDetailedWidget();

	QString getToolTip() const;
	void updateValue(QString newValue);
    void registerValueUpdated(QString newValue);
	QString getValue();

	int getWidth() const;

	int getRegOffset() const;

private:
    QFrame *mainFrame;
	QVBoxLayout *layout;
	QString toolTip;
	int width;
	QString description;
	bool reserved;
	int regOffset;
    QString access;

    QVector<BitFieldOption*> *options;
    QLabel *nameLabel;
    QLabel *descriptionLabel;
    QLabel *lastReadValue;
    QLabel *defaultValueLabel;
    QLabel *value = nullptr;
	QComboBox *valueComboBox = nullptr;
	QCheckBox *valueCheckBox = nullptr;
    QLineEdit *valueLineEdit = nullptr;

    bool isFirstRead = true;

    void firstRead();

Q_SIGNALS:
    void valueUpdated(QString value);

};
}
#endif // BitFieldDetailedWidget_HPP
