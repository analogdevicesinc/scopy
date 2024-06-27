#ifndef DACINSTRUMENT_H
#define DACINSTRUMENT_H
#include "scopy-dac_export.h"

#include <QWidget>
#include <QPushButton>
#include <QList>

#include "verticalchannelmanager.h"
#include <gui/tooltemplate.h>
#include <gui/widgets/toolbuttons.h>
#include <tooltemplate.h>
#include <iioutil/connectionprovider.h>

namespace scopy {
class DacDataManager;
class MenuControlButton;
class CollapsableMenuControlButton;
class SCOPY_DAC_EXPORT DacInstrument : public QWidget
{
	Q_OBJECT
public:
	DacInstrument(const Connection *conn, QWidget *parent = nullptr);
	virtual ~DacInstrument();
private:
	void setupDacDataManagers();
	MenuControlButton *addDevice(DacDataManager *dev, QWidget *parent);


	const Connection *m_conn;
	ToolTemplate *tool;
	InfoBtn *infoBtn;
	PrintBtn *printBtn;
	QPushButton *openLastMenuBtn;
	QButtonGroup *devicesGroup;
	QButtonGroup *rightMenuBtnGrp;
	MenuControlButton *devicesBtn;
	MapStackedWidget *deviceStack;
	MapStackedWidget *dacManagerStack;
	QList<DacDataManager*> m_dacDataManagers;
	VerticalChannelManager *vcm;
	int uuid = 0;

	const QString devicesMenuId = "devices";
	const QString verticalChannelManagerId = "vcm";
};
} // namespace scopy



#endif //DACINSTRUMENT_H_
