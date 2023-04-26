#ifndef REGISTERCONTROLLER_HPP
#define REGISTERCONTROLLER_HPP

#include <QWidget>
#include "scopyregmap_export.h"

class QSpinBox;
class QTextSpinBox;
class QLabel;
class QPushButton;
class QLineEdit;
class QVBoxLayout;
class RegisterModel;

class SCOPYREGMAP_EXPORT RegisterController : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterController(QWidget *parent = nullptr);

	~RegisterController();

	void registerChanged(uint32_t address);
    void registerValueChanged(QString value);

private:

	QVBoxLayout *layout;
	QLineEdit *regValue;
	QPushButton *readButton;
	QPushButton *writeButton;
	QSpinBox *addressPicker;
	bool addressChanged = false;

Q_SIGNALS:
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);
	void registerAddressChanged(uint32_t address);
	void valueChanged(QString value);
};

#endif // REGISTERCONTROLLER_HPP
