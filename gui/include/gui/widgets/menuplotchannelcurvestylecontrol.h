#ifndef MENUPLOTCHANNELCURVESTYLECONTROL_H
#define MENUPLOTCHANNELCURVESTYLECONTROL_H

#include <QWidget>
#include "scopy-gui_export.h"
#include <menucombo.h>

namespace scopy {
class PlotChannel;
namespace gui {

class SCOPY_GUI_EXPORT MenuPlotChannelCurveStyleControl : public QWidget
{
	Q_OBJECT
public:
	MenuPlotChannelCurveStyleControl(QWidget *parent);
	~MenuPlotChannelCurveStyleControl();

public Q_SLOTS:
	void addChannels(PlotChannel *c);
	void removeChannels(PlotChannel *c);

private:
	void createCurveMenu(QWidget *parent);
	QList<PlotChannel *> m_channels;
	MenuCombo *cbThicknessW;
	MenuCombo *cbStyleW;
};
} // namespace gui
} // namespace scopy

#endif // MENUPLOTCHANNELCURVESTYLECONTROL_H
