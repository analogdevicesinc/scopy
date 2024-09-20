#ifndef DEVICEINFOPAGE_H
#define DEVICEINFOPAGE_H

#include "scopy-gui_export.h"
#include <QWidget>
#include <QLabel>
#include <infopage.h>
#include <iioutil/connection.h>

namespace scopy {
class SCOPY_GUI_EXPORT DeviceInfoPage : public QWidget
{
	Q_OBJECT
public:
	explicit DeviceInfoPage(Connection *conn, QWidget *parent = nullptr);

private:
	void setupUi();
	void setupInfoPage();

	Connection *m_conn;
	InfoPage *m_infoPage;
	QLabel *m_title;
};
} // namespace scopy

#endif // DEVICEINFOPAGE_H
