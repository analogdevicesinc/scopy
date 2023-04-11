#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();


	struct iio_device* getIioDevice(const char *dev_name);

	struct iio_context *ctx;
private:
	Ui::MainWindow *ui;
};
#endif // MAINWINDOW_HPP
