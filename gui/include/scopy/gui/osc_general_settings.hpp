#ifndef OSCGENERALSETTINGS_HPP
#define OSCGENERALSETTINGS_HPP

#include <QWidget>

namespace Ui {
class OscGeneralSettings;
}

namespace scopy {
namespace gui {

class OscGeneralSettings : public QWidget
{
	Q_OBJECT

public:
	explicit OscGeneralSettings(QWidget* parent = nullptr);
	~OscGeneralSettings();

private:
	Ui::OscGeneralSettings* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // OSCGENERALSETTINGS_HPP
