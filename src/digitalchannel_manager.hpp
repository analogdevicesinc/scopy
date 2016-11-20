#ifndef DIGITAL_CHANNEL_MANAGER_H
#define DIGITAL_CHANNEL_MANAGER_H


#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <QtUiTools/QUiLoader>
#include <vector>
#include <string>
#include <libserialport.h>

#include "src/pulseview/pv/devices/binarybuffer.hpp"
#include "filter.hpp"

//#include "pg_patterns.hpp"
#include "digitalchannel_manager.hpp"

// Generated UI
/*#include "ui_pattern_generator.h"*/


extern "C" {
struct iio_context;
struct iio_device;
struct iio_channel;
struct iio_buffer;
}

namespace pv {
class MainWindow;
class DeviceManager;
namespace toolbars {
class MainBar;
}
namespace view
{
class TraceTreeItem;
}
}

namespace sigrok {
class Context;
}

namespace Ui {
class PatternGenerator;
class PGChannel;
class PGChannelGroup;
class PGChannelManager;
}

namespace adiscope {


class Channel
{
private:
    uint16_t mask;
    uint16_t id;
    std::string label;
public:
    Channel(uint16_t id_, std::string label_);
    virtual ~Channel();
    uint16_t get_mask();
    virtual uint16_t get_id();
    std::string get_label();
};

class ChannelUI :  public QWidget
{
    Q_OBJECT
    Channel* ch;
public:
    ChannelUI(Channel* ch, QWidget *parent=0);
    virtual ~ChannelUI();
    Channel* get_channel();


    //pv::view::TraceTreeItem ch_trace;
};

class ChannelGroup
{

protected:
    bool selected;
    bool grouped;
    bool enabled;
    std::vector<Channel*> channels;

public:
    ChannelGroup(Channel* ch);
    virtual ~ChannelGroup();
    std::string label;
    void set_label(std::string label);
    std::string get_label();

    void add_channel(Channel* channel);
    uint16_t get_mask();
    std::vector<uint16_t> get_ids();
    size_t get_channel_count();
    std::vector<Channel*> get_channels() const;
    Channel* get_channel(int index=0);

    bool is_selected() const;
    bool is_grouped() const;
    bool is_enabled() const;
    virtual void select(bool value);
    virtual void group(bool value);
    virtual void enable(bool value);


};


class ChannelGroupUI : public QWidget
{
    Q_OBJECT
protected:
    ChannelGroup* chg;
public:
    ChannelGroupUI(ChannelGroup* chg, QWidget *parent=0);
    virtual ~ChannelGroupUI();
    ChannelGroup* get_group();


  //  std::vector<ChannelUI> channels;
  //  pv::view::TraceTreeItem chg_trace;
public Q_SLOTS:
    virtual void select(bool selected);
    virtual void enable(bool enabled);

};

class ChannelManager
{

protected:
    std::vector<Channel*> channel;
    std::vector<ChannelGroup*> channel_group;
public:
    ChannelManager();
    virtual ~ChannelManager();

    virtual void join(std::vector<int> index) = 0;
    virtual void split(int index) = 0;

    virtual void deselect_all();
    //void join_selected();

    uint16_t get_enabled_mask();
    uint16_t get_selected_mask();
    std::vector<int> get_enabled_indexes();
    std::vector<int> get_selected_indexes();
    std::vector<ChannelGroup*>* get_channel_groups();
    ChannelGroup* get_channel_group(int index);
};

}

#endif // PG_CHANNEL_MANAGER_H

