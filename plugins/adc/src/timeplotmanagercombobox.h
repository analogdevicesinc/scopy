#ifndef TIMEPLOTMANAGERCOMBOBOX_H
#define TIMEPLOTMANAGERCOMBOBOX_H

#include <channelcomponent.h>
#include <QWidget>
#include <timeplotmanager.h>

namespace scopy {
namespace adc {

class TimePlotManagerCombobox : public QWidget
{
	Q_OBJECT
public:
	TimePlotManagerCombobox(TimePlotManager *man, ChannelComponent *c, QWidget *parent = nullptr);
	~TimePlotManagerCombobox();

public Q_SLOTS:
	void addPlot(TimePlotComponent *p);
	void removePlot(TimePlotComponent *p);
	void renamePlot(TimePlotComponent *p);

private Q_SLOTS:
	void renamePlotSlot();

private:
	TimePlotManager *m_man;
	ChannelComponent *m_ch;
	MenuCombo *m_mcombo;
	QComboBox *m_combo;

	int findIndexFromUuid(uint32_t uuid);
};
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTMANAGERCOMBOBOX_H
