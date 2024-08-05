#include "menuspinbox.h"
#include <cfloat>



namespace scopy {
namespace gui {

MenuSpinbox::MenuSpinbox(QString name, double val, QString unit, double min, double max, QWidget *parent) : QWidget(parent) {
	lay = new QHBoxLayout(this);
	setLayout(lay);
	m_label = new QLabel(name);
	m_edit = new QLineEdit("0");
	m_scaleCb = new QComboBox();
	m_plus = new QPushButton("+");
	m_minus = new QPushButton("-");

	lay->addWidget(m_label);
	lay->addWidget(m_edit);
	lay->addWidget(m_scaleCb);
	lay->addWidget(m_plus);
	lay->addWidget(m_minus);

	m_incrementStrategy = new IncrementStrategyFixed();

	connect(m_plus, &QAbstractButton::clicked, this, [=](){
		setValue(m_incrementStrategy->increment(m_value));
	});
	connect(m_minus, &QAbstractButton::clicked, this, [=](){
		setValue(m_incrementStrategy->decrement(m_value));
	});

	connect(m_edit, &QLineEdit::editingFinished, this, [=]() {
		userInput(m_edit->text());
	});

	m_scales.append({QString("n"),1e-9});
	m_scales.append({QString("u"),1e-6});
	m_scales.append({QString("m"),1e-3});
	m_scales.append({QString("") ,1e0});
	m_scales.append({QString("k"),1e3});
	m_scales.append({QString("M"),1e6});
	m_scales.append({QString("G"),1e9});

	m_name = name;
	m_unit = unit;
	m_min = min;
	m_max = max;
	populateCombobox(unit,m_min,m_max);
}

MenuSpinbox::~MenuSpinbox()
{
	delete m_incrementStrategy;

}

double MenuSpinbox::value() const
{
	return m_value;
}

void MenuSpinbox::setValue(double newValue, bool force)
{
	if (qFuzzyCompare(m_value, newValue) || force)
		return;

	m_value = clamp(newValue, m_min, m_max);
	populateWidgets();
	Q_EMIT valueChanged(newValue);
}

/*void MenuSpinbox::setValue(QString s)
{
	userInput(s);
}*/

QString MenuSpinbox::unit() const
{
	return m_unit;
}

void MenuSpinbox::setUnit(const QString &newUnit)
{
	if (m_unit == newUnit)
		return;
	m_unit = newUnit;
	populateCombobox(m_unit,m_min, m_max);
	Q_EMIT unitChanged(newUnit);
}



IncrementStrategy *MenuSpinbox::incrementStrategy() const
{
	return m_incrementStrategy;
}

void MenuSpinbox::setIncrementStrategy(IncrementStrategy *newIncrementStrategy)
{
	if(m_incrementStrategy != newIncrementStrategy) {
		delete m_incrementStrategy;
		m_incrementStrategy = newIncrementStrategy;
	}
}

void MenuSpinbox::userInput(QString s)
{
	// remove whitespace
	s = s.simplified();
	s.replace( " ", "" );

	// find last digit position
	int i = findLastDigit(s);
	QString nr = s.left(i+1); // interpret number up to that digit - this makes sure you can also set stuff like 2e6 or 2M
	bool ok;
	double val = nr.toDouble(&ok);
	if(!ok)
		setValue(m_value); // reset

	QString unit = s.mid(i+1,1); // isolate prefix and unit from the whole string (mV)
	if(unit.length() > 0) { // user wrote a prefix and/or a unit
		double scale = getScaleForPrefix(unit, Qt::CaseSensitive);  // find the unit in the map
		if(scale == -1) {
			scale = getScaleForPrefix(unit, Qt::CaseInsensitive); // the user may have written 30K instead of 30k
		}

		if(scale == -1) {
			scale = 1; // do not scale the value at all
		} else  {
			val = val * scale; // scale accordingly
		}
	} else {
		val = val * m_scaleCb->currentData().toDouble(); // the user didnt write a scale => use scale in combobox
	}

	setValue(val);
}

void MenuSpinbox::populateWidgets()
{

	int i = 0;
	double scale = 1;
	for(i = m_scaleCb->count() - 1; i >= 0; i--) { // find most suitable scale
		scale = m_scaleCb->itemData(i).toDouble();
		if(m_value / scale > 1)
			break;
	}
	if( i < 0 ){
		i = 0;
		scale = m_scaleCb->itemData(i).toDouble();
	}

	QSignalBlocker sb1(m_edit);
	QSignalBlocker sb2(m_scaleCb);
	m_edit->setText(QString::number(m_value/scale)); // reduce number to a meaningful value
	m_scaleCb->setCurrentIndex(i); 	// set apropriate scale in combobox
	setToolTip(QString::number(m_value)); // set tooltip

}

void MenuSpinbox::applyStylesheet()
{

}

void MenuSpinbox::populateCombobox(QString unit, double min = 0, double max = DBL_MAX /*, enum metric */) {

	for(int i = 0;i<m_scales.count();i++) {
		auto scale = m_scales[i].scale;
		//if(scale / min > 1 && scale / max < 1) {
			m_scaleCb->addItem(m_scales[i].prefix + unit, scale);
		//}
	}

}

int MenuSpinbox::findLastDigit(QString str)
{
	for (int i = str.length() - 1; i >= 0; --i) {
		if (str[i].isDigit()) {
			return i;
		}
	}
	return -1; // Return -1 if no digit is found
}

double MenuSpinbox::clamp(double val, double min, double max)
{
	val = std::max(val, min);
	val = std::min(val, max);
	return val;
}

QString MenuSpinbox::name() const
{
	return m_name;
}

void MenuSpinbox::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	m_label->setText(m_name);
	Q_EMIT nameChanged(newName);
}

double MenuSpinbox::getScaleForPrefix(QString prefix, Qt::CaseSensitivity s) {
	for(int i = 0 ;i<m_scales.count();i++) {
		if(s == Qt::CaseSensitive) {
			if(m_scales[i].prefix == prefix) {
				return m_scales[i].scale;
			}
		} else {
			if(m_scales[i].prefix.toLower() == prefix.toLower()) {
				return m_scales[i].scale;
			}
		}
	}
	return -1;
}


}
}
