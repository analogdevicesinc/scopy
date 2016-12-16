#ifndef PG_CHANNEL_MANAGER_H
#define PG_CHANNEL_MANAGER_H

#include <QDebug>
#include <QGroupBox>
#include "pg_patterns.hpp"
//#include "pattern_generator.hpp"
#include "digitalchannel_manager.hpp"
#include "ui_pg_channel_group.h"
#include "ui_pg_channel_manager.h"
#include "ui_pg_channel_header.h"
#include "ui_pg_channel.h"


namespace Ui {
class PGChannelGroup;
class PGChannel;
class PGChannelManager;
class PGChannelManagerHeader;
}


namespace adiscope {
class PatternGenerator;
class PatternGeneratorChannelGroup;
class PatternGeneratorChannelGroupUI;
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
    PatternGeneratorChannelGroupUI *chgui;
    PatternGeneratorChannelGroup *chg;
    PatternGeneratorChannel *ch;
public:
    Ui::PGChannelGroup *ui;
    PatternGeneratorChannelUI(PatternGeneratorChannel* ch, PatternGeneratorChannelGroup* chg, PatternGeneratorChannelGroupUI *chgui, PatternGeneratorChannelManagerUI* managerUi, QWidget *parent = 0);
    ~PatternGeneratorChannelUI();
    PatternGeneratorChannelManagerUI *getManagerUi() const;
    PatternGeneratorChannel* getChannel();
    PatternGeneratorChannelGroup* getChannelGroup();
    void enableControls(bool val);
private Q_SLOTS:
    void split();
    void mousePressEvent(QMouseEvent*) override;

//    void set_decoder(std::string value);
};

class PatternGeneratorChannelGroup : public ChannelGroup
{

    bool collapsed;    
public:
    PatternGeneratorChannelGroup(PatternGeneratorChannel* ch, bool en);
    ~PatternGeneratorChannelGroup();
    int created_index;
    Pattern *pattern;
    bool isCollapsed();
    void collapse(bool val);
    void append(PatternGeneratorChannelGroup* tojoin);
};

class PatternGeneratorChannelGroupUI : public ChannelGroupUI
{
    Q_OBJECT
    Q_PROPERTY(int checked READ isChecked WRITE check)
    int checked;
    PatternGeneratorChannelManagerUI *managerUi;
    int isChecked();
    void check(int val);

public:
    Ui::PGChannelGroup *ui;
    std::vector<PatternGeneratorChannelUI*> ch_ui;
    PatternGeneratorChannelGroupUI(PatternGeneratorChannelGroup* chg, PatternGeneratorChannelManagerUI* managerUi, QWidget *parent = 0);
    ~PatternGeneratorChannelGroupUI();
    PatternGeneratorChannelGroup* getChannelGroup();
    PatternGeneratorChannelManagerUI *getManagerUi() const;


    void enableControls(bool enabled);

Q_SIGNALS:
    void channel_selected();
    void channel_enabled();
    void channelGroupSelected(PatternGeneratorChannelGroupUI* select);
private Q_SLOTS:
    void patternChanged(int index);    
    void select(bool selected);
    void enable(bool enabled);
    void split();
    void collapse();
    void mousePressEvent(QMouseEvent*) override;
};


class PatternGeneratorChannelManager : public ChannelManager
{
    PatternGeneratorChannelGroup *highlightedChannelGroup;
    PatternGeneratorChannel *highlightedChannel;
public:

    void highlightChannel(PatternGeneratorChannelGroup *chg, PatternGeneratorChannel *ch = nullptr);
    PatternGeneratorChannelGroup* getHighlightedChannelGroup();
    PatternGeneratorChannel* getHighlightedChannel();
    PatternGeneratorChannelManager();
    ~PatternGeneratorChannelManager();    
    PatternGeneratorChannelGroup* get_channel_group(int index);

    void join(std::vector<int> index);
    void split(int index);
    void splitChannel(int chgIndex, int chIndex);
    void preGenerate();
    void generatePatterns(short *mainbuffer, uint32_t sampleRate, uint32_t bufferSize);
    void commitBuffer(PatternGeneratorChannelGroup *chg, short *mainBuffer, uint32_t bufferSize);
    short remap_buffer(uint8_t *mapping, uint32_t val);

    uint32_t computeSuggestedSampleRate();
    uint32_t computeSuggestedBufferSize(uint32_t sample_rate);

};


class PatternGeneratorChannelManagerUI : public QWidget
{
    Q_OBJECT
    QWidget* settingsWidget; // pointer to settingspage in stacked widget in main pg ui
    QWidget* channelManagerHeaderWiget;

    PatternUI* currentUI; // pointer to currently drawn patternUI.
    bool disabledShown;
    bool detailsShown;
    bool highlightShown;

public:

    std::vector<PatternGeneratorChannelGroupUI*> chg_ui;

    PatternGeneratorChannelManagerUI(QWidget *parent, PatternGeneratorChannelManager* chm, QWidget *settingsWidget,PatternGenerator* pg);
    ~PatternGeneratorChannelManagerUI();

    PatternGeneratorChannelGroupUI *findUiByChannelGroup(PatternGeneratorChannelGroup* toFind);
    PatternGeneratorChannelUI* findUiByChannel(PatternGeneratorChannel* toFind);
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

    void showHighlight(bool val);

    void groupSplitSelected();
    PatternGeneratorChannelGroup *getSelectedChannelGroup() const;
    void setSelectedChannelGroup(PatternGeneratorChannelGroup *value);

Q_SIGNALS:
    void channelsChanged();


private Q_SLOTS:

private:
    void retainWidgetSizeWhenHidden(QWidget *w);
    void setWidgetNrOfChars(QWidget *w, int minNrOfChars, int maxNrOfChars=0);
};


}
#endif // PG_CHANNEL_MANAGER_H
