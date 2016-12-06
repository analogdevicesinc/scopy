#ifndef PG_CHANNEL_MANAGER_H
#define PG_CHANNEL_MANAGER_H

#include <QDebug>
#include <QGroupBox>
#include "pg_patterns.hpp"
//#include "pattern_generator.hpp"
#include "digitalchannel_manager.hpp"


namespace adiscope {
class PatternGenerator;
class PatternGeneratorChannelGroup;
class PatternGeneratorChannelManagerUI;

class PatternGeneratorChannel : public Channel
{
    std::string channel_role;
    std::string trigger;
public:
    PatternGeneratorChannel(uint16_t id_, std::string label_);

    std::string getChannel_role() const;
    void setChannel_role(const std::string &value);
    std::string getTrigger() const;
    void setTrigger(const std::string &value);
};

class PatternGeneratorChannelUI : public ChannelUI
{
    Q_OBJECT
    PatternGeneratorChannelManagerUI *managerUi;
    PatternGeneratorChannelGroup *chg;
    PatternGeneratorChannel *ch;
public:
    PatternGeneratorChannelUI(PatternGeneratorChannel* ch, PatternGeneratorChannelGroup* chg, PatternGeneratorChannelManagerUI* managerUi, QWidget *parent = 0);
    ~PatternGeneratorChannelUI();
private Q_SLOTS:
    void split();
//    void set_decoder(std::string value);
};

class PatternGeneratorChannelGroup : public ChannelGroup
{
    std::string decoder;
    bool collapsed;    
public:
    PatternGeneratorChannelGroup(PatternGeneratorChannel* ch);
    ~PatternGeneratorChannelGroup();
    int created_index;
    Pattern *pattern;
    bool isCollapsed();
    void collapse(bool val);
    void append(PatternGeneratorChannelGroup* tojoin);
    std::string getDecoder() const;
    void setDecoder(const std::string &value);
};

class PatternGeneratorChannelGroupUI : public ChannelGroupUI
{
    Q_OBJECT
    PatternGeneratorChannelManagerUI *managerUi;    
public:
    std::vector<PatternGeneratorChannelUI*> ch_ui;
    PatternGeneratorChannelGroupUI(PatternGeneratorChannelGroup* chg, PatternGeneratorChannelManagerUI* managerUi, QWidget *parent = 0);
    PatternGeneratorChannelGroup* getChannelGroup();
    PatternGeneratorChannelManagerUI *getManagerUi() const;
    void enableControls(bool enabled);

Q_SIGNALS:
    void channel_selected();
    void channel_enabled();
    void channelGroupSelected(PatternGeneratorChannelGroupUI* select);
private Q_SLOTS:
    void set_decoder(std::string value);
    void patternChanged(int index);    
    void select(bool selected);
    void enable(bool enabled);
    void split();
    void collapse();
    void settingsButtonHandler();

};


class PatternGeneratorChannelManager : public ChannelManager
{
public:
    PatternGeneratorChannelManager();
    ~PatternGeneratorChannelManager();
    PatternGeneratorChannelGroup* get_channel_group(int index);
    std::vector<PatternGeneratorChannelGroup*>* getChannelGroups();
    void join(std::vector<int> index);
    void split(int index);
    void splitChannel(int chgIndex, int chIndex);
};


class PatternGeneratorChannelManagerUI : public QWidget
{
    Q_OBJECT
    QWidget* settingsWidget; // pointer to settingspage in stacked widget in main pg ui
    PatternGeneratorChannelGroupUI *selectedChannelGroupUi;
    PatternGeneratorChannelGroup *selectedChannelGroup;
    PatternUI* currentUI; // pointer to currently drawn patternUI.
    QButtonGroup *channelButtonGroup;
    bool disabledShown;
    bool detailsShown;
public:

    pv::MainWindow* main_win;
    std::vector<PatternGeneratorChannelGroupUI*> chg_ui;
    PatternGeneratorChannelManagerUI(QWidget *parent, pv::MainWindow* main_win_, PatternGeneratorChannelManager* chm, QWidget *settingsWidget,PatternGenerator* pg);
    ~PatternGeneratorChannelManagerUI();


    PatternGeneratorChannelManager* chm;
    PatternGenerator *pg;
    Ui::PGChannelManager *ui;

    void updateUi();
    void selectChannelGroup(PatternGeneratorChannelGroupUI* selected);
    void deleteSettingsWidget();
    void createSettingsWidget();
    QWidget *getSettingsWidget() const;

    bool isDisabledShown();
    void showDisabled();
    void hideDisabled();

    bool areDetailsShown();
    void showDetails();
    void hideDetails();

    void groupSplitSelected();

    PatternGeneratorChannelGroup *getSelectedChannelGroup() const;
    void setSelectedChannelGroup(PatternGeneratorChannelGroup *value);

private Q_SLOTS:

private:
    void retainWidgetSizeWhenHidden(QWidget *w);
    void setWidgetMinimumNrOfChars(QWidget *w, int nrOfChars, QFont* font=nullptr);
};


}
#endif // PG_CHANNEL_MANAGER_H
