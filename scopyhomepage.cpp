#include "scopyhomepage.h"
#include "ui_scopyhomepage.h"
#include "scopyhomeinfopage.h"
#include "addcontextordemoinfopage.h"
#include <QPushButton>


using namespace adiscope;
ScopyHomePage::ScopyHomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScopyHomePage)
{
    ui->setupUi(this);
    auto &&is = ui->wInfoPageStack;

    is->add("home",new ScopyHomeInfoPage());
    is->add("add", new AddContextOrDemoInfoPage());
    is->add("dev1", new QLabel("dev1"));

    auto &&db = ui->wDeviceBrowser;
    QPushButton *w1 = new QPushButton("dev1");
    w1->setCheckable(true);
    db->addDevice("dev1",w1);


    connect(db,&DeviceBrowser::requestDevice,this,[=](QString k){is->slideInKey(k,true);});

}

ScopyHomePage::~ScopyHomePage()
{
    delete ui;
}
