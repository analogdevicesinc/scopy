#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

#include "qobject.h"
#include "src/config/configchannelview.h"
#include "configmodel.h"

namespace scopy::swiot {

class ConfigController: public QObject
{
	Q_OBJECT
public:
	explicit ConfigController(ConfigChannelView* chnlsView = nullptr,
				  ConfigModel* model = nullptr);
	~ConfigController();

	void addMenuAttrValues();
	void createConnections();

	QStringList getActiveFunctions();
public Q_SLOTS:
	void attrChanged(QString attrName, int deviceIdx);
private:
	ConfigChannelView* m_chnlsView;
	ConfigModel* m_model;

};

}


#endif // CONFIGCONTROLLER_H
