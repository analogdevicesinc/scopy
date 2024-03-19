#ifndef REGISTERSIMPLEWIDGET_HPP
#define REGISTERSIMPLEWIDGET_HPP

#include "registermodel.hpp"

#include <QColor>
#include <QFrame>
#include <QWidget>
#include <qstring.h>

class QLabel;
class QHBoxLayout;

namespace scopy::regmap {
class BitFieldSimpleWidget;

class RegisterSimpleWidget : public QFrame
{
	friend class RegmapStyleHelper;

	Q_OBJECT
public:
	explicit RegisterSimpleWidget(RegisterModel *registerModel, QVector<BitFieldSimpleWidget *> *bitFields,
				      QWidget *parent = nullptr);
	~RegisterSimpleWidget();

	void valueUpdated(uint32_t value);
	void setRegisterSelected(bool selected);

	RegisterModel *getRegisterModel() const;

public Q_SLOTS:
	void applyStyle();

private:
	QHBoxLayout *layout;
	QLabel *value;
	QVector<BitFieldSimpleWidget *> *bitFields;
	QFrame *regBaseInfoWidget = nullptr;
	QLabel *registerNameLabel;
	RegisterModel *registerModel;

protected:
	bool eventFilter(QObject *object, QEvent *event) override;
Q_SIGNALS:
	void registerSelected(uint32_t address);
};
} // namespace scopy::regmap
#endif // REGISTERSIMPLEWIDGET_HPP
