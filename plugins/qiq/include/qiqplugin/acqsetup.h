#ifndef ACQSETUP_H
#define ACQSETUP_H

#include "menucombo.h"
#include "menuonoffswitch.h"
#include <iio.h>
#include <QWidget>
#include <QListWidget>
#include <menulineedit.h>
#include <filebrowserwidget.h>

namespace scopy::qiqplugin {

class AcqSetup : public QWidget
{
	Q_OBJECT
public:
	typedef struct
	{
		int samples;
		QString deviceName;
		QString cliPath;
		QStringList enChnls;
	} AcqConfig;

	AcqSetup(QWidget *parent = nullptr);
	~AcqSetup();

public Q_SLOTS:
	void init(QMap<QString, QStringList> map);
Q_SIGNALS:
	void configPressed(AcqConfig config);

private Q_SLOTS:
	void onConfigPressed();

private:
	void initChnlList(const QString &devName);
	void clearChnlsList();
	QStringList getBufferedDevices();

	int m_enChnls;
	MenuCombo *m_devicesCb;
	MenuLineEdit *m_sampleRateEdit;
	QWidget *m_chnlsW;
	FileBrowserWidget *m_fileBrowser;

	QMap<QString, MenuOnOffSwitch *> m_chnlMap;
	QMap<QString, QStringList> m_map;
};
} // namespace scopy::qiqplugin

#endif // ACQSETUP_H
