#ifndef CHANNELSETTINGS_HPP
#define CHANNELSETTINGS_HPP

#include <QWidget>

namespace Ui {
class ChannelSettings;
};

namespace scopy {
namespace gui {

class ChannelSettings : public QWidget
{
	Q_OBJECT

public:
	explicit ChannelSettings(QWidget* parent = nullptr);
	~ChannelSettings();

private:
	Ui::ChannelSettings* m_ui;

public:
	void setMathLayoutVisibility(bool visible);
	void setFilter1Visibility(bool visible);
	void setFilter2Visibility(bool visible);
};
} // namespace gui
} // namespace scopy

#endif // CHANNELSETTINGS_HPP
