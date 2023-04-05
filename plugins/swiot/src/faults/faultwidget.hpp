#ifndef SCOPY_FAULTWIDGET_HPP
#define SCOPY_FAULTWIDGET_HPP

#include <QWidget>
#include "ui_faultwidget.h"
#include "gui/dynamicWidget.h"
#include <qcoreevent.h>


QT_BEGIN_NAMESPACE
namespace Ui { class FaultWidget; }
QT_END_NAMESPACE

namespace adiscope::swiot {
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

	bool isPressed() const;

	void setPressed(bool pressed);

Q_SIGNALS:

	void faultSelected(unsigned int id);

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

private:
	Ui::FaultWidget *ui;

	bool stored;
	bool active;
	bool pressed{};
	unsigned int id{};
	QString name;
	QString faultExplanation;
};
}

#endif //SCOPY_FAULTWIDGET_HPP
