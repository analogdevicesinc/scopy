#include "registermapsettingsmenu.hpp"

#include <QColor>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <QFileDialog>
#include <QDebug>
#include <QComboBox>
#include "dynamicWidget.h"

#include <src/readwrite/fileregisterreadstrategy.hpp>

using namespace scopy;
using namespace regmap;
using namespace regmap::gui;

RegisterMapSettingsMenu::RegisterMapSettingsMenu(QWidget *parent)
    : GenericMenu{parent}
{
    initInteractiveMenu();
    const QColor *color = new QColor("orange");
    setMenuHeader("Settings", color, false);

    QVBoxLayout *settingsLayout = new QVBoxLayout();
    this->setLayout(settingsLayout);

    //autoread section
    scopy::gui::SubsectionSeparator *autoreadSection = new scopy::gui::SubsectionSeparator("Autoread", false);
    insertSection(autoreadSection);

    QCheckBox *autoread = new QCheckBox("Autoread");
    QObject::connect(autoread, &QCheckBox::toggled, this, &RegisterMapSettingsMenu::autoreadToggled);
    autoreadSection->getContentWidget()->layout()->addWidget(autoread);

    // interval section
    scopy::gui::SubsectionSeparator *intervalSection = new scopy::gui::SubsectionSeparator("Interval operations", true);
    insertSection(intervalSection);
    intervalSection->getContentWidget()->layout()->setSpacing(5);

    QLineEdit *startReadInterval = new QLineEdit();
    startReadInterval->setPlaceholderText("From register");
//    startReadInterval->setInputMask("0x");

    QLineEdit *endReadInterval = new QLineEdit();
    endReadInterval->setPlaceholderText("To register");
//    endReadInterval->setInputMask("0x");

    QPushButton *readInterval = new QPushButton("Read interval");
    scopy::setDynamicProperty(readInterval, "blue_button", true);

    QObject::connect(readInterval, &QPushButton::clicked, this, [=](){
        bool ok;
        int startInterval = startReadInterval->text().toInt(&ok,16);
        int endInterval = endReadInterval->text().toInt(&ok,16);
        for (int i = startInterval; i <= endInterval; i++) {
            Q_EMIT requestRead(i);
        }
    });
    intervalSection->getContentWidget()->layout()->addWidget(startReadInterval);
    intervalSection->getContentWidget()->layout()->addWidget(endReadInterval);
    intervalSection->getContentWidget()->layout()->addWidget(readInterval);

    QLineEdit *filePath = new QLineEdit();
    filePath->setPlaceholderText("File path");
    QPushButton *pathButton = new QPushButton("Find path");
    scopy::setDynamicProperty(pathButton, "blue_button", true);

    QObject::connect(pathButton, &QPushButton::clicked, this, [=](){
        filePath->setText(QFileDialog::getOpenFileName(this, ("Open File"), ""));
    });

    intervalSection->getContentWidget()->layout()->addWidget(filePath);
    intervalSection->getContentWidget()->layout()->addWidget(pathButton);


    QPushButton *writeListOfValuesButton = new QPushButton("Write values");
    scopy::setDynamicProperty(writeListOfValuesButton, "blue_button", true);
    QObject::connect(writeListOfValuesButton, &QPushButton::clicked, this, [=](){

        bool ok;
        int startInterval = startReadInterval->text().toInt(&ok,16);
        int endInterval = endReadInterval->text().toInt(&ok,16);

        FileRegisterReadStrategy *fileRead = new FileRegisterReadStrategy(filePath->text());

        QObject::connect(fileRead, &FileRegisterReadStrategy::readDone, this, &RegisterMapSettingsMenu::requestWrite);

        for (int i = startInterval; i <= endInterval; i++) {
            fileRead->read(i);
        }
    });

    intervalSection->getContentWidget()->layout()->addWidget(writeListOfValuesButton);

    QPushButton *registerDump = new QPushButton("Register dump");
    scopy::setDynamicProperty(registerDump, "blue_button", true);
    QObject::connect(registerDump, &QPushButton::clicked, this, [=](){
        if (autoread->isChecked()) {
            readInterval->click();
        }
        QFile::remove(filePath->text());
        Q_EMIT requestRegisterDump(filePath->text());
    });

    intervalSection->getContentWidget()->layout()->addWidget(registerDump);

    QSpacerItem *spacer = new QSpacerItem(10,10,QSizePolicy::Preferred, QSizePolicy::Expanding);
    settingsLayout->addItem(spacer);
}
