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

#include "attributessection.h"

#include "nodepanel.h"
#include "readpolicy.h"

#include <QEvent>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <iiowidget.h>
#include <iiowidgetbuilder.h>
#include <datastrategy/datastrategyinterface.h>
#include <guistrategy/guistrategyinterface.h>
#include <menusectionwidget.h>
#include <mousewheelwidgetguard.h>
#include <pluginbase/preferences.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
// Lists shorter than this get no filter box.
constexpr int FILTER_THRESHOLD = 8;

// An IIOWidget reads its attribute when built or on its first show (lazy loading), whatever the
// read policy says: build with lazy loading on and drop the first show (see eventFilter), so that
// only the policy and the explicit read buttons read.
class LazyLoadOn
{
public:
	LazyLoadOn()
		: m_previous(Preferences::get(KEY))
	{
		set(true);
	}
	~LazyLoadOn() { set(m_previous); }

private:
	void set(const QVariant &v)
	{
		QSignalBlocker quiet(Preferences::GetInstance());
		Preferences::set(KEY, v);
	}
	static constexpr const char *KEY = "iiowidgets_use_lazy_loading";
	QVariant m_previous;
};
} // namespace

AttributesSection::AttributesSection(const QString &title, const QList<component::Attribute *> &attributes,
				     QWidget *parent)
	: QWidget(parent)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	MenuSectionCollapseWidget *section = makeSection(title, this);
	section->contentLayout()->setSpacing(Style::getDimension(json::global::unit_1));
	lay->addWidget(section);

	if(attributes.size() >= FILTER_THRESHOLD) {
		m_filter = new QLineEdit(section);
		m_filter->setPlaceholderText("Filter attributes");
		m_filter->setClearButtonEnabled(true);
		Style::setStyle(m_filter, style::properties::lineedit::menuLineEdit);
		section->contentLayout()->addWidget(m_filter);
		connect(m_filter, &QLineEdit::textChanged, this, &AttributesSection::applyFilter);
	}

	LazyLoadOn lazy;
	for(component::Attribute *attr : attributes) {
		IIOWidget *w = IIOWidgetBuilder(section).attribute(attr).manualReadWrite(true).buildSingle();
		if(!w) {
			continue;
		}
		w->installEventFilter(this);
		w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		section->contentLayout()->addWidget(w);
		m_widgets.append({attr->name(), w});
		m_attributes.append(attr);
		// Nothing is read here: show what is cached.
		if(!attr->cachedValue().isEmpty()) {
			DataStrategyInterface *ds = w->getDataStrategy();
			w->getUiStrategy()->receiveData(ds->data(), ds->optionalData());
		}
	}

	// Scrolling the panel must not change combo / spin values under the cursor.
	auto *guard = new MouseWheelWidgetGuard(this);
	guard->installEventRecursively(section);
}

bool AttributesSection::eventFilter(QObject *watched, QEvent *event)
{
	// The first show of an IIOWidget is its lazy-loading read.
	if(event->type() == QEvent::Show) {
		return true;
	}
	return QWidget::eventFilter(watched, event);
}

void AttributesSection::refresh()
{
	for(const auto &w : std::as_const(m_widgets)) {
		w.second->readAsync();
	}
}

void AttributesSection::autoRefresh()
{
	for(int i = 0; i < m_widgets.size(); ++i) {
		if(m_attributes[i] && ReadPolicy::takeAutoRead(m_attributes[i])) {
			m_widgets[i].second->readAsync();
		}
	}
}

void AttributesSection::applyFilter(const QString &text)
{
	for(const auto &w : std::as_const(m_widgets)) {
		w.second->setVisible(text.isEmpty() || w.first.contains(text, Qt::CaseInsensitive));
	}
}

#include "moc_attributessection.cpp"
