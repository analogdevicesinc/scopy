#ifndef REGISTERMAPSETTINGSMENU_HPP
#define REGISTERMAPSETTINGSMENU_HPP

#include <QObject>
#include <QWidget>

#include <generic_menu.hpp>

class QCheckBox;

class QLineEdit;
namespace scopy {
namespace regmap {

class RegisterMapSettingsMenu : public ::scopy::gui::GenericMenu
{
	Q_OBJECT
public:
	explicit RegisterMapSettingsMenu(QWidget *parent = nullptr);

Q_SIGNALS:
	void autoreadToggled(bool toggled);
	void requestRead(int address);
	void requestWrite(uint32_t address, uint32_t value);
	void requestRegisterDump(QString path);

private:
	void applyStyle();
	QCheckBox *autoread;
	QLabel *hexaPrefix1;
	QLabel *hexaPrefix2;
	QLineEdit *startReadInterval;
	QLineEdit *endReadInterval;
	QPushButton *readInterval;
	QLineEdit *filePath;
	QPushButton *writeListOfValuesButton;
	QPushButton *registerDump;
	QPushButton *pathButton;
};
} // namespace regmap
} // namespace scopy
#endif // REGISTERMAPSETTINGSMENU_HPP
