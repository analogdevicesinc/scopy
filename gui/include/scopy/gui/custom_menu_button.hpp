#ifndef CUSTOMMENUBUTTON_H
#define CUSTOMMENUBUTTON_H

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

#include <scopy/gui/custom_push_button.hpp>

namespace Ui {
class CustomMenuButton;
}

namespace scopy {
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

public Q_SLOTS:
	void checkBoxToggled(bool toggled);

private:
	Ui::CustomMenuButton* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // CUSTOMMENUBUTTON_H
