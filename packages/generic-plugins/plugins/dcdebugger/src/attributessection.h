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

#ifndef ATTRIBUTESSECTION_H
#define ATTRIBUTESSECTION_H

#include <QList>
#include <QPointer>
#include <QWidget>

class QEvent;
class QLineEdit;

namespace scopy {
class IIOWidget;
}

namespace scopy::component {
class Attribute;
}

namespace scopy::dcdebugger {

// Read / write editors (IIOWidgets over component::Attribute) for a list of attributes,
// with a name filter when the list is long.
class AttributesSection : public QWidget
{
	Q_OBJECT
public:
	AttributesSection(const QString &title, const QList<component::Attribute *> &attributes,
			  QWidget *parent = nullptr);

	int count() const { return m_widgets.size(); }

public Q_SLOTS:
	// Re-reads every attribute from the device.
	void refresh();
	// Reads the attributes ReadPolicy allows on selection (all, or the ones never read nor cached).
	void autoRefresh();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	void applyFilter(const QString &text);

	QList<QPair<QString, IIOWidget *>> m_widgets;
	QList<QPointer<component::Attribute>> m_attributes; // parallel to m_widgets
	QLineEdit *m_filter = nullptr;
};

} // namespace scopy::dcdebugger

#endif // ATTRIBUTESSECTION_H
