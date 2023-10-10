#ifndef TOOLBROWSER_H
#define TOOLBROWSER_H

#include "scopy-core_export.h"

#include <QWidget>

#include <toolmenu.h>

namespace Ui {
class ToolBrowser;
}

namespace scopy {
class SCOPY_CORE_EXPORT ToolBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit ToolBrowser(QWidget *parent = nullptr);
	~ToolBrowser();
	ToolMenu *getToolMenu();

public Q_SLOTS:
	void toggleCollapse();
Q_SIGNALS:
	void collapsed(bool);
	void requestTool(QString);
	void requestSave();
	void requestLoad();

private:
	bool m_collapsed;
	Ui::ToolBrowser *ui;
	void hideMenuText(bool collapesd);
};
} // namespace scopy

#endif // TOOLBROWSER_H
