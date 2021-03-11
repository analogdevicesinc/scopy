//#include <QtWidgets>
//#include <QtTest/QtTest>

//#include "spinbox_a.hpp"

//class TestGui: public QObject
//{
//    Q_OBJECT

//private:
//	void testCustomSpinBox(scopy::SpinBoxA *spinBox, QString *label, QVector<QString> *cboxData, QVector<QString> *lineEditData);

//private slots:
//	void testSpinBoxButtons();

//};

//void TestGui::testSpinBoxButtons()
//{
//	QWidget *parent = new QWidget();
//	scopy::ScaleSpinButton *scaleSpinBox = new scopy::ScaleSpinButton({
//						 {"sps", 1E0},
//						 {"ksps", 1E+3},
//						 {"Msps", 1E+6}
//						 }, tr("Sample Rate"), 1,
//						 10e7,
//						 true, false, parent, {1, 2, 5});

//	testCustomSpinBox(scaleSpinBox, new QString("Sample Rate"),
//			  new QVector<QString>({"sps", "ksps", "Msps"}),
//			  new QVector<QString>({"1", "2", "20", "20"}));

//	scopy::PositionSpinButton *positionSpinBox = new scopy::PositionSpinButton({
//								    {"s", 1},
//								    {"min", 60},
//								    {"h", 3600}
//							    }, tr("Timer"), 0, 3600,
//							    true, false, parent);

//	testCustomSpinBox(positionSpinBox, new QString("Timer"),
//			  new QVector<QString>({"s", "min", "h"}),
//			  new QVector<QString>({"0", "1", "60", "1"}));

//	scopy::PhaseSpinButton *phaseSPinBox = new scopy::PhaseSpinButton({
//							   {"deg",1},
//							   {"π rad",180}
//						   },tr("Phase"),0,360,true,true,parent);

//	testCustomSpinBox(phaseSPinBox, new QString("Phase"),
//			  new QVector<QString>({"deg", "π rad"}),
//			  new QVector<QString>({"0", "45", "60", "60"}));

//}

//void TestGui::testCustomSpinBox(scopy::SpinBoxA *spinBox, QString *labelValue, QVector<QString> *cboxData, QVector<QString> *lineEditData){
//	QLabel *label = spinBox->getLabel();
//	QComboBox *comboBox = spinBox->getComboBox();
//	QLineEdit *lineEdit = spinBox->getLineEdit();
//	QPushButton *upButton = spinBox->getUpButton();
//	QPushButton *downButton = spinBox->getDownButton();

//	// check initial state
//	QCOMPARE(label->text(), labelValue);
//	QCOMPARE(comboBox->currentText(), cboxData->first());
//	QCOMPARE(lineEdit->text(), lineEditData->first());

//	// press plus button
//	QTest::mouseClick(upButton, Qt::LeftButton);
//	QCOMPARE(lineEdit->text(), lineEditData->at(1));

//	// press down button
//	QTest::mouseClick(downButton,Qt::LeftButton);
//	QCOMPARE(lineEdit->text(), lineEditData->first());

//	// type value in the line edit
//	lineEdit->clear();
//	QTest::keyClicks(lineEdit, lineEditData->at(2));
//	QTest::keyClick(lineEdit, Qt::Key_Enter);
//	QCOMPARE(lineEdit->text(), lineEditData->at(3));

//}

//QTEST_MAIN(TestGui)
//#include "testgui.moc"
