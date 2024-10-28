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

#ifndef IIOTABWIDGET_H
#define IIOTABWIDGET_H

#include "menucombo.h"
#include <QComboBox>
#include <QFutureWatcher>
#include <QWidget>
#include <animationpushbutton.h>
#include <menulineedit.h>

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
	void setupConnections();
	QStringList computeBackendsList();
	void addScanFeedbackMsg(QString message);
	QCheckBox *createBackendCheckBox(QString backEnd, QWidget *parent);
	void setupFilterWidget(QStringList backednsList);
	QString getSerialPath();
	bool isSerialCompatible();
	void setupBtnLdIcon(AnimationPushButton *btn);
	void rstUriMsgLabel();
	QWidget *createFilterWidget(QWidget *parent);
	QWidget *createAvlCtxWidget(QWidget *parent);
	QWidget *createSerialSettWidget(QWidget *parent);
	QWidget *createUriWidget(QWidget *parent);
	QWidget *createVerifyBtnWidget(QWidget *parent);

	QWidget *m_filterWidget;
	QLabel *m_ctxUriLabel;
	QComboBox *m_avlCtxCb;
	MenuCombo *m_serialPortCb;
	MenuCombo *m_baudRateCb;
	MenuLineEdit *m_serialFrameEdit;
	MenuLineEdit *m_uriEdit;
	QLabel *m_uriMsgLabel;
	AnimationPushButton *m_btnScan;
	AnimationPushButton *m_btnSerialScan;
	AnimationPushButton *m_btnVerify;

	QFutureWatcher<int> *m_fwScan;
	QFutureWatcher<QVector<QString>> *m_fwSerialScan;
	QStringList m_scanParamsList;
	QVector<QPair<QString, QString>> m_scanList;

	const QVector<unsigned int> m_availableBaudRates = {2400,  4800,   9600,   14400,  19200, 38400,
							    57600, 115200, 230400, 460800, 921600};
};
} // namespace scopy

#endif // IIOTABWIDGET_H
