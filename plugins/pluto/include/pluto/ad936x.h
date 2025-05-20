/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef AD936X_H
#define AD936X_H

#include "scopy-pluto_export.h"
#include <QBoxLayout>
#include <QWidget>
#include <tooltemplate.h>

#include <iio-widgets/iiowidgetbuilder.h>

#include <animatedrefreshbtn.h>

namespace scopy {
namespace pluto {
class SCOPY_PLUTO_EXPORT AD936X : public QWidget
{
	Q_OBJECT
public:
	AD936X(iio_context *ctx, QWidget *parent = nullptr);
	~AD936X();

Q_SIGNALS:
	void readRequested();

private:
	iio_context *m_ctx = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	QWidget *m_controlsWidget;
	QWidget *m_blockDiagramWidget;
	AnimatedRefreshBtn *m_refreshButton;

	QWidget *generateGlobalSettingsWidget(QWidget *parent);

	QWidget *generateRxChainWidget(QWidget *parent);
	QWidget *generateRxWidget(iio_channel *chn, QString title, QWidget *parent);

	QWidget *generateTxChainWidget(QWidget *parent);
	QWidget *generateTxWidget(iio_channel *chn, QString title, QWidget *parent);
};
} // namespace pluto
} // namespace scopy
#endif // AD936X_H
