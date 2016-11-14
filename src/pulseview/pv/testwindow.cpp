#include "testwindow.hpp"

TestWindow::TestWindow(QWidget *parent) :
    QMainWindow(parent)
{
    m_btn = new QPushButton("btn", this);
    m_btn2 = new QPushButton("second btn", this);
    m_btn2->setGeometry(100,100,100,100);
}

TestWindow::~TestWindow()
{
}
