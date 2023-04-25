#ifndef TOOLBROWSER_H
#define TOOLBROWSER_H

#include <QWidget>
#include <toolmenu.h>
#include "scopy-core_export.h"

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
	ToolMenu* getToolMenu();

public Q_SLOTS:
	void toggleCollapse();
Q_SIGNALS:
	void requestTool(QString);
	void requestSave();
	void requestLoad();

private:
	bool collapsed;
    Ui::ToolBrowser *ui;
};
}

#endif // TOOLBROWSER_H
