#ifndef BROWSEMENU_H
#define BROWSEMENU_H

#include "instrumentmenu.h"

#include <QWidget>
#include <gui/widgets/menuwidget.h>
#include <scopy-core_export.h>

namespace scopy {
using namespace gui;

class SCOPY_CORE_EXPORT BrowseMenu : public QWidget
{
	Q_OBJECT
public:
	BrowseMenu(QWidget *parent = nullptr);
	~BrowseMenu();

	InstrumentMenu *instrumentMenu() const;

Q_SIGNALS:
	void requestTool(QString tool);
	void requestSave();
	void requestLoad();

private:
	MenuWidget *m_menu;
	InstrumentMenu *m_instrumentMenu;
};
} // namespace scopy

#endif // BROWSEMENU_H
