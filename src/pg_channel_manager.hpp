#ifndef PG_CHANNEL_MANAGER_H
#define PG_CHANNEL_MANAGER_H

#include <QDebug>
#include "pg_patterns.hpp"
//#include "pattern_generator.hpp"
#include "digitalchannel_manager.hpp"


namespace adiscope {
class PatternGenerator;

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
    PatternGeneratorChannel *lch;
public:
    PatternGeneratorChannelUI(PatternGeneratorChannel* ch, QWidget *parent = 0);
private Q_SLOTS:
//    void set_decoder(std::string value);
};

class PatternGeneratorChannelGroup : public ChannelGroup
{
    std::string decoder;
public:
    PatternGeneratorChannelGroup(PatternGeneratorChannel* ch);
    ~PatternGeneratorChannelGroup();
    int created_index;
    Pattern *pattern;
    void append(PatternGeneratorChannelGroup* tojoin);
    std::string getDecoder() const;
    void setDecoder(const std::string &value);
};

class PatternGeneratorChannelGroupUI : public ChannelGroupUI
{
    Q_OBJECT
    PatternGeneratorChannelGroup *lchg;    
public:
    PatternGeneratorChannelGroupUI(PatternGeneratorChannelGroup* chg, QWidget *parent = 0);
    PatternGeneratorChannelGroup* getChannelGroup();
Q_SIGNALS:
    void channel_selected();
    void channel_enabled();
    void changeRightPatternWidget(PatternUI* rightwidget);    
private Q_SLOTS:
    void set_decoder(std::string value);
    void patternChanged(int index);
    void createPatternUI();
    void select(bool selected);
    void enable(bool enabled);

};


class PatternGeneratorChannelManager : public ChannelManager
{
public:
    PatternGeneratorChannelManager();
    ~PatternGeneratorChannelManager();
    void join(std::vector<int> index);
    void split(int index);
};


class PatternGeneratorChannelManagerUI : public QWidget
{
    Q_OBJECT
public:

    pv::MainWindow* main_win;
    std::vector<PatternGeneratorChannelGroupUI*> chg_ui;
    PatternGeneratorChannelManagerUI(QWidget *parent, pv::MainWindow* main_win_, PatternGeneratorChannelManager* chm, PatternGenerator* pg);
    PatternGeneratorChannelManager* chm;
    PatternGenerator *pg;
    Ui::PGChannelManager *ui;
    void update_ui();

private Q_SLOTS:

   /*void select_channel(bool checked);
    void on_pushButton_2_clicked();*/
    void on_groupSplit_clicked();

};


}
#endif // PG_CHANNEL_MANAGER_H
