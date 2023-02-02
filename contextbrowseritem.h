#ifndef CONTEXTBROWSERITEM_H
#define CONTEXTBROWSERITEM_H

#include <QWidget>

namespace Ui {
class ContextBrowserItem;
}

class ContextBrowserItem : public QWidget
{
    Q_OBJECT

public:
    explicit ContextBrowserItem(QWidget *parent = nullptr);
    ~ContextBrowserItem();

private:
    Ui::ContextBrowserItem *ui;
};

#endif // CONTEXTBROWSERITEM_H
