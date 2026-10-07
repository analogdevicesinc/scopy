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
 */

#include "diogroupwidget.h"

#include "diopinwidget.h"
#include "m2kdigitaliocontroller.h"

#include "component/attribute.h"
#include "component/attributereader.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <gui/style.h>
#include <gui/customSwitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <iio-widgets/iiowidgetbuilder.h>

Q_LOGGING_CATEGORY(CAT_DIOGROUPWIDGET, "DioGroupWidget")

using namespace scopy;
using namespace scopy::adalm2000;

namespace {
constexpr int INDIVIDUAL_PAGE = 0;
constexpr int GROUP_PAGE = 1;
constexpr int SLIDER_TICK_INTERVAL = 32;
constexpr int SLIDER_WRITE_COALESCE_MS = 50;
} // namespace

DioGroupWidget::DioGroupWidget(M2kDigitalIoController *controller, IIOWidgetGroup *group, int groupIndex,
			       QWidget *parent)
	: QWidget(parent)
	, m_controller(controller)
	, m_group(group)
	, m_groupIndex(groupIndex)
{
	const int base = dio::groupBase(groupIndex);

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(Style::getDimension(json::global::unit_1));

	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		QStringLiteral("DIO %1 - %2").arg(base).arg(base + dio::PINS_PER_GROUP - 1),
		MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *centralWidget = new QWidget(section);
	QVBoxLayout *centralWidgetLayout = new QVBoxLayout(centralWidget);
	centralWidgetLayout->setContentsMargins(0, 0, 0, 0);
	section->contentLayout()->addWidget(centralWidget);

	m_modeCombo = new MenuCombo(QString(), centralWidget);
	m_modeCombo->combo()->addItem(tr("Individual"));
	m_modeCombo->combo()->addItem(tr("Group"));
	centralWidgetLayout->addWidget(m_modeCombo);

	m_stack = new QStackedWidget(this);

	auto *individual = new QWidget(m_stack);
	auto *individualLayout = new QHBoxLayout(individual);
	individualLayout->setContentsMargins(0, 0, 0, 0);
	for(int i = 0; i < dio::PINS_PER_GROUP; ++i) {
		m_pins[i] = new DioPinWidget(controller, group, base + i, individual);
		individualLayout->addWidget(m_pins[i], 1);
	}
	m_stack->insertWidget(INDIVIDUAL_PAGE, individual);

	auto *grouped = new QWidget(m_stack);
	auto *groupedLayout = new QHBoxLayout(grouped);
	groupedLayout->setContentsMargins(0, 0, 0, 0);
	groupedLayout->setSpacing(Style::getDimension(json::global::unit_2));

	if(controller) {
		component::Attribute *dirAttr = controller->groupDirection(groupIndex);
		if(dirAttr) {
			m_directionWidget =
				IIOWidgetBuilder(grouped)
					.attribute(dirAttr)
					.uiStrategy(IIOWidgetBuilder::SwitchUi)
					.infoMessage(
						tr("Direction for all %1 pins in this block").arg(dio::PINS_PER_GROUP))
					.group(group)
					.buildSingle();
		}
		if(m_directionWidget) {
			m_directionWidget->showProgressBar(false);
			if(!m_directionWidget->findChild<CustomSwitch *>()) {
				qWarning(CAT_DIOGROUPWIDGET)
					<< "no CustomSwitch inside the group direction IIOWidget for group"
					<< groupIndex;
			}
			groupedLayout->addWidget(m_directionWidget);

			connect(dirAttr, &component::Attribute::valueChanged, this,
				&DioGroupWidget::onGroupDirectionChanged);
		} else {
			qWarning(CAT_DIOGROUPWIDGET) << "group" << groupIndex << "has no direction attribute";
		}

		component::Attribute *valAttr = controller->groupValue(groupIndex);

		m_slider = new QSlider(Qt::Horizontal, grouped);
		m_slider->setRange(0, dio::GROUP_VALUE_MAX);
		m_slider->setTickPosition(QSlider::TicksBelow);
		m_slider->setTickInterval(SLIDER_TICK_INTERVAL);
		m_slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
		m_slider->setToolTip(tr("Value driven onto this block's %1 pins, or the value read "
					"back from them while the block is an input")
					     .arg(dio::PINS_PER_GROUP));
		groupedLayout->addWidget(m_slider, 1);

		if(valAttr) {
			// The attribute's {0, 1, 255} range triple is what makes
			// IIOWidgetBuilder pick a spinbox over a line edit.
			m_valueWidget = IIOWidgetBuilder(grouped)
						.attribute(valAttr)
						.title(tr("Value"))
						.group(group)
						.buildSingle();
		}
		if(m_valueWidget) {
			m_valueWidget->showProgressBar(false);
			m_valueSpin = m_valueWidget->findChild<gui::MenuSpinbox *>();
			if(!m_valueSpin) {
				qWarning(CAT_DIOGROUPWIDGET)
					<< "no MenuSpinbox inside the group value IIOWidget for group" << groupIndex;
			}
			groupedLayout->addWidget(m_valueWidget);
		} else {
			qWarning(CAT_DIOGROUPWIDGET) << "group" << groupIndex << "has no value attribute";
		}

		if(valAttr) {
			valueUpdated(valAttr->cachedValue().toInt());

			// Coalesced: each write fans out to PINS_PER_GROUP setValueRaw
			// calls, so an uncoalesced drag is ~100 writes a second.
			m_sliderWriteTimer = new QTimer(this);
			m_sliderWriteTimer->setSingleShot(true);
			m_sliderWriteTimer->setInterval(SLIDER_WRITE_COALESCE_MS);
			connect(m_sliderWriteTimer, &QTimer::timeout, this, [this, valAttr]() {
				if(!m_groupIsOutput || !valAttr->writeCapability()) {
					return;
				}
				valAttr->writeCapability()->writeAsync(QString::number(m_slider->value()));
			});
			connect(m_slider, &QSlider::valueChanged, this, [this](int value) {
				valueUpdated(value);
				// Input mode: the poll owns the position, and writing would
				// drive pins the user set to read.
				if(!m_groupIsOutput) {
					return;
				}
				m_sliderWriteTimer->start(); // restart: only the last value lands
			});
			if(m_valueSpin) {
				connect(m_valueSpin, &gui::MenuSpinbox::valueChanged, this,
					[this](double value) { valueUpdated(static_cast<int>(value)); });
			}

			// Output mode only: in input mode the poll owns the displays.
			connect(valAttr, &component::Attribute::valueChanged, this, [this](const QString &v) {
				if(m_groupIsOutput) {
					valueUpdated(v.toInt());
				}
			});
		}
	}

	m_stack->insertWidget(GROUP_PAGE, grouped);

	centralWidgetLayout->addWidget(m_stack);

	m_stack->setCurrentIndex(INDIVIDUAL_PAGE);
	m_modeCombo->combo()->setCurrentIndex(INDIVIDUAL_PAGE);
	connect(m_modeCombo->combo(), &QComboBox::activated, this, &DioGroupWidget::onModeChanged);

	setInputState(0, 0);

	layout->addWidget(section);
}

DioGroupWidget::~DioGroupWidget() {}

void DioGroupWidget::onModeChanged(int index)
{
	m_stack->setCurrentIndex(index);

	// Re-writing the group direction attribute is what re-applies it to all eight
	// pins; its writer is that fan-out.
	if(index != GROUP_PAGE || !m_controller) {
		return;
	}
	component::Attribute *dirAttr = m_controller->groupDirection(m_groupIndex);
	if(dirAttr && dirAttr->writeCapability()) {
		dirAttr->writeCapability()->writeAsync(dirAttr->cachedValue().isEmpty() ? QStringLiteral("in")
											: dirAttr->cachedValue());
	}
}

void DioGroupWidget::onGroupDirectionChanged(const QString &value)
{
	const bool wasOutput = m_groupIsOutput;
	m_groupIsOutput = (value == QLatin1String("out"));

	// Only the group controls grey out; the per-pin toggles stay live so a level can
	// be buffered before the pin becomes an output.
	if(m_valueWidget) {
		m_valueWidget->setEnabled(m_groupIsOutput);
	}
	if(m_slider) {
		m_slider->setEnabled(m_groupIsOutput);
	}

	if(m_groupIsOutput && !wasOutput && m_controller) {
		if(component::Attribute *valAttr = m_controller->groupValue(m_groupIndex)) {
			valueUpdated(valAttr->cachedValue().toInt());
		}
	}
}

void DioGroupWidget::valueUpdated(int value)
{
	QSignalBlocker sliderBlocker(m_slider);
	QSignalBlocker spinBlocker(m_valueSpin);
	if(m_slider) {
		m_slider->setValue(value);
	}
	if(m_valueSpin) {
		m_valueSpin->setValue(value);
	}
}

void DioGroupWidget::setInputState(quint16 gpi, quint16 shorted)
{
	const int base = dio::groupBase(m_groupIndex);
	for(int i = 0; i < dio::PINS_PER_GROUP; ++i) {
		if(m_pins[i]) {
			m_pins[i]->setInputState(dio::bit(gpi, base + i), dio::bit(shorted, base + i));
		}
	}
	// In input mode the group controls are a live read-back of the eight pins.
	if(!m_groupIsOutput) {
		valueUpdated(dio::groupValue(gpi, m_groupIndex));
	}
}

bool DioGroupWidget::isGrouped() const { return m_stack && m_stack->currentIndex() == GROUP_PAGE; }

void DioGroupWidget::setGrouped(bool grouped)
{
	const int index = grouped ? GROUP_PAGE : INDIVIDUAL_PAGE;
	if(m_modeCombo) {
		m_modeCombo->combo()->setCurrentIndex(index);
	}
	// setCurrentIndex() does not emit activated(), which is user-interaction only,
	// so the side effect has to be invoked explicitly.
	onModeChanged(index);
}

DioPinWidget *DioGroupWidget::pinWidget(int pin) const
{
	const int local = pin - dio::groupBase(m_groupIndex);
	if(local < 0 || local >= dio::PINS_PER_GROUP) {
		return nullptr;
	}
	return m_pins[local];
}

#include "moc_diogroupwidget.cpp"
