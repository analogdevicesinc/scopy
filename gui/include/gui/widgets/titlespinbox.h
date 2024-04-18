#ifndef TITLESPINBOX_H
#define TITLESPINBOX_H

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
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
	QLineEdit *getLineEdit();

	double step() const;
	void setStep(double newStep);

	double max() const;
	void setMax(double newMax);

	double min() const;
	void setMin(double newMin);

	void setValue(double newValue);
	void setSpinButtonsDisabled(bool isDisabled);

private:
	/**
	 * @brief truncValue This function is needed because the QString::number function that
	 * would be normally used to parse a QString to a number does not work well with large
	 * numbers that end with multiple zeroes. The QString returned will be in scientific
	 * notation. The alternative is that we can set a number of fixed decimal points, but
	 * it would look awkward so this function converts it to a QString and chops the
	 * trailing zeroes.
	 * @param value
	 * @return
	 */
	static QString truncValue(double value);

	QPushButton *m_spinBoxUpButton;
	QPushButton *m_spinBoxDownButton;
	QLabel *m_titleLabel;

	// This lineedit will act like a spinbox because the QSpinBox
	// is more restrictive than we would like :/
	QLineEdit *m_lineedit;
	double m_min;
	double m_max;
	double m_step;
};
} // namespace scopy
#endif // TITLESPINBOX_H
