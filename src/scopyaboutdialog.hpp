#ifndef SCOPYABOUTDIALOG_HPP
#define SCOPYABOUTDIALOG_HPP

#include <QDialog>
#include <QUrl>

namespace Ui {
class ScopyAboutDialog;
}

namespace adiscope {
class ScopyAboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ScopyAboutDialog(QWidget *parent = nullptr);
	~ScopyAboutDialog();

public Q_SLOTS:
	void dismiss(int);

private:
	Ui::ScopyAboutDialog *ui;
	QUrl landingPage;
};
}
#endif // SCOPYABOUTDIALOG_HPP
