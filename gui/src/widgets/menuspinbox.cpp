#include "menuspinbox.h"
#include <stylehelper.h>

namespace scopy {
namespace gui {

MenuSpinbox::MenuSpinbox(QString name, double val, QString unit, double min, double max, bool vertical, bool left,
			 QWidget *parent)
	: QWidget(parent)
{

	m_label = new QLabel(name, parent);
	m_edit = new QLineEdit("0", parent);
	m_scaleCb = new QComboBox(parent);
	m_plus = new QPushButton("+", parent);
	m_minus = new QPushButton("-", parent);

	m_plus->setAutoRepeat(true);
	m_plus->setAutoRepeatDelay(300);
	m_plus->setAutoRepeatInterval(20);

	m_minus->setAutoRepeat(true);
	m_minus->setAutoRepeatDelay(300);
	m_minus->setAutoRepeatInterval(20);

	if(vertical) {
		layoutVertically(left);
	} else {
		layoutHorizontally(left);
	}

	m_incrementStrategy = new IncrementStrategyPower2();

	connect(m_plus, &QAbstractButton::clicked, this, [=]() { setValue(m_incrementStrategy->increment(m_value)); });
	connect(m_minus, &QAbstractButton::clicked, this, [=]() { setValue(m_incrementStrategy->decrement(m_value)); });

	connect(m_edit, &QLineEdit::editingFinished, this, [=]() { userInput(m_edit->text()); });

	connect(m_scaleCb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_incrementStrategy->setScale(m_scaleCb->itemData(idx).toDouble());
		userInput(m_edit->text());
	});

	m_scales.append({QString("n"), 1e-9});
	m_scales.append({QString("u"), 1e-6});
	m_scales.append({QString("m"), 1e-3});
	m_scales.append({QString(""), 1e0});
	m_scales.append({QString("k"), 1e3});
	m_scales.append({QString("M"), 1e6});
	m_scales.append({QString("G"), 1e9});

	m_name = name;
	m_unit = unit;
	m_min = min;
	m_max = max;
	m_scaleMin = min;
	m_scaleMax = max;
	setScaleRange(m_scaleMin, m_scaleMax);
	setValue(val);
	m_scalingEnabled = true;
}

MenuSpinbox::~MenuSpinbox() { delete m_incrementStrategy; }

void MenuSpinbox::layoutVertically(bool left)
{
	auto lay = new QHBoxLayout(this);
	setLayout(lay);

	lay->setSpacing(5);
	lay->setMargin(0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QVBoxLayout();
	editLay = new QVBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setMargin(0);
	editLay->setSpacing(2);
	editLay->setMargin(0);

	btnLay->addWidget(m_plus);
	btnLay->addWidget(m_minus);

	editLay->addWidget(m_label);
	editLay->addWidget(m_edit);

	m_line = new QFrame(this);
	m_line->setFrameShape(QFrame::HLine);
	m_line->setFrameShadow(QFrame::Plain);
	editLay->addWidget(m_line);
	StyleHelper::MenuSpinboxLine(m_line);
	editLay->addWidget(m_scaleCb);

	if(left) {
		lay->addLayout(btnLay);
		lay->addLayout(editLay);
	} else {
		lay->addLayout(editLay);
		lay->addLayout(btnLay);
	}

	StyleHelper::MenuSpinboxLabel(m_label);
	StyleHelper::MenuSpinboxLineEdit(m_edit);
	StyleHelper::IIOLineEdit(m_edit);
	StyleHelper::MenuSpinComboBox(m_scaleCb);

	StyleHelper::SpinBoxUpButton(m_plus, "plus_btn");
	m_plus->setFixedSize(30, 30);
	StyleHelper::SpinBoxDownButton(m_minus, "minus_btn");
	m_minus->setFixedSize(30, 30);
}

void MenuSpinbox::layoutHorizontally(bool left)
{
	// Layout for the underline
	auto lineLay = new QVBoxLayout(this);
	lineLay->setSpacing(5);
	lineLay->setMargin(0);
	setLayout(lineLay);

	// Elements layout
	auto lay = new QHBoxLayout(this);
	lay->setSpacing(5);
	lay->setMargin(0);

	QLayout *btnLay;
	QLayout *editLay;

	btnLay = new QHBoxLayout();
	editLay = new QHBoxLayout();

	btnLay->setSpacing(2);
	btnLay->setMargin(0);
	editLay->setSpacing(8);
	editLay->setContentsMargins(0, 0, 5, 0);

	btnLay->addWidget(m_minus);
	btnLay->addWidget(m_plus);

	editLay->addWidget(m_label);
	editLay->addWidget(m_edit);

	editLay->addWidget(m_scaleCb);

	m_line = new QFrame(this);
	m_line->setFrameShape(QFrame::HLine);
	m_line->setFrameShadow(QFrame::Plain);
	StyleHelper::MenuSpinboxLine(m_line);

	if(left) {
		lay->addLayout(btnLay);
		lay->addLayout(editLay);
	} else {
		lay->addLayout(editLay);
		lay->addLayout(btnLay);
	}

	StyleHelper::MenuSmallLabel(m_label);
	StyleHelper::IIOLineEdit(m_edit);
	StyleHelper::MenuComboBox(m_scaleCb);

	StyleHelper::SpinBoxUpButton(m_plus, "plus_btn");
	m_plus->setFixedSize(30, 30);
	StyleHelper::SpinBoxDownButton(m_minus, "minus_btn");
	m_minus->setFixedSize(30, 30);

	m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_scaleCb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	lineLay->addLayout(lay);
	lineLay->addWidget(m_line);
}

double MenuSpinbox::value() const { return m_value; }

void MenuSpinbox::setValue(double newValue) { setValueForce(newValue, 0); }

void MenuSpinbox::setValueForce(double newValue, bool force)
{
	if(qFuzzyCompare(m_value, newValue) || force)
		return;

	m_value = clamp(newValue, m_min, m_max);
	populateWidgets();
	Q_EMIT valueChanged(m_value);
}

void MenuSpinbox::setValueString(QString s) { userInput(s); }

QString MenuSpinbox::unit() const { return m_unit; }

void MenuSpinbox::setUnit(const QString &newUnit)
{
	if(m_unit == newUnit)
		return;
	m_unit = newUnit;
	setScaleRange(m_scaleMin, m_scaleMax);
	Q_EMIT unitChanged(newUnit);
}

void MenuSpinbox::setMinValue(double min) { m_min = min; }

void MenuSpinbox::setMaxValue(double max) { m_max = max; }

IncrementStrategy *MenuSpinbox::incrementStrategy() const { return m_incrementStrategy; }

void MenuSpinbox::setIncrementMode(IncrementMode im)
{
	if(m_im == im)
		return;

	m_im = im;
	delete m_incrementStrategy;
	switch(m_im) {

	case IS_POW2:
		m_incrementStrategy = new IncrementStrategyPower2();
		break;
	case IS_125:
		m_incrementStrategy = new IncrementStrategy125();
		break;
	case IS_FIXED:
	default:
		m_incrementStrategy = new IncrementStrategyFixed();
		break;
	}
	m_incrementStrategy->setScale(m_scaleCb->currentData().toDouble());
}

void MenuSpinbox::setScalingEnabled(bool en)
{
	m_scalingEnabled = en;
	m_scaleCb->setVisible(en);
}

void MenuSpinbox::setLineVisible(bool isVisible) { m_line->setVisible(isVisible); }

void MenuSpinbox::userInput(QString s)
{
	// remove whitespace
	s = s.simplified();
	s.replace(" ", "");

	// find last digit position
	int i = findLastDigit(s);
	QString nr = s.left(
		i + 1); // interpret number up to that digit - this makes sure you can also set stuff like 2e6 or 2M
	bool ok;
	double val = nr.toDouble(&ok);
	if(!ok)
		setValue(m_value); // reset

	QString unit = s.mid(i + 1, 1); // isolate prefix and unit from the whole string (mV)
	if(unit.length() > 0) {		// user wrote a prefix and/or a unit
		double scale = getScaleForPrefix(unit, Qt::CaseSensitive); // find the unit in the map
		if(scale == -1) {
			scale = getScaleForPrefix(unit,
						  Qt::CaseInsensitive); // the user may have written 30K instead of 30k
		}

		if(scale == -1) {
			scale = 1; // do not scale the value at all
		} else {
			val = val * scale; // scale accordingly
		}
	} else {
		val = val *
			m_scaleCb->currentData().toDouble(); // the user didnt write a scale => use scale in combobox
	}

	setValue(val);
}

void MenuSpinbox::populateWidgets()
{
	// TODO: Review this function
	if(!m_scalingEnabled) {
		QSignalBlocker sb1(m_edit);
		QSignalBlocker sb2(m_scaleCb);
		m_edit->setText(QString::number(m_value));
		setToolTip(QString::number(m_value, 'f', 6)); // set tooltip
		return;
	}

	int i = 0;
	double scale = 1;
	double absvalue = abs(m_value);
	if(qFuzzyCompare(absvalue, 0)) {
		scale = 1;
		for(i = m_scaleCb->count() - 1; i >= 0; i--) { // find most suitable scale
			if(m_scaleCb->itemData(i).toDouble() == 1)
				break;
		}
	} else {
		for(i = m_scaleCb->count() - 1; i >= 0; i--) { // find most suitable scale
			scale = m_scaleCb->itemData(i).toDouble();
			if(absvalue / scale >= 10)
				break;
		}
		if(i < 0) {
			i = 0;
			scale = m_scaleCb->itemData(i).toDouble();
		}
	}

	QSignalBlocker sb1(m_edit);
	QSignalBlocker sb2(m_scaleCb);
	m_edit->setText(QString::number(m_value / scale)); // reduce number to a meaningful value
	m_scaleCb->setCurrentIndex(i);			   // set apropriate scale in combobox
	m_incrementStrategy->setScale(m_scaleCb->currentData().toDouble());
	setToolTip(QString::number(m_value, 'f', 6)); // set tooltip
}

void MenuSpinbox::applyStylesheet() {}

void MenuSpinbox::setScaleRange(double min, double max)
{
	m_scaleCb->clear();
	for(int i = 0; i < m_scales.count(); i++) {
		auto scale = m_scales[i].scale;
		if(scale >= min && scale <= max) {
			m_scaleCb->addItem(m_scales[i].prefix + m_unit, scale);
		}
	}
	m_incrementStrategy->setScale(m_scaleCb->currentData().toDouble());
}

int MenuSpinbox::findLastDigit(QString str)
{
	for(int i = str.length() - 1; i >= 0; --i) {
		if(str[i].isDigit()) {
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

QString MenuSpinbox::name() const { return m_name; }

void MenuSpinbox::setName(const QString &newName)
{
	if(m_name == newName)
		return;
	m_name = newName;
	m_label->setText(m_name);
	Q_EMIT nameChanged(newName);
}

double MenuSpinbox::getScaleForPrefix(QString prefix, Qt::CaseSensitivity s)
{
	for(int i = 0; i < m_scales.count(); i++) {
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

} // namespace gui
} // namespace scopy

#include "moc_menuspinbox.cpp"
