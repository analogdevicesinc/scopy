#ifndef RESTARTDIALOG_H
#define RESTARTDIALOG_H

#include "scopy-gui_export.h"
#include <widgets/popupwidget.h>
#include <QDialog>

namespace scopy::gui {
class SCOPY_GUI_EXPORT RestartDialog : public QDialog
{
	Q_OBJECT
public:
	RestartDialog(QWidget *parent = nullptr);
	~RestartDialog();

	void setDescription(const QString &description);
	void showDialog();

Q_SIGNALS:
	void restartButtonClicked();

private:
	QWidget *parent;
	PopupWidget *m_popupWidget;
};
} // namespace scopy::gui
#endif // RESTARTDIALOG_H
