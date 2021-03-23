#ifndef SCOPYABOUTDIALOG_HPP
#define SCOPYABOUTDIALOG_HPP

#include <QDialog>
#include <QUrl>

namespace Ui {
class ScopyAboutDialog;
}

namespace scopy {
namespace gui {

class ScopyAboutDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ScopyAboutDialog(QWidget* parent = nullptr);
	~ScopyAboutDialog();

Q_SIGNALS:
	void forceCheckForUpdates();

public Q_SLOTS:
	void dismiss(int);
	void updateCheckUpdateLabel(qint64);

private:
	Ui::ScopyAboutDialog* m_ui;
	QUrl m_landingPage;
};
} // namespace gui
} // namespace scopy

#endif // SCOPYABOUTDIALOG_HPP
