#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include <QWidget>
#include <gui/homepage_controls.h>

namespace Ui {
class ScopyHomePage;
}

class ScopyHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit ScopyHomePage(QWidget *parent = nullptr);
    ~ScopyHomePage();

private:
    Ui::ScopyHomePage *ui;
//    HomepageControls *hc;
};

#endif // SCOPYHOMEPAGE_H
