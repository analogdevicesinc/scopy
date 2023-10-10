/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#ifndef FAULTSGROUP_H
#define FAULTSGROUP_H

#include "faultwidget.h"

#include "ui_faultsgroup.h"

#include <QVector>

#include <gui/flexgridlayout.hpp>
#include <set>

QT_BEGIN_NAMESPACE
namespace Ui {
class FaultsGroup;
}
QT_END_NAMESPACE

namespace scopy::swiot {
class FaultsGroup : public QWidget
{
	Q_OBJECT
public:
	explicit FaultsGroup(QString name, const QString &path, QWidget *parent = nullptr);
	~FaultsGroup() override;

	const QVector<FaultWidget *> &getFaults() const;

	const QString &getName() const;
	void setName(const QString &name);

	void clearSelection();
	void update(uint32_t faults_numeric);
	QStringList getExplanations();
	QString getExplanation(unsigned int id);
	std::set<unsigned int> getSelectedIndexes();
	std::set<unsigned int> getActiveIndexes();

Q_SIGNALS:
	void selectionUpdated();
	void minimumSizeChanged();
	void specialFaultsUpdated(unsigned int index, QString channelFunction);
	void specialFaultExplanationChanged(unsigned int, QString);

private:
	Ui::FaultsGroup *ui;
	QString m_name;
	QVector<FaultWidget *> m_faults;
	std::set<unsigned int> m_currentlySelected;
	std::set<unsigned int> m_actives;
	int m_max_faults;
	FlexGridLayout *m_customColGrid;

protected:
	void setupDynamicUi();
	QWidget *buildActiveStoredWidget();
	QJsonArray *getJsonArray(const QString &path);
};
} // namespace scopy::swiot

#endif // FAULTSGROUP_H
