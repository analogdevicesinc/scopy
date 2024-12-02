/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef EMUWIDGET_H
#define EMUWIDGET_H

#include "scopy-core_export.h"

#include <QComboBox>
#include <QLineEdit>
#include <QProcess>
#include <QWidget>
#include <QLabel>
#include <animationpushbutton.h>

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

Q_SIGNALS:
	void demoEnabled(bool en);

private:
	QWidget *createDemoOptWidget(QWidget *parent);
	QWidget *createXmlPathWidget(QWidget *parent);
	QWidget *createRxTxDevWidget(QWidget *parent);
	QWidget *createUriWidget(QWidget *parent);
	void initEnBtn(QWidget *parent);
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
	void setEnableDemo(bool en);

	QComboBox *m_demoOptCb;
	QLineEdit *m_xmlPathEdit;
	QLineEdit *m_rxTxDevEdit;
	QLineEdit *m_uriEdit;
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
