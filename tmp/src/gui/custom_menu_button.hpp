#ifndef CUSTOMMENUBUTTON_H
#define CUSTOMMENUBUTTON_H

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

#include "customPushButton.hpp"

using namespace adiscope;

namespace Ui {
class CustomMenuButton;
}

namespace adiscope {
namespace gui {

class CustomMenuButton : public QWidget
{
	Q_OBJECT

public:
	explicit CustomMenuButton(QString labelText = nullptr, bool checkboxVisible = false,
				  bool checkBoxChecked = false, QWidget* parent = nullptr);
	explicit CustomMenuButton(QWidget* parent = nullptr);
	~CustomMenuButton();

	void setLabel(const QString& text);
	void setCheckboxVisible(bool visible);

	CustomPushButton* getBtn();
	QCheckBox* getCheckBox();
	bool getCheckBoxState();
	void setCheckBoxState(bool checked);
	void setMenuFloating(bool floating);

public Q_SLOTS:
	void checkBoxToggled(bool toggled);

private:
	Ui::CustomMenuButton* m_ui;

	bool m_floatingMenu;
};
} // namespace gui
} // namespace scopy

#endif // CUSTOMMENUBUTTON_H
