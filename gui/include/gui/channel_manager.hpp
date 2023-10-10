#ifndef CHANNEL_MANAGER_HPP
#define CHANNEL_MANAGER_HPP

#include "channel_widget.hpp"
#include "customPushButton.h"
#include "scopy-gui_export.h"

#include <QLabel>
#include <QScrollArea>
#include <QStackedWidget>
#include <QWidget>

namespace scopy {
namespace gui {

enum class ChannelsPositionEnum
{
	VERTICAL,
	HORIZONTAL
};

class SCOPY_GUI_EXPORT ChannelManager : public QWidget
{
	Q_OBJECT

public:
	explicit ChannelManager(ChannelsPositionEnum position = ChannelsPositionEnum::HORIZONTAL,
				QWidget *parent = nullptr);
	~ChannelManager();

	void build(QWidget *parent);
	void insertAddBtn(QWidget *menu, bool dockable);

	ChannelWidget *buildNewChannel(int chId, bool deletable, bool simplefied, QColor color, const QString &fullName,
				       const QString &shortName);
	void removeChannel(ChannelWidget *ch);
	QList<ChannelWidget *> getEnabledChannels();

	CustomPushButton *getAddChannelBtn();
	QList<ChannelWidget *> getChannelsList();
	int getChannelsCount();

	void setChannelAlignment(ChannelWidget *ch, Qt::Alignment alignment);
	void setChannelIdVisible(bool visible);
	const QString &getToolStatus() const;
	void setToolStatus(const QString &newToolStatus);
	void enableSwitchButton(bool en);
	int getChannelID(ChannelWidget *ch);
	int getSelectedChannel();
	const ChannelWidget *getChannelAt(int id);
	void updatePosition(ChannelsPositionEnum position);
public Q_SLOTS:
	void changeParent(QWidget *newParent);
	void toggleChannelManager(bool toggled);

	void onChannelSelected(bool toggled);
	void onChannelEnabled(bool toggled);
	void onChannelDeleted();

Q_SIGNALS:
	void configureAddBtn(QWidget *menu, bool dockable);
	void positionChanged(ChannelsPositionEnum position);
	void channelManagerToggle(bool toggled);

	void selectedChannel(int id, bool toggled);
	void enabledChannel(int id, bool toggled);
	void deletedChannel(QString name);

private:
	QWidget *m_parent;
	QWidget *header;
	QScrollArea *m_scrollArea;
	QWidget *m_channelsWidget;
	bool m_hasAddBtn;
	QPushButton *m_switchBtn;
	CustomPushButton *m_addChannelBtn;
	ChannelsPositionEnum m_position;
	QList<ChannelWidget *> m_channelsList;
	bool m_channelIdVisible;
	QPushButton *toggleChannels;
	bool channelManagerToggled;
	QLabel *toolStatus;
	int m_maxChannelWidth;
	int m_minChannelWidth;
	int m_selectedChannel;
};
} // namespace gui
} // namespace scopy

#endif // CHANNEL_MANAGER_HPP
