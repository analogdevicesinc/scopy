#ifndef REGISTERCONTROLLER_HPP
#define REGISTERCONTROLLER_HPP

#include "scopy-regmapplugin_export.h"

#include <QWidget>

class QHBoxLayout;
class QSpinBox;
class QTextSpinBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;

namespace scopy::regmap {
class RegisterModel;
class RegisterValueWidget;
class AddressPicker;

class SCOPY_REGMAPPLUGIN_EXPORT RegisterController : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterController(QWidget *parent = nullptr);

	~RegisterController();

	void registerChanged(uint32_t address);
	void registerValueChanged(QString value);
	void addNameAndDescription(QString name, QString description);

private:
	QHBoxLayout *mainLayout;

	QSpinBox *addressPicker;
	QPushButton *readButton;
	QPushButton *writeButton;
	QLineEdit *regValue;
	QLabel *addressLabel;
	QLabel *valueLabel;

	bool addressChanged = false;
	QLabel *nameLabel = nullptr;
	QLabel *descriptionLabel = nullptr;
	void applyStyle();

Q_SIGNALS:
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);
	void registerAddressChanged(uint32_t address);
	void valueChanged(QString value);
};
} // namespace scopy::regmap
#endif // REGISTERCONTROLLER_HPP
