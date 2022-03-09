#ifndef CHANNEL_MANAGER_HPP
#define CHANNEL_MANAGER_HPP

#include <QScrollArea>
#include <QStackedWidget>
#include <QWidget>
#include <QLabel>

#include "channel_widget.hpp"
#include "customPushButton.hpp"

namespace adiscope {
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
	void insertAddBtn(QWidget* menu, bool dockable);

	ChannelWidget* buildNewChannel(int chId, bool deletable, bool simplefied, QColor color, const QString& fullName,
				       const QString& shortName);
	void removeChannel(ChannelWidget* ch);
	QList<ChannelWidget*> getEnabledChannels();

	CustomPushButton* getAddChannelBtn();
	QList<ChannelWidget*> getChannelsList();

	void setChannelAlignment(ChannelWidget* ch,Qt::Alignment alignment);
	void setChannelIdVisible(bool visible);
	const QString &getToolStatus() const;
	void setToolStatus(const QString &newToolStatus);

public Q_SLOTS:
	void changeParent(QWidget* newParent);
	void toggleChannelManager(bool toggled);

Q_SIGNALS:
	void configureAddBtn(QWidget* menu, bool dockable);
	void positionChanged(ChannelsPositionEnum position);
	void channelManagerToggle(bool toggled);

private:
	QWidget* m_parent;
	QScrollArea* m_scrollArea;
	QWidget* m_channelsWidget;
	bool m_hasAddBtn;
	CustomPushButton* m_addChannelBtn;
	ChannelsPositionEnum m_position;
	QList<ChannelWidget*> m_channelsList;
	bool m_channelIdVisible;
	QPushButton *toggleChannels;
	bool channelManagerToggled;
	QLabel *toolStatus;
	int m_maxChannelWidth;
	int m_minChannelWidth;
};
} // namespace gui
} // namespace scopy

#endif // CHANNEL_MANAGER_HPP
