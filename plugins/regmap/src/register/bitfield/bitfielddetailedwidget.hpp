#ifndef BitFieldDetailedWidget_HPP
#define BitFieldDetailedWidget_HPP

#include <QWidget>

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QVBoxLayout;

class BitFieldDetailedWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BitFieldDetailedWidget(QString name,
				int defaultValue,
				QString description,
				int width,
				QString notes,
				int regOffset,
				QMap<QString,QString> *options,
				QWidget *parent = nullptr);
	~BitFieldDetailedWidget();

	QString getToolTip() const;
	void updateValue(QString newValue);
	QString getValue();

	int getWidth() const;

	int getRegOffset() const;

private:
	QVBoxLayout *layout;
	QString toolTip;
	int width;
	QString description;
	bool reserved;
	int regOffset;

	QMap<QString,QString> *options;
	QLabel *value;
	QComboBox *valueComboBox = nullptr;
	QCheckBox *valueCheckBox = nullptr;
	QLineEdit *valueLineEdit = nullptr;

	void firstRead();

Q_SIGNALS:
    void valueUpdated(QString value);

};

#endif // BitFieldDetailedWidget_HPP
