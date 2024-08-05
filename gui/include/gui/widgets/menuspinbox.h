#ifndef MENUSPINBOX_H
#define MENUSPINBOX_H

#include "qboxlayout.h"
#include "utils.h"
#include <cmath>
#include <scopy-gui_export.h>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

namespace scopy {
namespace gui {

class SCOPY_GUI_EXPORT IncrementStrategy {
public:
	virtual ~IncrementStrategy() {};
	virtual double increment(double val) = 0;
	virtual double decrement(double val) = 0;
};

/*class SCOPY_GUI_EXPORT IncrementStrategy125 : public IncrementStrategy {
public:
	IncrementStrategy125(){};
	~IncrementStrategy125(){};
	virtual double increment(double val) override;
	virtual double decrement(double val) override;
};

class SCOPY_GUI_EXPORT IncrementStrategyPower2 : public IncrementStrategy {
public:
	IncrementStrategyPower2(){};
	~IncrementStrategyPower2(){};
	virtual double increment(double val) override;
	virtual double decrement(double val) override;
};*/
class SCOPY_GUI_EXPORT IncrementStrategyFixed : public IncrementStrategy {
public:
	IncrementStrategyFixed(double k = 1) { m_k = k;};
	~IncrementStrategyFixed(){};
	virtual double increment(double val) override {
		double pow10 = pow(10,nrOfDigits(val)-2);
		val = val + m_k * pow10;
		return val;
	}
	virtual double decrement(double val) override {
		double pow10 = pow(10,nrOfDigits(val)-2);
		val = val - m_k * pow10;
		return val;
	}
	void setK(double val) {m_k = val;}
	double k() { return m_k;}
private:

	int nrOfDigits(double val) {
		int i = 0;
		while(val >= 1) {
			val = val / 10;
			i++;
		}
		return i;
	}
	double m_k;
};

class SCOPY_GUI_EXPORT UnitPrefix {
public:
	QString prefix;
	double scale;
	// enum type - metric, hour, logarithmic, etc
};


class SCOPY_GUI_EXPORT MenuSpinbox : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER

public:
	MenuSpinbox(QString name, double val, QString unit, double min, double max, QWidget *parent = nullptr);
	~MenuSpinbox();

	double value() const;
	QString unit() const;
	IncrementStrategy *incrementStrategy() const;

	QString name() const;

public Q_SLOTS:
	void setName(const QString &newName);
	void setUnit(const QString &newUnit);
	void setValue(double newValue, bool force = false);
	//void setValue(QString s);
	void setIncrementStrategy(IncrementStrategy *newIncrementStrategy);

Q_SIGNALS:
	void nameChanged(QString);
	void valueChanged(double);
	void unitChanged(QString);

private Q_SLOTS:
	void userInput(QString s);
	void populateWidgets();

private:
	void applyStylesheet();
	void populateCombobox(QString unit, double min, double max);
	int findLastDigit(QString str);
	double clamp(double val, double min, double max);

	QHBoxLayout *lay;

	QLabel *m_label;
	QLineEdit *m_edit;
	QComboBox *m_scaleCb;
	QPushButton *m_plus;
	QPushButton *m_minus;

	IncrementStrategy* m_incrementStrategy;

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
	Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)

	QString m_name;
	double m_value, m_min, m_max;
	QString m_unit;

	QList<UnitPrefix> m_scales;
	// QMap<QString, double> m_scaleMap;
	double getScaleForPrefix(QString prefix, Qt::CaseSensitivity s = Qt::CaseSensitive);
};
}
}
#endif // MENUSPINBOX_H
