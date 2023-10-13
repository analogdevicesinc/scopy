#ifndef REGISTERDETAILEDWIDGET_HPP
#define REGISTERDETAILEDWIDGET_HPP

#include <QWidget>

class QLabel;
namespace scopy::regmap {
class RegisterModel;
class BitFieldModel;
class BitFieldDetailedWidget;

class RegisterDetailedWidget : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterDetailedWidget(RegisterModel *regModel, QWidget *parent = nullptr);

	void updateBitFieldsValue(uint32_t value);
	void registerValueUpdated(uint32_t value);
	QString getBitFieldsValue();

private:
	QVector<BitFieldDetailedWidget *> *bitFieldList;
	int regWidth;
Q_SIGNALS:
	void bitFieldValueChanged(QString value);
};
} // namespace scopy::regmap

#endif // REGISTERDETAILEDWIDGET_HPP
