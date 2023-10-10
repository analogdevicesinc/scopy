#ifndef EMUWIDGET_H
#define EMUWIDGET_H

#include "scopy-core_export.h"

#include <QLineEdit>
#include <QProcess>
#include <QWidget>

namespace Ui {
class EmuWidget;
}

namespace scopy {
class SCOPY_CORE_EXPORT EmuWidget : public QWidget
{
	Q_OBJECT
public:
	explicit EmuWidget(QString path = "", QWidget *parent = nullptr);
	~EmuWidget();

Q_SIGNALS:
	void emuDeviceAvailable(QString uri);

protected:
	void showEvent(QShowEvent *event) override;
private Q_SLOTS:
	void onEnableDemoClicked();
	void onOptionChanged(QString option);
	void browseFile(QLineEdit *lineEditPath);

private:
	void init();
	QStringList createArgList();
	void setStatusMessage(QString msg);
	QString findEmuPath();
	void stopEnableBtn(QString btnText);
	bool startIioEmuProcess();
	void killEmuProcess();

	Ui::EmuWidget *m_ui;
	QString m_emuPath;
	bool m_enableDemo;
	QProcess *m_emuProcess;
	const QVector<QString> m_availableOptions{"adalm2000", "generic"};
};
} // namespace scopy

#endif // EMUWIDGET_H
