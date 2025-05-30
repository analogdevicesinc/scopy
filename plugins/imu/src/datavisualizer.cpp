#include <datavisualizer.hpp>

using namespace scopy;

DataVisualizer::DataVisualizer(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(10);
	setLayout(lay);

	m_accelTextBox = new QPlainTextEdit();
	m_accelTextBox->setFont(QFont("Courier New"));
	m_accelTextBox->setReadOnly(true);
	lay->addWidget(m_accelTextBox);

	m_linearATextBox = new QPlainTextEdit();
	m_linearATextBox->setFont(QFont("Courier New"));
	m_linearATextBox->setReadOnly(true);
	lay->addWidget(m_linearATextBox);

	m_tempTextBox = new QPlainTextEdit();
	m_tempTextBox->setFont(QFont("Courier New"));
	m_tempTextBox->setReadOnly(true);
	lay->addWidget(m_tempTextBox);

	this->setFixedHeight(80);
	this->setFixedWidth(500);
}

void DataVisualizer::updateValues(data3P rot, data3P pos, float temp)
{
	QString infoA = QString("Rotation: \nX: %1º\nY: %2º").arg(rot.dataX).arg(rot.dataY);
	m_accelTextBox->setPlainText(infoA);

	QString infoL = QString("Linear Acceleration: \nX: %1 m/s²\nY: %2 m/s²\nZ: %3 m/s²")
				.arg(pos.dataX)
				.arg(pos.dataY)
				.arg(pos.dataZ);
	m_linearATextBox->setPlainText(infoL);

	QString infoT = QString("Temperature:   %1ºC\n").arg(temp / 100);
	m_tempTextBox->setPlainText(infoT);
}
