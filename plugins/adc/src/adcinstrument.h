#ifndef ADCINSTRUMENT_H
#define ADCINSTRUMENT_H

#include <QPushButton>
#include <QWidget>

#include <adcplugin.h>
#include <cursorcontroller.h>
#include <gui/widgets/toolbuttons.h>

#include "gui/tooltemplate.h"
#include "measurementsettings.h"
#include "verticalchannelmanager.h"
#include "plotrecipe.h"

namespace scopy {
class MenuControlButton;
class CollapsableMenuControlButton;

/*
class AcqTreeNode;

class AcqTree : public QObject {
	Q_OBJECT
public:
	AcqTree(QObject *parent = nullptr) : QObject(parent) {};
	~AcqTree();
	QList<AcqTreeNode*> m_nodes;

	void reset();

};

class AcqTreeNode : public QObject {
	Q_OBJECT
public:
	AcqTreeNode(QString name, QObject *parent = nullptr) : QObject(parent) { m_name = name; }
	~AcqTreeNode() { }

	void addDep(AcqTreeNode* t) { m_deps.append(t); }
	bool removeDep(AcqTreeNode* t) { return m_deps.removeAll(t); }
	AcqTree* tree() { return m_tree;}

	virtual void enable() {
		for(AcqTreeNode *n : m_deps) {
			n->enable();
		}
	};

	virtual void disable() {

	};

private:
	AcqTree *m_tree;
	QString m_name;
	QList<AcqTreeNode*> m_deps;

};

class IIODeviceNode : AcqTreeNode {
public:
	IIODeviceNode(struct iio_device *dev, QString name, QObject *parent = nullptr) : AcqTreeNode(name,parent), m_dev(dev) {
		m_devId = iio_device_get_id(m_dev);
	}
	~IIODeviceNode() {}

	virtual void enable() override {};
	virtual void disable() override {};

private:
	size_t m_buffersize;
	QString m_devId;
	struct iio_device *m_dev;
	struct iio_buffer *m_buf;
};

class IIOChannelNode: AcqTreeNode {
public:
	IIOChannelNode(struct iio_channel *ch, QString name, QObject *parent = nullptr) : AcqTreeNode(name,parent), m_ch(ch) {
		m_chId= iio_channel_get_id(m_ch);
	}
	~IIOChannelNode() {}

	virtual void enable() override { iio_channel_enable(m_ch);};
	virtual void disable() override { iio_channel_disable(m_ch);};

private:
	size_t m_buffersize;
	QString m_chId;
	struct iio_channel *m_ch;
};




class AdcInstrumentRecipe {

public:
	AdcInstrumentRecipe();
	~AdcInstrumentRecipe();
private:
	AcqTree *man;


};*/

class AdcInstrument : public QWidget
{
	Q_OBJECT
public:
	AdcInstrument(PlotRecipe *r, QWidget *parent = nullptr);
	~AdcInstrument();
	void init();
	void deinit();

	bool running() const;
	void setRunning(bool newRunning);
public Q_SLOTS:
	void run(bool);
	void stop();
	void start();
	void restart();
	void showMeasurements(bool b);

Q_SIGNALS:
	void runningChanged(bool);
	void newPlot(int);
	void removePlot(QWidget*);

private:
	bool m_running;
	PlotRecipe *m_recipe;

	RunBtn *runBtn;
	SingleShotBtn *singleBtn;
	ToolTemplate *tool;
	QPushButton *openLastMenuBtn;

	MenuControlButton *channelsBtn;

	MeasurementsPanel *measure_panel;
	MeasurementSettings *measureSettings;
	StatsPanel *stats_panel;

	VerticalChannelManager *vcm;

	MapStackedWidget *channelStack;
	QButtonGroup *rightMenuBtnGrp;
	QButtonGroup *channelGroup;

	CursorController *cursorController;

	void setupTimeButtonHelper(MenuControlButton *time);
	void setupCursorButtonHelper(MenuControlButton *cursor);
	void setupMeasureButtonHelper(MenuControlButton *measure);
	void setupChannelsButtonHelper(MenuControlButton *channelsBtn);
	void setupDeviceMenuControlButtonHelper(MenuControlButton *devBtn, GRDeviceAddon *dev);
	void setupChannelMenuControlButtonHelper(MenuControlButton *btn, ChannelAddon *ch);
	void initCursors();

	void setupChannelDelete(ChannelAddon *ch);

	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

	int uuid = 0;
	const QString channelsMenuId = "channels";
	const QString measureMenuId = "measure";
	const QString statsMenuId = "stats";
	const QString verticalChannelManagerId = "vcm";
	void setupChannelMeasurement(ChannelAddon *ch);
	void setupChannelSnapshot(ChannelAddon *ch);
};
} // namespace scopy
#endif // ADCINSTRUMENT_H
