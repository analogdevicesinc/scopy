#ifndef SCOPY_PROGRESSLINEEDIT_H
#define SCOPY_PROGRESSLINEEDIT_H

#include <QLineEdit>
#include <QVBoxLayout>
#include <stylehelper.h>
#include <widgets/smallprogressbar.h>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT ProgressLineEdit : public QWidget
{
	Q_OBJECT
public:
	explicit ProgressLineEdit(QWidget *parent = nullptr);

	SmallProgressBar *getProgressBar();
	QLineEdit *getLineEdit();

private:
	QLineEdit *m_lineEdit;
	SmallProgressBar *m_progressBar;
};
} // namespace scopy

#endif // SCOPY_PROGRESSLINEEDIT_H
