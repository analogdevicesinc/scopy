#ifndef TOOLBROWSER_H
#define TOOLBROWSER_H

#include <QWidget>

namespace Ui {
class ToolBrowser;
}

class ToolBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBrowser(QWidget *parent = nullptr);
    ~ToolBrowser();

Q_SIGNALS:
	void toolSelected(QString);

private:
    Ui::ToolBrowser *ui;
};

#endif // TOOLBROWSER_H
