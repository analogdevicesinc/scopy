#ifndef TOOL_VIEW_HPP
#define TOOL_VIEW_HPP

#include <QButtonGroup>
#include <QMap>
#include <QQueue>
#include <QStackedWidget>
#include <QWidget>

#include <scopy/gui/channel_widget.hpp>
#include <scopy/gui/custom_menu_button.hpp>
#include <scopy/gui/custom_push_button.hpp>
#include <scopy/gui/linked_button.hpp>

namespace Ui {
class ToolView;
}

namespace scopy {
namespace gui {

enum class MenusEnum
{
	CURSORS,
	MEASURE,
	TRIGGER,
	SWEEP,
	MARKERS,
	CHANNEL,
	ADD_CHANNEL,
	CHANNELS_SETTINGS

};

class ToolView : public QWidget
{
	Q_OBJECT

public:
	explicit ToolView(QWidget* parent = nullptr);
	~ToolView();

	void setBtns(QMap<MenusEnum, CustomMenuButton*> btns);
	void setUpperMenus();

	QWidget* getMenu(MenusEnum menu);
	QWidget* getGeneralSettingsMenu();
	QWidget* getButtonMenu(MenusEnum btn);

	LinkedButton* getHelpBtn();
	void setHelpBtnVisible(bool visible);
	void setUrlHelpBtn(const QString& url);

	QPushButton* getPrintBtn();
	void setPrintBtnVisible(bool visible);

	QPushButton* getRunBtn();
	void setRunBtnVisible(bool visible);

	QPushButton* getSingleBtn();
	void setSingleBtnVisible(bool visible);

	CustomPushButton* getGeneralSettingsBtn();
	QPushButton* getSettingsBtn();
	void setPairSettingsVisible(bool visible);

	QWidget* getExtraWidget();
	void setExtraWidget(QWidget* widget);

	QWidget* getCentralWidget();

	void setInstrumentNotesVisible(bool visible);

	CustomPushButton* getAddMathBtn();
	void setAddMathBtnVisible(bool visible);
	void configureAddMathBtn();

	QWidget* getChannels();
	void buildDefaultChannels();
	ChannelWidget* buildNewChannel(int chId, bool deletable, bool simplified, QColor color, const QString& fullName,
				       const QString& shortName);

	QStackedWidget* getStackedWidget();

	void setFixedMenu(QWidget* menu);

private:
	void toggleRightMenu(CustomPushButton* btn, bool checked);
	void settingsPanelUpdate(int id);
	void rightMenuFinished(bool opened);

public Q_SLOTS:
	void triggerRightMenuToggle(bool checked);

private:
	Ui::ToolView* m_ui;

	QButtonGroup m_group;
	QButtonGroup m_channelsGroup; // selected state of each channel

	QWidget* m_generalSettingsMenu;

	QMap<MenusEnum, CustomMenuButton*> m_btns;
	QMap<MenusEnum, QWidget*> m_menus;

	QQueue<QPair<CustomPushButton*, bool>> m_menuButtonActions;
	QList<CustomPushButton*> m_menuOrder;
};
} // namespace gui
} // namespace scopy

#endif // TOOL_VIEW_HPP
