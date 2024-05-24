#ifndef SETTINGSCUSTOMWIDGET_H
#define SETTINGSCUSTOMWIDGET_H

#include "scopy-m2k-gui_export.h"
#include "customPushButton.h"

#include <QButtonGroup>
#include <QWidget>

using namespace scopy;

namespace Ui {
class SettingsPairWidget;
}

namespace scopy {
namespace m2kgui {
class SCOPY_M2K_GUI_EXPORT SettingsPairWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsPairWidget(QWidget *parent = nullptr);
	~SettingsPairWidget();

	CustomPushButton *getGeneralSettingsBtn();
	QPushButton *getSettingsBtn();

private:
	Ui::SettingsPairWidget *m_ui;
};
} // namespace m2kgui
} // namespace scopy

#endif // SETTINGSPAIRWIDGET_H
