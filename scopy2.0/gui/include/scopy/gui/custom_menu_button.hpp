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
	explicit CustomMenuButton(QWidget* parent = nullptr, QString labelText = nullptr, bool checkboxVisible = false);
	~CustomMenuButton();

private:
	Ui::CustomMenuButton* m_ui;

	CustomPushButton* getButton();
	QLabel* getLabel();
	QCheckBox* getCheckBox();
};
} // namespace gui
} // namespace scopy

#endif // CUSTOMMENUBUTTON_H
