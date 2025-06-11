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

#ifndef FILEDATAGUISTRATEGY_H
#define FILEDATAGUISTRATEGY_H

#include <QWidget>
#include "dataguistrategyinterface.h"
#include "databufferrecipe.h"
#include "scopy-dac_export.h"
#include "dac_logging_categories.h"

namespace scopy {
class MenuOnOffSwitch;
class TitleSpinBox;
namespace dac {
class SCOPY_DAC_EXPORT FileDataGuiStrategy : public QObject, public DataGuiStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::dac::DataGuiStrategyInterface)
public:
	explicit FileDataGuiStrategy(QWidget *parent = nullptr);
	~FileDataGuiStrategy() {}

	QWidget *ui() override;
	void init() override;

Q_SIGNALS:
	void recipeUpdated(DataBufferRecipe) override;

private:
	QWidget *m_ui;
	MenuOnOffSwitch *m_scaled;
	TitleSpinBox *m_scaleSpin;
};
} // namespace dac
} // namespace scopy
#endif // FILEDATAGUISTRATEGY_H
