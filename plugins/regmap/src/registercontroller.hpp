#ifndef REGISTERCONTROLLER_HPP
#define REGISTERCONTROLLER_HPP

#include <QWidget>
#include "titlespinbox.hpp"
#include "scopy-regmap_export.h"

#include <QWidget>

class QHBoxLayout;
class QSpinBox;
class TextSpinBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;

namespace scopy::regmap {
class RegisterModel;
class RegisterValueWidget;
class AddressPicker;

class SCOPY_REGMAP_EXPORT RegisterController : public QWidget
{
	friend class RegMap_API;
	Q_OBJECT
public:
	explicit RegisterController(QWidget *parent = nullptr);

	~RegisterController();

	void registerChanged(uint32_t address);
	void registerValueChanged(QString value);
	QString getAddress();
	void setHasMap(bool hasMap);

private:
	QHBoxLayout *mainLayout;

	TitleSpinBox *adrPck = nullptr;

	QSpinBox *addressPicker;
	QPushButton *readButton;
	QPushButton *writeButton;
	QPushButton *detailedRegisterToggle = nullptr;
	QLineEdit *regValue;
	QLabel *addressLabel;
	QLabel *valueLabel;
	QHBoxLayout *writeWidgetLayout;

	bool addressChanged = false;
	bool hasMap = false;
	void applyStyle();

Q_SIGNALS:
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);
	void registerAddressChanged(uint32_t address);
	void valueChanged(QString value);
	void toggleDetailedMenu(bool toggled);
};
} // namespace scopy::regmap
#endif // REGISTERCONTROLLER_HPP
