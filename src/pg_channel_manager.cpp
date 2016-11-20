#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/view.hpp"
#include "pulseview/pv/view/tracetreeitem.hpp"
#include "pg_channel_manager.hpp"
#include "pattern_generator.hpp"

#include "ui_pg_channel_group.h"
#include "ui_pg_channel_manager.h"
#include "ui_pg_channel.h"


namespace Ui {
class PGChannelGroup;
class PGChannel;
class PGChannelManager;

}

namespace pv
{
class MainWindow;
namespace view
{
class View;
class TraceTreeItem;
}
}

namespace adiscope
{


//////////////////////// CHANNEL
PatternGeneratorChannel::PatternGeneratorChannel(uint16_t id_, std::string label_) : Channel(id_,label_), channel_role("None"),trigger("rising")
{

}
std::string PatternGeneratorChannel::getTrigger() const
{
    return trigger;
}

void PatternGeneratorChannel::setTrigger(const std::string &value)
{
    trigger = value;
}

std::string PatternGeneratorChannel::getChannel_role() const
{
    return channel_role;
}

void PatternGeneratorChannel::setChannel_role(const std::string &value)
{
    channel_role = value;
}
///////////////////////// CHANNEL UI
PatternGeneratorChannelUI::PatternGeneratorChannelUI(PatternGeneratorChannel* ch, QWidget *parent) : ChannelUI(ch,parent)
{
    this->lch = ch;
}

/////////////////////// CHANNEL GROUP
std::string PatternGeneratorChannelGroup::getDecoder() const
{
    return decoder;
}

void PatternGeneratorChannelGroup::setDecoder(const std::string &value)
{
    decoder = value;
}

PatternGeneratorChannelGroup::PatternGeneratorChannelGroup(PatternGeneratorChannel* ch) : ChannelGroup(ch)
{
    created_index = 0;
    pattern=PatternFactory::create(0);
}

PatternGeneratorChannelGroup::~PatternGeneratorChannelGroup()
{
    qDebug()<<"pgchannelgroupdestroyed";
    if(pattern)
    {
            delete pattern;
            pattern = nullptr;
    }

}

void PatternGeneratorChannelGroup::append(PatternGeneratorChannelGroup* tojoin)
{
    for(auto i=0;i<tojoin->channels.size();i++)
        channels.push_back(tojoin->channels[i]);
}


////////////////////////// CHANNEL GROUP UI
PatternGeneratorChannelGroupUI::PatternGeneratorChannelGroupUI(PatternGeneratorChannelGroup* chg, QWidget *parent) : ChannelGroupUI(chg,parent)
{
this->lchg = chg;
}

PatternGeneratorChannelGroup* PatternGeneratorChannelGroupUI::getChannelGroup()
{
    return this->lchg;
}

void PatternGeneratorChannelGroupUI::set_decoder(std::string value)
{
    static_cast<PatternGeneratorChannelGroup*>(get_group())->setDecoder(value);
    qDebug()<<QString().fromStdString(lchg->getDecoder());
}

void PatternGeneratorChannelGroupUI::patternChanged(int index)
{
    delete lchg->pattern;
    lchg->created_index=index;
    lchg->pattern = PatternFactory::create(index);
}

void PatternGeneratorChannelGroupUI::createPatternUI()
{
    Q_EMIT changeRightPatternWidget(PatternFactory::create_ui(lchg->pattern,lchg->created_index));
}

void PatternGeneratorChannelGroupUI::select(bool selected)
{
    ChannelGroupUI::select(selected);
    Q_EMIT channel_selected();
}

void PatternGeneratorChannelGroupUI::enable(bool enabled)
{
    ChannelGroupUI::enable(enabled);
    Q_EMIT channel_enabled();
}



/////////////////////////// CHANNEL MANAGER
PatternGeneratorChannelManager::PatternGeneratorChannelManager() : ChannelManager()
{
    for(auto i=0;i<16;i++)
    {
        std::string temp = "DIO" + std::to_string(i);
        //LogicAnalyzerChannel ch_temp(i,temp);
        channel.push_back(new PatternGeneratorChannel(i,temp));
    }
    /*logic_channel = channel;
    qDebug()<< logic_channel->back()->getChannel_role();*/

    auto temp = static_cast<PatternGeneratorChannel*>(channel.back());

    qDebug()<< QString().fromStdString(temp->getChannel_role());
    for(auto&& ch : channel)
    {
        channel_group.push_back(new PatternGeneratorChannelGroup(static_cast<PatternGeneratorChannel*>(ch)));
    }

}

PatternGeneratorChannelManager::~PatternGeneratorChannelManager()
{
    for(auto ch : channel_group)
    {
        delete ch;
    }
    for(auto ch : channel)
    {
        delete ch;
    }
}

void PatternGeneratorChannelManager::join(std::vector<int> index)
{
    for(auto i=1;i<index.size();i++){
        auto it = std::next(channel_group.begin(), index[i]);
        static_cast<PatternGeneratorChannelGroup*>(channel_group[index[0]])->append(static_cast<PatternGeneratorChannelGroup*>(channel_group[index[i]]));
        delete *it;
        channel_group.erase(it);
        for(auto j=0;j<index.size();j++)
        {
            if(index[i] < index[j]) index[j]--;
        }
    }
    channel_group[index[0]]->group(true);
    channel_group[index[0]]->set_label("GROUP");
}

void PatternGeneratorChannelManager::split(int index)
{
    auto it = std::next(channel_group.begin(), index);
    it++;
    for(auto&& subch : channel_group[index]->get_channels())
    {
        channel_group.insert(it,new PatternGeneratorChannelGroup(static_cast<PatternGeneratorChannel*>(subch)));
        it++;
    }
    it = std::next(channel_group.begin(), index);
    delete *it;
    channel_group.erase(it);
}
////////////////////////////////////// CHANNEL MANAGER UI
PatternGeneratorChannelManagerUI::PatternGeneratorChannelManagerUI(QWidget *parent, pv::MainWindow *main_win_, PatternGeneratorChannelManager *chm, PatternGenerator* pg)  : QWidget(parent),  ui(new Ui::PGChannelManager)
{
    ui->setupUi(this);
    main_win = main_win_;
    this->chm = chm;
    this->pg = pg;

   // update_ui();
}

void PatternGeneratorChannelManagerUI::update_ui()
{
    for(auto ch : chg_ui)
    {
        delete ch;
    }
    chg_ui.erase(chg_ui.begin(),chg_ui.end());

    auto offset = 0;
    for(auto&& ch : *(chm->get_channel_groups()))
    {
        chg_ui.push_back(new PatternGeneratorChannelGroupUI(static_cast<PatternGeneratorChannelGroup*>(ch),0)); // create widget for channelgroup

        Ui::PGChannelGroup *pgchannelgroupui = new Ui::PGChannelGroup;
        pgchannelgroupui->setupUi(chg_ui.back());
        ui->verticalLayout->insertWidget(chg_ui.size(),chg_ui.back());
        pgchannelgroupui->ChannelGroupLabel->setText(QString().fromStdString(ch->get_label()));

        int i = 0;
        for(auto var : PatternFactory::get_ui_list())
        {
            pgchannelgroupui->patternCombo->addItem(var);
            pgchannelgroupui->patternCombo->setItemData(i, (PatternFactory::get_description_list())[i],Qt::ToolTipRole);
            i++;
        }

        pgchannelgroupui->enablebox->setChecked(ch->is_enabled());
        connect(pgchannelgroupui->enablebox,SIGNAL(toggled(bool)),chg_ui.back(),SLOT(enable(bool)));
        connect(static_cast<PatternGeneratorChannelGroupUI*>(chg_ui.back()),SIGNAL(changeRightPatternWidget(PatternUI*)),pg,SLOT(createRightPatternWidget(PatternUI*)));
        connect(static_cast<PatternGeneratorChannelGroupUI*>(chg_ui.back()),SIGNAL(channel_selected()),pg,SLOT(onChannelSelectedChanged())); // TEMP
        connect(static_cast<PatternGeneratorChannelGroupUI*>(chg_ui.back()),SIGNAL(channel_enabled()),pg,SLOT(onChannelEnabledChanged())); // TEMP
        connect(pgchannelgroupui->pushButton,SIGNAL(clicked()),static_cast<PatternGeneratorChannelGroupUI*>(chg_ui.back()),SLOT(createPatternUI()));
        connect(pgchannelgroupui->selectbox,SIGNAL(toggled(bool)),static_cast<PatternGeneratorChannelGroupUI*>(chg_ui.back()),SLOT(select(bool)));
        connect(pgchannelgroupui->patternCombo,SIGNAL(currentIndexChanged(int)),chg_ui.back(),SLOT(patternChanged(int)));




        offset+=(chg_ui.back()->geometry().bottomRight().y()-10);
        if(ch->is_grouped()) // create subwidgets
        {
            for(auto i=0;i<ch->get_channel_count();i++)
            {
                Ui::PGChannel *pgchannelui = new Ui::PGChannel;
                QWidget *p = new QWidget();
                pgchannelui->setupUi(p);
                pgchannelgroupui->subChannelLayout->insertWidget(i,p);

                auto str = QString().fromStdString(ch->get_channel(i)->get_label());
                pgchannelui->ChannelLabel->setText(str);
                str = QString().number(ch->get_channel(i)->get_id());
                //pgchannelui->channelIndex->setText(str);
                auto index = ch->get_channel(i)->get_id();
               // auto trace = main_win->view_->get_trace_by_id(index);

                /*p->setAttribute(Qt::WA_DontShowOnScreen);
                p->show();
                p->setAttribute(Qt::WA_DontShowOnScreen,false);*/
                //forceUpdate(p);
                offset+=p->geometry().bottomRight().y();
                /*auto offset = p->geometry().bottomRight().y();
                auto offset2 = chg_ui.back()->pos().y();*/
           //     trace->force_to_v_offset(offset);
            }            
        }
        else
        {
            auto index = ch->get_channel()->get_id();
           // auto trace = main_win->view_->get_trace_by_id(index);
        //    trace->force_to_v_offset(offset);
        }
    }
}


void PatternGeneratorChannelManagerUI::on_groupSplit_clicked()
{
    std::vector<int> selection = chm->get_selected_indexes();
    if(selection.size() == 0) {update_ui();return;}
    if(selection.size() == 1)
    {
        if(chm->get_channel_group(selection.back())->is_grouped())
        {
            chm->split(selection.back());
        }
        else
        {
            chm->join(selection);
        }
        chm->deselect_all();
    }
    else
    {
        chm->join(selection);
        chm->deselect_all();
    }
    update_ui();
}
}



