#ifndef PLOTMANAGERCOMBOBOX_H
#define PLOTMANAGERCOMBOBOX_H

#include <channelcomponent.h>
#include <QWidget>
#include <timeplotmanager.h>

namespace scopy {
namespace adc {

class PlotManagerCombobox : public QWidget
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
} // namespace adc
} // namespace scopy
#endif // PLOTMANAGERCOMBOBOX_H
