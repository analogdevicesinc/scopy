#ifndef SCOPYMAINWINDOW_H
#define SCOPYMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ScopyMainWindow; }
QT_END_NAMESPACE
namespace adiscope {
class ScopyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ScopyMainWindow(QWidget *parent = nullptr);
    ~ScopyMainWindow();

private:
    Ui::ScopyMainWindow *ui;
};
}
#endif // SCOPYMAINWINDOW_H
