#ifndef TOOL_VIEW_HPP
#define TOOL_VIEW_HPP

#include <QButtonGroup>
#include <QMap>
#include <QQueue>
#include <QStackedWidget>
#include <QWidget>

#include <scopy/gui/channel_manager.hpp>
#include <scopy/gui/channel_widget.hpp>
#include <scopy/gui/custom_menu_button.hpp>
#include <scopy/gui/custom_push_button.hpp>
#include <scopy/gui/linked_button.hpp>

namespace Ui {
class ToolView;
}

namespace scopy {
namespace gui {

class ToolView : public QWidget
{
	friend class ToolViewBuilder;

	Q_OBJECT

public:
	explicit ToolView(QWidget* parent = nullptr);
	~ToolView();

	LinkedButton* getHelpBtn();
	void setHelpBtnVisible(bool visible);
	void setUrlHelpBtn(const QString& url);

	QPushButton* getPrintBtn();
	void setPrintBtnVisible(bool visible);

	QPushButton* getRunBtn();
	void setRunBtnVisible(bool visible);

	QPushButton* getSingleBtn();
	void setSingleBtnVisible(bool visible);

	QWidget* getTopExtraWidget();
	void setVisibleTopExtraWidget(bool visible);
	void addTopExtraWidget(QWidget* widget);

	QWidget* getBottomExtraWidget();
	void setVisibleBottomExtraWidget(bool visible);
	void addBottomExtraWidget(QWidget* widget);

	QWidget* getCentralWidget();

	CustomPushButton* getGeneralSettingsBtn();
	QPushButton* getSettingsBtn();
	void setPairSettingsVisible(bool visible);

	void setInstrumentNotesVisible(bool visible);

	void setFixedMenu(QWidget* menu);
	void setGeneralSettingsMenu(QWidget* menu);

	ChannelWidget* buildNewChannel(ChannelManager* channelManager, QWidget* menu, int chId, bool deletable,
				       bool simplified, QColor color, const QString& fullName,
				       const QString& shortName);
	void buildNewInstrumentMenu(QWidget* menu, const QString& name, bool checkBoxVisible = false,
				    bool checkBoxChecked = false);

private:
	void configurePairSettings();
	void buildChannelsContainer(ChannelManager* channelManager, ChannelsPositionEnum position);
	void toggleRightMenu(CustomPushButton* btn, bool checked);
	void settingsPanelUpdate(int id);
	void rightMenuFinished(bool opened);

public Q_SLOTS:
	void triggerRightMenuToggle(bool checked);
	void configureAddMathBtn(QWidget* menu);

Q_SIGNALS:
	void changeParent(QWidget* newParent);

private:
	Ui::ToolView* m_ui;

	QButtonGroup m_group;
	QButtonGroup m_channelsGroup; // selected state of each channel

	QQueue<QPair<CustomPushButton*, bool>> m_menuButtonActions;
	QList<CustomPushButton*> m_menuOrder;
};
} // namespace gui
} // namespace scopy

#endif // TOOL_VIEW_HPP
