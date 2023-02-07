#ifndef SCOPYHOMEPAGE_H
#define SCOPYHOMEPAGE_H

#include <QWidget>

namespace Ui {
class ScopyHomePage;
}

namespace adiscope {
class ScopyHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit ScopyHomePage(QWidget *parent = nullptr);
    ~ScopyHomePage();

private:
    Ui::ScopyHomePage *ui;
};
}

#endif // SCOPYHOMEPAGE_H
