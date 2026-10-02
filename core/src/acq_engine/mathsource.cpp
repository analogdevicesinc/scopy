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

#include "mathsource.h"
#include "datastore.h"
#include "mathblockwidget.h"

#include <QWidget>

namespace scopy {
namespace acq {

MathSource::MathSource(const QString &id, QObject *parent)
	: SourceBlock(id, parent)
	, m_outputKey(DataKey::withStage(id, "out", "math"))
{}

void MathSource::acquire(DataStore *store)
{
	QVector<float> out(static_cast<int>(m_bufferSize));
	QString err;
	const QVector<const QVector<float> *> noInputs;
	if(!m_evaluator.evaluateBatch(out.size(), noInputs, out, &err))
		// Per-cycle; same reasoning as MathProcessor::process().
		reportOnce(AcquisitionError::Severity::Warning,
			   QStringLiteral("formula evaluation failed (%1) — writing zeros").arg(err));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathSource::createSettingsWidget(QWidget *parent)
{
	return withBaseSettings(new MathBlockWidget(&m_evaluator), parent);
}

void MathSource::setFormula(const QString &formula) { m_evaluator.setFormula(formula); }

} // namespace acq
} // namespace scopy

#include "moc_mathsource.cpp"
