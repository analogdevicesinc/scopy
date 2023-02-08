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
    auto &&hc = is->getHomepageControls();

    is->add("home",new ScopyHomeInfoPage());
    is->add("add", new AddContextOrDemoInfoPage());
    is->add("dev1", new QLabel("dev1"));
    is->add("dev2", new QLabel("dev2"));

    auto &&db = ui->wDeviceBrowser;
    QPushButton *w1 = new QPushButton("dev1");
    QPushButton *w2 = new QPushButton("dev2");
    w1->setCheckable(true);
    w2->setCheckable(true);
    db->addDevice("dev1",w1);
    db->addDevice("dev2",w2);
    w1->setVisible(false);

    connect(hc,SIGNAL(goLeft()),db,SLOT(prevDevice()));
    connect(hc,SIGNAL(goRight()),db,SLOT(nextDevice()));
    connect(db,SIGNAL(requestDevice(QString,int)),is,SLOT(slideInKey(QString,int)));
}

ScopyHomePage::~ScopyHomePage()
{
    delete ui;
}
