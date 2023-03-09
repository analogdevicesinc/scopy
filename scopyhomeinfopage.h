#ifndef SCOPYHOMEINFOPAGE_H
#define SCOPYHOMEINFOPAGE_H

#include <QWidget>

namespace Ui {
class ScopyHomeInfoPage;
}

class ScopyHomeInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit ScopyHomeInfoPage(QWidget *parent = nullptr);
    ~ScopyHomeInfoPage();

private:
    Ui::ScopyHomeInfoPage *ui;
};

#endif // SCOPYHOMEINFOPAGE_H
