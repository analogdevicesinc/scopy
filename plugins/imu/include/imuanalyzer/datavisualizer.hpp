#ifndef DATAVISUALIZER_HPP
#define DATAVISUALIZER_HPP

#include "scopy-imuanalyzer_export.h"
#include <QWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "imuanalyzerutils.hpp"

namespace scopy {

	class DataVisualizer : public QWidget{
Q_OBJECT
	public:
		DataVisualizer(QWidget *parent = nullptr);

	public Q_SLOTS:
		void updateValues(data3P rot, data3P pos, float temp);

	private:
		data3P m_rot, m_deltaRot;
		float temp = 0.0f;
		QPlainTextEdit *m_accelTextBox, *m_linearATextBox, *m_magnTextBox, *m_tempTextBox;
	};
	}

#endif // DATAVISUALIZER_HPP
