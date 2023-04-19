#ifndef DATALOGGERGENERICMENU_H
#define DATALOGGERGENERICMENU_H

#include <QWidget>
#include "gui/generic_menu.hpp"
#include "gui/spinbox_a.hpp"
#include "qcombobox.h"

namespace adiscope {
class CustomSwitch;

namespace gui {
class GenericMenu;

class DataLoggerGenericMenu : public GenericMenu
{
	Q_OBJECT
public:
	explicit DataLoggerGenericMenu(QWidget *parent = nullptr);
	~DataLoggerGenericMenu();

	void init(QString title, QColor* color);

public Q_SLOTS:
	void scaleToggle(bool toggled);
	void peakHolderToggle(bool toggled);
	void historyToggle(bool toggled);
	void peakHolderResetClicked();
	void historySizeChanged(int size);
	void changeLineStyle(int index);

Q_SIGNALS:
	void toggleScale(bool toggle);
	void monitorColorChanged(QString color);
	void toggleHistory(bool toggled);
	void changeHistorySize(int size);
	void lineStyleChanged(Qt::PenStyle lineStyle);
	void lineStyleIndexChanged(int index);
	void historySizeIndexChanged(int index);
	void resetPeakHolder();
	void togglePeakHolder(bool toggled);
	void toggleAll(bool toggle);

private:
	double numSamplesFromIdx(int idx);
	Qt::PenStyle lineStyleFromIdx(int idx);
	void setHistorySize(int idx);
	CustomSwitch *peakHolderSwitch;
	CustomSwitch *scaleSWitch;
	CustomSwitch *historySwitch;
	QPushButton *peakHolderReset;
	QComboBox *historyStyle;
	QComboBox *historySize;
};
}
}

#endif // DATALOGGERGENERICMENU_H
