#ifndef CONTEXTMANAGERMenu_H
#define CONTEXTMANAGERMenu_H

#include <QWidget>

namespace Ui {
class ContextManagerMenu;
}

class ContextManagerMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ContextManagerMenu(QWidget *parent = nullptr);
    ~ContextManagerMenu();

private:
    Ui::ContextManagerMenu *ui;
};

#endif // CONTEXTMANAGERMenu_H
