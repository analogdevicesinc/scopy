#ifndef CURSORSETTINGS_H
#define CURSORSETTINGS_H

#include <scopy-gui_export.h>
#include <utils.h>
#include <QWidget>
#include <QVBoxLayout>
#include <widgets/menuonoffswitch.h>
#include <widgets/menusectionwidget.h>

namespace scopy {

class SCOPY_GUI_EXPORT CursorSettings : public QWidget
{
	Q_OBJECT
public:
	CursorSettings(QWidget *parent = nullptr);
	~CursorSettings();

	QAbstractButton *getHorizEn();
	QAbstractButton *getHorizLock();
	QAbstractButton *getHorizTrack();
	QAbstractButton *getVertEn();
	QAbstractButton *getVertLock();
	QAbstractButton *getReadoutsDrag();

private:
	MenuSectionWidget *horizControls;
	MenuOnOffSwitch *horizEn;
	MenuOnOffSwitch *horizLock;
	MenuSectionWidget *vertControls;
	MenuOnOffSwitch *vertEn;
	MenuOnOffSwitch *vertLock;
	MenuOnOffSwitch *horizTrack;
	MenuSectionWidget *readoutsControls;
	MenuOnOffSwitch *readoutsDrag;
	QVBoxLayout *layout;

	void initUI();
	void connectSignals();
};
} // namespace scopy

#endif // CURSORSETTINGS_H
