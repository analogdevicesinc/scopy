#ifndef EXTERNALLOADLINEEDIT_H
#define EXTERNALLOADLINEEDIT_H

#include <QLineEdit>

namespace adiscope::m2k {
class ExternalLoadLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	static constexpr double MAX_EXTERNAL_LOAD = 100000.0;
	static constexpr double MIN_EXTERNAL_LOAD = 50.0;
	static constexpr double OUTPUT_AWG_RESISTANCE = 50.0;

	ExternalLoadLineEdit(QWidget *parent = nullptr);
	~ExternalLoadLineEdit();

	Q_PROPERTY(double value READ getValue WRITE setValue);

	double getValue();
public Q_SLOTS:
	void setValue();
	void setValue(double val);
	void setValue(QString val);

Q_SIGNALS:
	void valueChanged(double);
protected:
	double value;

};
}
#endif // EXTERNALLOADLINEEDIT
