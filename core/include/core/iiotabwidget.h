#ifndef IIOTABWIDGET_H
#define IIOTABWIDGET_H

#include <QFutureWatcher>
#include <QWidget>

namespace Ui {
class IioTabWidget;
}

namespace scopy {

class IioTabWidget : public QWidget
{
	Q_OBJECT
public:
	IioTabWidget(QWidget *parent = nullptr);
	~IioTabWidget();

public Q_SLOTS:
	void onVerifyFinished(bool result);
	void updateUri(QString uri);
Q_SIGNALS:
	void uriChanged(QString uri);
	void startVerify(QString uri, QString cat);

protected:
	void showEvent(QShowEvent *event) override;
private Q_SLOTS:
	void scanFinished();
	void serialScanFinished();
	void futureScan();
	void futureSerialScan();
	void verifyBtnClicked();

private:
	Ui::IioTabWidget *m_ui;
	QFutureWatcher<int> *fwScan;
	QFutureWatcher<QVector<QString>> *fwSerialScan;
	QStringList scanParamsList;
	QStringList scanList;

	void init();
	void verifyIioBackend();
	void addScanFeedbackMsg(QString message);
	void createBackEndCheckBox(QString backEnd);
	QString getSerialPath();

	const QVector<unsigned int> availableBaudRates = {2400,	 4800,	 9600,	 14400,	 19200, 38400,
							  57600, 115200, 230400, 460800, 921600};
};
} // namespace scopy

#endif // IIOTABWIDGET_H
