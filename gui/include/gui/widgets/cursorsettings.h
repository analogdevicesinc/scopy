#ifndef CURSORSETTINGS_H
#define CURSORSETTINGS_H

#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>
#include <widgets/menuonoffswitch.h>
#include <widgets/menusectionwidget.h>

namespace scopy {

class SCOPY_GUI_EXPORT CursorSettings : public QWidget
{
	Q_OBJECT
public:
	CursorSettings(QWidget *parent = nullptr);
	~CursorSettings();

	QAbstractButton *getXEn();
	QAbstractButton *getXLock();
	QAbstractButton *getXTrack();
	QAbstractButton *getYEn();
	QAbstractButton *getYLock();
	QAbstractButton *getReadoutsDrag();

	void updateSession();

Q_SIGNALS:
	void sessionUpdated();

protected:
	void initSession();

private:
	MenuSectionWidget *xControls;
	MenuOnOffSwitch *xEn;
	MenuOnOffSwitch *xLock;
	MenuSectionWidget *yControls;
	MenuOnOffSwitch *yEn;
	MenuOnOffSwitch *yLock;
	MenuOnOffSwitch *xTrack;
	MenuSectionWidget *readoutsControls;
	MenuOnOffSwitch *readoutsDrag;
	QVBoxLayout *layout;

	void initUI();
	void connectSignals();
};
} // namespace scopy

#endif // CURSORSETTINGS_H
