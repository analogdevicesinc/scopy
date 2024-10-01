#ifndef EMUWIDGET_H
#define EMUWIDGET_H

#include "scopy-core_export.h"

#include <QComboBox>
#include <QLineEdit>
#include <QProcess>
#include <QWidget>
#include <animationpushbutton.h>
#include <menulineedit.h>

namespace Ui {
class EmuWidget;
}

namespace scopy {
class SCOPY_CORE_EXPORT EmuWidget : public QWidget
{
	Q_OBJECT
public:
	explicit EmuWidget(QWidget *parent = nullptr);
	~EmuWidget();

Q_SIGNALS:
	void emuDeviceAvailable(QString uri);

protected:
	void showEvent(QShowEvent *event) override;
private Q_SLOTS:
	void onEnableDemoClicked();
	void browseFile(QLineEdit *lineEditPath);

private:
	QWidget *createDemoOptWidget(QWidget *parent);
	QWidget *createXmlPathWidget(QWidget *parent);
	QWidget *createRxTxDevWidget(QWidget *parent);
	QWidget *createUriWidget(QWidget *parent);
	QWidget *createEnBtnWidget(QWidget *parent);
	void init();
	void enGenericOptWidget(QWidget *xmlPathWidget, QWidget *rxTxDevWidget, QString crtOpt);
	QStringList createArgList();
	void setStatusMessage(QString msg);
	QString findEmuPath();
	void stopEnableBtn(QString btnText);
	bool startIioEmuProcess(QString processPath, QStringList arg = {});
	void killEmuProcess();

	void getEmuOptions();
	void configureOption(QString option);
	void getJsonConfiguration();

	QWidget *m_emuWidget;
	QComboBox *m_demoOptCb;
	MenuLineEdit *m_xmlPathEdit;
	MenuLineEdit *m_rxTxDevEdit;
	MenuLineEdit *m_uriEdit;
	QLabel *m_uriMsgLabel;
	AnimationPushButton *m_enDemoBtn;

	QString m_emuPath;
	QString m_workingDir;
	bool m_enableDemo;
	QProcess *m_emuProcess;
	QStringList m_availableOptions;

	QString m_jsonConfigVal;
	QString m_emuType = "generic";
};
} // namespace scopy

#endif // EMUWIDGET_H
