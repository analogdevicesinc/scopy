#ifndef CUSTOMMENUBUTTON_H
#define CUSTOMMENUBUTTON_H

#include "scopy-m2k-gui_export.h"
#include "customPushButton.h"

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

using namespace scopy;

namespace Ui {
class CustomMenuButton;
}

namespace scopy {
namespace m2kgui {

class SCOPY_M2K_GUI_EXPORT CustomMenuButton : public QWidget
{
	Q_OBJECT

public:
	explicit CustomMenuButton(QString labelText = nullptr, bool checkboxVisible = false,
				  bool checkBoxChecked = false, QWidget *parent = nullptr);
	explicit CustomMenuButton(QWidget *parent = nullptr);
	~CustomMenuButton();

	void setLabel(const QString &text);
	void setCheckboxVisible(bool visible);

	CustomPushButton *getBtn();
	QCheckBox *getCheckBox();
	bool getCheckBoxState();
	void setCheckBoxState(bool checked);
	void setMenuFloating(bool floating);

public Q_SLOTS:
	void checkBoxToggled(bool toggled);

private:
	Ui::CustomMenuButton *m_ui;

	bool m_floatingMenu;
};
} // namespace m2kgui
} // namespace scopy

#endif // CUSTOMMENUBUTTON_H
