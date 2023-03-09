#ifndef TOOLBROWSER_H
#define TOOLBROWSER_H

#include <QWidget>
#include <toolmenu.h>

namespace Ui {
class ToolBrowser;
}

namespace adiscope {
class ToolBrowser : public QWidget
{
    Q_OBJECT

public:
	explicit ToolBrowser(QWidget *parent = nullptr);
	~ToolBrowser();
	ToolMenu* getToolMenu();

Q_SIGNALS:
	void requestTool(QString);

private:
    Ui::ToolBrowser *ui;
};
}

#endif // TOOLBROWSER_H
