#ifndef CHANNEL_MANAGER_HPP
#define CHANNEL_MANAGER_HPP

#include <QStackedWidget>
#include <QWidget>

#include <scopy/gui/channel_widget.hpp>
#include <scopy/gui/custom_push_button.hpp>

namespace scopy {
namespace gui {

enum class ChannelsPositionEnum
{
	VERTICAL,
	HORIZONTAL
};

class ChannelManager : public QWidget
{
	Q_OBJECT

public:
	explicit ChannelManager(ChannelsPositionEnum position = ChannelsPositionEnum::HORIZONTAL,
				QWidget* parent = nullptr);
	~ChannelManager();

	void build(QWidget* parent);
	void insertAddBtn(QWidget* menu);

	ChannelWidget* buildNewChannel(int chId, bool deletable, bool simplefied, QColor color, const QString& fullName,
				       const QString& shortName);
	void removeChannel(ChannelWidget* ch);
	QList<ChannelWidget*> getEnabledChannels();

	CustomPushButton* getAddChannelBtn();
	QWidget* getChannelsList();

Q_SIGNALS:
	void configureAddBtn(QWidget* menu);

private:
	QWidget* m_parent;
	QWidget* m_channelsList;

	ChannelsPositionEnum m_position;

	bool m_hasAddBtn;
	CustomPushButton* m_addChannelBtn;
};
} // namespace gui
} // namespace scopy

#endif // CHANNEL_MANAGER_HPP
