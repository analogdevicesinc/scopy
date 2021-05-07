#ifndef CHANNELSETTINGS_HPP
#define CHANNELSETTINGS_HPP

#include <scopy/gui/generic_menu.hpp>

namespace Ui {
class ChannelSettings;
};

namespace scopy {
namespace gui {

class ChannelSettings : public GenericMenu
{
	Q_OBJECT

public:
	explicit ChannelSettings(GenericMenu* parent = nullptr);
	explicit ChannelSettings(const QString& menuTitle, const QColor* lineColor, GenericMenu* parent = nullptr);
	~ChannelSettings();

public:
	void setMenuButton(bool toggled) override;

	void setMathLayoutVisibility(bool visible);
	void setFilter1Visibility(bool visible);
	void setFilter2Visibility(bool visible);

private:
	void initUi();

private:
	Ui::ChannelSettings* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // CHANNELSETTINGS_HPP
