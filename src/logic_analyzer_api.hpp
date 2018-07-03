#ifndef LOGIC_ANALYZER_API_HPP
#define LOGIC_ANALYZER_API_HPP

#include "logic_analyzer.hpp"

namespace adiscope {

class LogicAnalyzer_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(int channel_groups_list_size READ channel_groups_list_size WRITE
		setChannelGroupsListSize SCRIPTABLE false)
	Q_PROPERTY(QVariantList channel_groups READ getChannelGroups)
	Q_PROPERTY(bool running READ running WRITE run STORED false)
	Q_PROPERTY(bool single READ single WRITE runSingle STORED false)
	Q_PROPERTY(double time_position READ getTimePos WRITE setTimePos)
	Q_PROPERTY(double time_base READ getTimeBase WRITE setTimeBase)
	Q_PROPERTY(QString run_mode READ runMode WRITE setRunMode)
	Q_PROPERTY(bool external_trigger READ externalTrigger WRITE setExternalTrigger)
	Q_PROPERTY(bool cursors_active READ cursorsActive WRITE setCursorsActive)
	Q_PROPERTY(bool cursors_locked READ cursorsLocked WRITE setCursorsLocked)
	Q_PROPERTY(bool inactive_hidden READ inactiveHidden WRITE setInactiveHidden)
	Q_PROPERTY(bool export_all READ getExportAll WRITE setExportAll)
	Q_PROPERTY(QList<int> data READ data STORED false)

public:
	explicit LogicAnalyzer_API(LogicAnalyzer *lga) :
		ApiObject(), lga(lga) {}
	~LogicAnalyzer_API() {}

	int channel_groups_list_size() const;
	void setChannelGroupsListSize(int size);

	QVariantList getChannelGroups();

	bool running() const;
	void run(bool en);

        bool single() const;
        void runSingle(bool en);

	double getTimePos() const;
	void setTimePos(double pos);

	double getTimeBase() const;
	void setTimeBase(double base);

	QString runMode() const;
	void setRunMode(QString);

	bool externalTrigger() const;
	void setExternalTrigger(bool val);

	bool cursorsActive() const;
	void setCursorsActive(bool en);

	bool cursorsLocked() const;
	void setCursorsLocked(bool en);

	bool inactiveHidden() const;
	void setInactiveHidden(bool en);

	bool getExportAll() const;
	void setExportAll(bool);

	Q_INVOKABLE void show();

	QList<int> data() const;

private:
	LogicAnalyzer *lga;
};

class ChannelGroup_API : public ApiObject
{
	friend class LogicChannel_API;
	Q_OBJECT
	Q_PROPERTY(QString decoder READ getDecoder WRITE setDecoder)
	Q_PROPERTY(int channels_list_size READ channels_list_size WRITE
		   setChannelsListSize SCRIPTABLE false)
	Q_PROPERTY(QVariantList channels READ getChannels)
	Q_PROPERTY(bool enabled READ chEnabled WRITE setChEnabled)
	Q_PROPERTY(bool grouped READ chGrouped WRITE setChGrouped)
	Q_PROPERTY(QString name READ getName WRITE setName)
	Q_PROPERTY(bool collapsed READ getChCollapsed WRITE setChCollapsed)

public:
	explicit ChannelGroup_API(LogicAnalyzer *lga, int index = -1,
				bool load = true) :
		ApiObject(), lga(lga), index(index)
		{
			if(!load)
				set_channels_api();
		}
	~ChannelGroup_API() {
		qDeleteAll(channels_api);
		channels_api.clear();
	}
	bool chEnabled() const;
	void setChEnabled(bool en);

	bool chGrouped() const;
	void setChGrouped(bool val);

	QString getName() const;
	void setName(QString val);

	bool getChCollapsed() const;
	void setChCollapsed(bool val);

	int channels_list_size() const;
	void setChannelsListSize(int size);

	QVariantList getChannels();

	int getIndex() const;
	void set_channels_api();

	QString getDecoder() const;
	void setDecoder(QString val);
private:
	LogicAnalyzer *lga;
	QList<LogicChannel_API *> channels_api;
	int index;
};

class LogicChannel_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(int index READ getIndex WRITE setIndex)
	Q_PROPERTY(QString trigger READ getTrigger WRITE setTrigger)
	Q_PROPERTY(QString name READ getName WRITE setName)
	Q_PROPERTY(QString role READ getRole WRITE setRole)

public:
	explicit LogicChannel_API(LogicAnalyzer *lga,
			ChannelGroup_API *chg, int index=-1) :
		ApiObject(), lga(lga), lchg(chg), index(index)
		{
		}
	~LogicChannel_API() { }

	QString getTrigger() const;
	void setTrigger(QString val);

	QString getName() const;
	void setName(QString val);

	int getIndex() const;
	void setIndex(int);

	QString getRole() const;
	void setRole(QString val);

	QList<bool> data() const;

private:
	LogicAnalyzer *lga;
	ChannelGroup_API *lchg;
	int index;
};
}
#endif // LOGIC_ANALYZER_API_HPP
