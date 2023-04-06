#ifndef SWIOTCONFIGCONTROLLER_HPP
#define SWIOTCONFIGCONTROLLER_HPP

#include "qobject.h"
#include "swiotconfigchnlview.hpp"
#include "swiotconfigmodel.hpp"

namespace adiscope{

class SwiotConfigController: public QObject
{
	Q_OBJECT
public:
	explicit SwiotConfigController(gui::SwiotConfigChnlView* chnlsView = nullptr,
				       SwiotConfigModel* model = nullptr);
	~SwiotConfigController();

	void addMenuAttrValues();
	void createConnections();

	QStringList getActiveFunctions();
public Q_SLOTS:
	void attrChanged(QString attrName, int deviceIdx);
private:
	gui::SwiotConfigChnlView* m_chnlsView;
	SwiotConfigModel* m_model;

};

}


#endif // SWIOTCONFIGCONTROLLER_HPP
