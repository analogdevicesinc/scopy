#ifndef SCOPYHOMEINFOPAGE_H
#define SCOPYHOMEINFOPAGE_H

#include <QWidget>
#include "scopycore_export.h"

namespace Ui {
class ScopyHomeInfoPage;
}

class SCOPYCORE_EXPORT ScopyHomeInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit ScopyHomeInfoPage(QWidget *parent = nullptr);
    ~ScopyHomeInfoPage();

private:
    Ui::ScopyHomeInfoPage *ui;
};

#endif // SCOPYHOMEINFOPAGE_H
