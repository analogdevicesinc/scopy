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
namespace view {
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
	QColor edgecolor;
	QColor highcolor;
	QColor lowcolor;
	QColor bgcolor;
public:
	Channel(uint16_t id_, std::string label_);
	virtual ~Channel();
	uint16_t get_mask();
	virtual uint16_t get_id();
	virtual void set_id(int val);
	std::string get_label();
	void set_label(std::string);
	QColor getBgcolor() const;
	void setBgcolor(const QColor& value);
	QColor getEdgecolor() const;
	void setEdgecolor(const QColor& value);
	QColor getHighcolor() const;
	void setHighcolor(const QColor& value);
	QColor getLowcolor() const;
	void setLowcolor(const QColor& value);
};

class ChannelUI :  public QWidget
{
	Q_OBJECT
	Channel *ch;
public:
	ChannelUI(Channel *ch, QWidget *parent=0);
	virtual ~ChannelUI();
	Channel *get_channel();


	//pv::view::TraceTreeItem ch_trace;
};

class ChannelGroup
{

protected:
	bool selected;
	bool grouped;
	bool enabled;
	std::vector<Channel *> channels;
	QColor edgecolor;
	QColor highcolor;
	QColor lowcolor;
	QColor bgcolor;

public:
	ChannelGroup(Channel *ch);
	ChannelGroup();
	virtual ~ChannelGroup();
	std::string label;
	void set_label(std::string label);
	std::string get_label();

	void add_channel(Channel *channel);
	void remove_channel(int channelIndex);
	uint16_t get_mask();
	std::vector<uint16_t> get_ids();
	size_t get_channel_count();
	std::vector<Channel *> *get_channels();
	Channel *get_channel(int index=0);

	bool is_selected() const;
	bool is_grouped() const;
	bool is_enabled() const;
	virtual void select(bool value);
	virtual void group(bool value);
	virtual void enable(bool value);
	QColor getBgcolor() const;
	void setBgcolor(const QColor& value);
	QColor getHighcolor() const;
	void setHighcolor(const QColor& value);
	QColor getLowcolor() const;
	void setLowcolor(const QColor& value);
	QColor getEdgecolor() const;
	void setEdgecolor(const QColor& value);
};


class ChannelGroupUI : public QWidget
{
	Q_OBJECT
protected:
	ChannelGroup *chg;
public:
	ChannelGroupUI(ChannelGroup *chg, QWidget *parent=0);
	virtual ~ChannelGroupUI();
	ChannelGroup *get_group();


	//  std::vector<ChannelUI> channels;
	//  pv::view::TraceTreeItem chg_trace;
public Q_SLOTS:
	virtual void select(bool selected);
	virtual void enable(bool enabled);

};

class ChannelManager
{
protected:
	std::vector<Channel *> channel;
	std::vector<ChannelGroup *> channel_group;
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
	std::vector<ChannelGroup *> *get_channel_groups();
	size_t get_channel_count();
	size_t get_channel_group_count();
	ChannelGroup *get_channel_group(int index);
};

class DIOManager : public QObject
{
	Q_OBJECT
	int direction;
	int lockMask;
	int nrOfChannels;
	int gpo;
	int gpi;
	bool outputEnabled;
	static const char *channelNames[];
	iio_context *ctx;
	iio_device *dev;

public:
	void init();
	DIOManager(iio_context *ctx, Filter *filt);
	~DIOManager();
	bool getOutputEnabled();
	void enableOutput(bool output);
	int getGpo();
	void setOutRaw(int ch, bool val);
	bool getOutRaw(int ch);
	int getGpi();
	bool getInRaw(int ch);
	void setDeviceOutRaw(int ch);

	void setOutputMode(int ch, bool mode);
	void setMode(int mask);
	void setDirection(int ch, bool output);
	bool getDirection(int ch);
	void setDeviceDirection(int ch, bool force);
	iio_channel *getChannel(int ch);
	void lock(int mask);
	int getLockMask();
	bool isLocked(int ch);
	void unlock();

Q_SIGNALS:
	void locked();
	void unlocked();
};

}

#endif // PG_CHANNEL_MANAGER_H

