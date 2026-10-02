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

#include "mathprocessor.h"
#include "datastore.h"
#include "mathblockwidget.h"

#include <algorithm>
#include <deque>
#include <QWidget>

namespace scopy {
namespace acq {

MathProcessor::MathProcessor(const QString &name, QObject *parent)
	: ProcessorBlock(name, parent)
{
	m_evaluator.setFormula(QStringLiteral("X1"));
}

void MathProcessor::setWatchedKeys(const QList<DataKey> &keys)
{
	ProcessorBlock::setWatchedKeys(keys);
	Q_EMIT inputsChanged();
}

void MathProcessor::process(DataStore *store)
{
	// held owns the chunks; inputs points into it. std::deque never
	// invalidates element addresses on push_back, so the pointers stay valid.
	std::deque<QVector<float>> held;
	QVector<const QVector<float> *> inputs;
	inputs.reserve(m_watchedKeys.size());

	int n = 0;
	for(const DataKey &k : m_watchedKeys) {
		auto src = store->latestAs<QVector<float>>(k);
		if(!src) {
			inputs.append(nullptr); // absent or non-float reads as 0
			continue;
		}
		held.push_back(std::move(*src));
		inputs.append(&held.back());
		n = std::max<int>(n, held.back().size());
	}

	if(n == 0)
		return;

	QVector<float> out(n);
	QString err;
	if(!m_evaluator.evaluateBatch(n, inputs, out, &err))
		// A bad formula fails identically every cycle until it is edited, so
		// reportOnce — the error text is the identity, so an edit to a *different*
		// broken formula still reports.
		reportOnce(AcquisitionError::Severity::Warning,
			   QStringLiteral("formula evaluation failed (%1) — writing zeros").arg(err));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathProcessor::createSettingsWidget(QWidget *parent)
{
	return withBaseSettings(new MathBlockWidget(this), parent);
}

void MathProcessor::setFormula(const QString &formula) { m_evaluator.setFormula(formula); }

} // namespace acq
} // namespace scopy

#include "moc_mathprocessor.cpp"
