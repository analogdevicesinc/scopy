#ifndef PLOTMANAGERCOMBOBOX_H
#define PLOTMANAGERCOMBOBOX_H

#include <scopy-gui_export.h>
#include <channelcomponent.h>
#include <QWidget>
#include <menucombo.h>

namespace scopy {

class PlotManager;

class SCOPY_GUI_EXPORT PlotManagerCombobox : public QWidget
{
	Q_OBJECT
public:
	PlotManagerCombobox(PlotManager *man, ChannelComponent *c, QWidget *parent = nullptr);
	~PlotManagerCombobox();

public Q_SLOTS:
	void addPlot(PlotComponent *p);
	void removePlot(PlotComponent *p);
	void renamePlot(PlotComponent *p);

private Q_SLOTS:
	void renamePlotSlot();

private:
	PlotManager *m_man;
	ChannelComponent *m_ch;
	MenuCombo *m_mcombo;
	QComboBox *m_combo;

	int findIndexFromUuid(uint32_t uuid);
};
} // namespace scopy
#endif // PLOTMANAGERCOMBOBOX_H
