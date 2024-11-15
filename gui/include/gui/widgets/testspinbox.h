#ifndef TESTSPINBOX_H
#define TESTSPINBOX_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <incrementstrategy.h>
#include <mousewheelwidgetguard.h>
#include <scale.h>
#include <scopy-gui_export.h>

namespace scopy {
namespace gui {

class SCOPY_GUI_EXPORT TestSpinbox : public QWidget
{
	Q_OBJECT
public:
	typedef enum
	{
		IS_POW2,
		IS_125,
		IS_FIXED

	} IncrementMode;

	explicit TestSpinbox(QString name, double val, QString unit, double min, double max, bool vertical = 0,
			     bool left = 0, QWidget *parent = nullptr);

	IncrementStrategy *incrementStrategy() const;

	double value() const;

	Scale *scale() const;
	void setScale(Scale *newScale);
	void setScalingEnabled(bool en);
	bool scallingEnabled();

public Q_SLOTS:
	void setIncrementMode(IncrementMode im);
	void setValue(double newValue);
	void setValueForce(double newValue, bool force = true);

Q_SIGNALS:
	void nameChanged(QString);
	void valueChanged(double);
	void unitChanged(QString);

private:
	void layoutVertically(bool left);
	void layoutHorizontally(bool left);
	void updateWidgetsVal();
	double clamp(double val, double min, double max);

	QLabel *m_label;
	QLineEdit *m_edit;
	QPushButton *m_plus;
	QPushButton *m_minus;
	MouseWheelWidgetGuard *m_mouseWheelGuard;

	double m_value, m_min, m_max;

	IncrementStrategy *m_incrementStrategy;
	IncrementMode m_im;

	Scale *m_scale;

	int findLastDigit(QString str);
	double userInput(QString s);
};
} // namespace gui
} // namespace scopy
#endif // TESTSPINBOX_H
