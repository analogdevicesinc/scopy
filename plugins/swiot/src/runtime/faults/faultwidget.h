#ifndef SCOPY_FAULTWIDGET_H
#define SCOPY_FAULTWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include "ui_faultwidget.h"
#include "gui/dynamicWidget.h"
#include <qcoreevent.h>


QT_BEGIN_NAMESPACE
namespace Ui { class FaultWidget; }
QT_END_NAMESPACE

namespace scopy::swiot {
class FaultWidget : public QWidget {
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

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

private:
	Ui::FaultWidget *ui;

	bool m_stored;
	bool m_active;
	bool m_pressed{};
	unsigned int m_id{};
	QString m_name;
	QString m_faultExplanation;
	QJsonObject m_faultExplanationOptions;
};
}

#endif //SCOPY_FAULTWIDGET_H
