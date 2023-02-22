#include <QTest>

class ScopyMainWindowTest : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void hello();
};

void ScopyMainWindowTest::hello() {

};

QTEST_MAIN(ScopyMainWindowTest)

#include "tst_mainwindow.moc"
