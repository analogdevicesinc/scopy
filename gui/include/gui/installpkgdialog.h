#ifndef INSTALLPKGDIALOG_H
#define INSTALLPKGDIALOG_H

#include <QPushButton>
#include <QWidget>
#include <popupwidget.h>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT InstallPkgDialog : public QWidget
{
	Q_OBJECT
public:
	InstallPkgDialog(QWidget *parent = nullptr);
	~InstallPkgDialog();
	void showDialog();
	void setMessage(const QString &msg);

Q_SIGNALS:
	void yesClicked();
	void noClicked();

private:
	QWidget *parent;
	PopupWidget *m_popupWidget;
};
} // namespace scopy

#endif // INSTALLPKGDIALOG_H
