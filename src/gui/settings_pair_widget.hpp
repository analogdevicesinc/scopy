#ifndef SETTINGSCUSTOMWIDGET_H
#define SETTINGSCUSTOMWIDGET_H

#include <QButtonGroup>
#include <QWidget>

#include "customPushButton.hpp"

using namespace adiscope;

namespace Ui {
class SettingsPairWidget;
}

namespace adiscope {
namespace gui {
class SettingsPairWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsPairWidget(QWidget* parent = nullptr);
	~SettingsPairWidget();

	CustomPushButton* getGeneralSettingsBtn();
	QPushButton* getSettingsBtn();

private:
	Ui::SettingsPairWidget* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // SETTINGSPAIRWIDGET_H
