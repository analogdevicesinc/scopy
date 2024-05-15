#ifndef TITLESPINBOX_H
#define TITLESPINBOX_H

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT TitleSpinBox : public QWidget
{
	Q_OBJECT

public:
	explicit TitleSpinBox(QString title, QWidget *parent = nullptr);
	~TitleSpinBox();

	void setTitle(QString title);
	QPushButton *getSpinBoxUpButton();
	QPushButton *getSpinBoxDownButton();
	QSpinBox *getSpinBox();

private:
	QPushButton *m_spinBoxUpButton;
	QPushButton *m_spinBoxDownButton;
	QLabel *m_titleLabel;
	QSpinBox *m_spinBox;
};
} // namespace scopy
#endif // TITLESPINBOX_H
