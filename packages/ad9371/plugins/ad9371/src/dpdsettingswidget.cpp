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

#include "dpdsettingswidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

DpdSettingsWidget::DpdSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: m_device(device)
	, m_group(group)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	Style::setStyle(widget, style::properties::widget::border_interactive);
	m_layout->addWidget(widget);

	QLabel *title = new QLabel("DPD Settings", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &t) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, t, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	addRange("adi,dpd-damping", "Damping");
	addRange("adi,dpd-num-weights", "Num Weights");
	addRange("adi,dpd-model-version", "Model Version");
	addRange("adi,dpd-model-prior-weight", "Model Prior Weight");
	addRange("adi,dpd-outlier-threshold", "Outlier Threshold");
	addRange("adi,dpd-additional-delay-offset", "Additional Delay Offset");
	addRange("adi,dpd-path-delay-pn-seq-level", "Path Delay PN Seq Level");
	addRange("adi,dpd-samples", "Samples");
	addRange("adi,dpd-weights0-real", "Weights0 Real");
	addRange("adi,dpd-weights0-imag", "Weights0 Imag");
	addRange("adi,dpd-weights1-real", "Weights1 Real");
	addRange("adi,dpd-weights1-imag", "Weights1 Imag");
	addRange("adi,dpd-weights2-real", "Weights2 Real");
	addRange("adi,dpd-weights2-imag", "Weights2 Imag");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &DpdSettingsWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

DpdSettingsWidget::~DpdSettingsWidget() {}
