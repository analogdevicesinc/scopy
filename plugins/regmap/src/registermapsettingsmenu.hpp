#ifndef REGISTERMAPSETTINGSMENU_HPP
#define REGISTERMAPSETTINGSMENU_HPP

#include <QObject>
#include <QPushButton>
#include <QWidget>
#include <menuheader.h>

class QCheckBox;

class QLineEdit;
namespace scopy {
namespace regmap {

class RegisterMapSettingsMenu : public QWidget
{
	friend class RegmapStyleHelper;

	Q_OBJECT

public:
	explicit RegisterMapSettingsMenu(QWidget *parent = nullptr);

	void startTutorial();

Q_SIGNALS:
	void autoreadToggled(bool toggled);
	void requestRead(int address);
	void requestWrite(uint32_t address, uint32_t value);
	void requestRegisterDump(QString path);
	void tutorialDone();
	void tutorialAborted();

private:
	MenuHeaderWidget *header;
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
