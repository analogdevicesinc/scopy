#include <QtTest/QtTest>
#include <QtWidgets>

#include <scopy/gui/run_single_widget.hpp>

class RunSingleWidgetUnitTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void testSingleRunWidget();
};

void RunSingleWidgetUnitTest::testSingleRunWidget()
{
	scopy::gui::RunSingleWidget* widget = new scopy::gui::RunSingleWidget();

	QTest::mouseClick(widget->getRunButton(), Qt::LeftButton);
	QCOMPARE(widget->getRunButton()->text(), "Stop");

	QTest::mouseClick(widget->getRunButton(), Qt::LeftButton);
	QCOMPARE(widget->getRunButton()->text(), "Run");

	QTest::mouseClick(widget->getSingleButton(), Qt::LeftButton);
	QCOMPARE(widget->getSingleButton()->text(), "Stop");

	QTest::mouseClick(widget->getSingleButton(), Qt::LeftButton);
	QCOMPARE(widget->getSingleButton()->text(), "Single");
}

QTEST_MAIN(RunSingleWidgetUnitTest)
#include "run_single_widget_unittest.moc"
