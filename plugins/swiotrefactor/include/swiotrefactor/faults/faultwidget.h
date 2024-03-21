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

#ifndef SCOPY_FAULTWIDGET_H
#define SCOPY_FAULTWIDGET_H

#include "scopy-swiotrefactor_export.h"

#include <QJsonObject>
#include <QLabel>
#include <QFrame>
#include <qcoreevent.h>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT FaultWidget : public QFrame
{
	Q_OBJECT

public:
	explicit FaultWidget(unsigned int id, QString name, QString faultExplanation, QWidget *parent = nullptr);
	~FaultWidget() override;

	unsigned int getId() const;
	void setId(unsigned int id);

	bool isStored() const;
	void setStored(bool stored);

	bool isActive() const;
	void setActive(bool active);

	const QString &getName() const;
	void setName(const QString &name);

	const QString &getFaultExplanation() const;
	void setFaultExplanation(const QString &faultExplanation);

	void setFaultExplanationOptions(QJsonObject options);

	bool isPressed() const;
	void setPressed(bool pressed);

public Q_SLOTS:
	void specialFaultUpdated(int index, QString channelFunction);

Q_SIGNALS:
	void faultSelected(unsigned int id);
	void specialFaultExplanationChanged(unsigned int, QString);

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

private:
	QLabel *m_activeLabel;
	QLabel *m_storedLabel;
	QLabel *m_titleLabel;

	bool m_stored = false;
	bool m_active = false;
	bool m_pressed = false;
	unsigned int m_id{};
	QString m_name;
	QString m_faultExplanation;
	QJsonObject m_faultExplanationOptions;
};
} // namespace scopy::swiotrefactor

#endif // SCOPY_FAULTWIDGET_H
