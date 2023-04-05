#ifndef SWIOTCONFIGCONTROLLER_HPP
#define SWIOTCONFIGCONTROLLER_HPP

#include "qobject.h"
#include "src/config/configchannelview.hpp"
#include "configmodel.hpp"

namespace adiscope::swiot {

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


#endif // SWIOTCONFIGCONTROLLER_HPP
