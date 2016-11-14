#ifndef TestWindow_H
#define TestWindow_H

#include <QMainWindow>
#include <QPushButton>
#include <QProcess>


namespace Ui {
class TestWindow;
}

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TestWindow(QWidget *parent = 0);
    ~TestWindow();
private:
    QPushButton *m_btn;
    QPushButton *m_btn2;
};

#endif // TestWindow_H
