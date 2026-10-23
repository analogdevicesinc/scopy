/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef BACKENDSTABWIDGET_H
#define BACKENDSTABWIDGET_H

#include <QCheckBox>
#include <QComboBox>
#include <QFutureWatcher>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QWidget>
#include <animationpushbutton.h>

#include <component/contextbackend.h>

namespace scopy {

// Add-page tab over every registered context backend: one filter checkbox per backend, one scan
// merging their results, and a URI entry with a backend selector. The backend id is the device
// category. Backends registered later (e.g. from Plugin::init()) are added as they appear.
class BackendsTabWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BackendsTabWidget(QWidget *parent = nullptr);
	~BackendsTabWidget() override;

public Q_SLOTS:
	void onVerifyFinished(bool result);
	void updateUri(QString uri);

Q_SIGNALS:
	void startVerify(QString uri, QString cat);

protected:
	void showEvent(QShowEvent *event) override;

private Q_SLOTS:
	void futureScan();
	void scanFinished();
	void verifyBtnClicked();
	void addBackend(const QString &id);
	void removeBackend(const QString &id);
	void updateUriHint(const QString &id);

private:
	QWidget *createScanSection(QWidget *parent);
	QWidget *createUriWidget(QWidget *parent);
	AnimationPushButton *createLoadingButton(QWidget *parent);
	void addScanFeedbackMsg(const QString &message);
	void selectResult(int idx);

	QWidget *m_filterWidget;
	QMap<component::BackendId, QCheckBox *> m_filters;
	QComboBox *m_resultsCb;
	AnimationPushButton *m_btnScan;
	QLineEdit *m_uriEdit;
	QComboBox *m_backendCb;
	AnimationPushButton *m_btnVerify;
	QLabel *m_uriMsgLabel;

	QVector<component::ScanResult> m_results;
	QFutureWatcher<QVector<component::ScanResult>> *m_fwScan;
};

} // namespace scopy

#endif // BACKENDSTABWIDGET_H
