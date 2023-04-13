#include <externalloadlineedit.h>


using namespace scopy::m2k;
constexpr double ExternalLoadLineEdit::MAX_EXTERNAL_LOAD;
constexpr double ExternalLoadLineEdit::MIN_EXTERNAL_LOAD;
constexpr double ExternalLoadLineEdit::OUTPUT_AWG_RESISTANCE;

ExternalLoadLineEdit::ExternalLoadLineEdit(QWidget *parent) : QLineEdit(parent) {
	value = MAX_EXTERNAL_LOAD;
	connect(this,SIGNAL(editingFinished()),this,SLOT(setValue()));
}

ExternalLoadLineEdit::~ExternalLoadLineEdit() {

};

double ExternalLoadLineEdit::getValue() {
	return value;
}

void ExternalLoadLineEdit::setValue() {
	setValue(text().toDouble());
}

void ExternalLoadLineEdit::setValue(QString val) {
	setValue(val.toDouble());
}
void ExternalLoadLineEdit::setValue(double val) {
	QString textVal;
	val = std::max(val, MIN_EXTERNAL_LOAD);
	if(val >= MAX_EXTERNAL_LOAD) {
		textVal = "inf";
		val = MAX_EXTERNAL_LOAD;
	} else {
		textVal = QString::number(val);
	}
	setText(textVal);
	value = val;
	Q_EMIT valueChanged(value);
}
