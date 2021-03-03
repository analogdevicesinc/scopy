#ifndef SETTINGSCUSTOMWIDGET_H
#define SETTINGSCUSTOMWIDGET_H

#include <QButtonGroup>
#include <QWidget>

namespace Ui {
class SettingsPairWidget;
}

namespace scopy {
namespace gui {
class SettingsPairWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsPairWidget(QWidget* parent = nullptr);
	~SettingsPairWidget();

private:
	Ui::SettingsPairWidget* m_ui;
	QButtonGroup* m_buttonGroup;
};
} // namespace gui
} // namespace scopy

#endif // SETTINGSPAIRWIDGET_H
