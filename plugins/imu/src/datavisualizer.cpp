#include <datavisualizer.hpp>

using namespace scopy;

DataVisualizer::DataVisualizer(QWidget *parent) : QWidget(parent){
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(10);
	setLayout(lay);

	m_accelTextBox = new QPlainTextEdit();
	m_accelTextBox->setFont(QFont("Courier New"));
	m_accelTextBox->setReadOnly(true);
	m_accelTextBox->setFixedHeight(80);
	lay->addWidget(m_accelTextBox);

	m_linearATextBox = new QPlainTextEdit();
	m_linearATextBox->setFont(QFont("Courier New"));
	m_linearATextBox->setReadOnly(true);
	m_linearATextBox->setFixedHeight(80);
	lay->addWidget(m_linearATextBox);

	m_tempTextBox = new QPlainTextEdit();
	m_tempTextBox->setFont(QFont("Courier New"));
	m_tempTextBox->setReadOnly(true);
	m_tempTextBox->setFixedHeight(80);
	lay->addWidget(m_tempTextBox);

}

void DataVisualizer::updateValues(data3P rot, data3P pos, float temp){
	QString infoA= QString("Angle: \nX: %1\nY: %2\nZ: %3")
			       .arg(rot.dataX)
			       .arg(rot.dataY)
			       .arg(rot.dataZ);
	m_accelTextBox->setPlainText(infoA);

	QString infoL = QString("Linear: \nX: %1\nY: %2\nZ: %3")
			       .arg(pos.dataX)
			       .arg(pos.dataY)
			       .arg(pos.dataZ);
	m_linearATextBox->setPlainText(infoL);

	QString infoT = QString("Temperature:   %1ºC\n")
				.arg(temp/100);
	m_tempTextBox->setPlainText(infoT);
}

void DataVisualizer::resetPos(){
	updateValues(data3P{0.0f,0.0f,0.0f} ,data3P{0.0f, 0.0f, 0.0f}, 0.0f);
}
