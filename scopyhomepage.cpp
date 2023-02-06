#include "scopyhomepage.h"
#include "ui_scopyhomepage.h"
#include "scopyhomeinfopage.h"
#include "addcontextordemoinfopage.h"

#include "ui_contextmanagermenu.h"

ScopyHomePage::ScopyHomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScopyHomePage)
{
    ui->setupUi(this);
    auto &&is = ui->wInfoPageStack;
    is->add("home",new ScopyHomeInfoPage());
    is->add("add", new AddContextOrDemoInfoPage());
    is->add("dev1", new QLabel("dev1"));
    auto &&dm = ui->wDeviceManager->ui;
    QPushButton *w1 = new QPushButton("dev1");

    dm->ContextBrowser->layout()->addWidget(w1);
    connect(dm->btnHome,&QPushButton::clicked,this,[=]{is->show("home");});
    connect(dm->btnAdd,&QPushButton::clicked,this,[=]{is->show("add");});
    connect(w1,&QPushButton::clicked,this,[=]{is->show("this is dev1");});


}

ScopyHomePage::~ScopyHomePage()
{
    delete ui;
}
