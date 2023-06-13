#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

#include "qobject.h"
#include "src/config/configchannelview.h"
#include "configmodel.h"

namespace scopy::swiot {

class ConfigController : public QObject {
Q_OBJECT
public:
	explicit ConfigController(ConfigChannelView *channelView,
				  ConfigModel *model,
				  int channelId);

	~ConfigController();

	void connectSignalsAndSlots();
	void initChannelView();

Q_SIGNALS:
	void clearDrawArea();

private:
	ConfigChannelView *m_channelsView;
	ConfigModel *m_model;
	int m_channelId;
};

}


#endif // CONFIGCONTROLLER_H
