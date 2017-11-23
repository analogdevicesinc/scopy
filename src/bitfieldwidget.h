#ifndef BITFIELD_H
#define BITFIELD_H

#include <QWidget>
#include <QDomDocument>

#include <math.h>

namespace Ui {
class BitfieldWidget;
}


namespace adiscope {

class BitfieldWidget : public QWidget
{
	Q_OBJECT

public:
	explicit BitfieldWidget(QWidget *parent = 0, QDomElement *bitfield = nullptr);
	explicit BitfieldWidget(QWidget *parent, int bitNumber);
	void createWidget(void);
	~BitfieldWidget();
	void updateValue(uint32_t *value);
	int getRegOffset(void) const;
	int getSliceWidth(void) const;
	uint32_t getDefaultValue(void) const;

Q_SIGNALS:
	void valueChanged(uint32_t value, uint32_t mask);

private Q_SLOTS:
	void setValue(int value);

private:
	Ui::BitfieldWidget *ui;
	QDomElement *bitfield;
	QDomElement options;

	int width;
	int sliceWidth;
	int regOffset;

	uint32_t value;
	uint32_t defaultValue;

	QString description;
	QString notes;
	QString name, access;
	QString option;

};
}
#endif // BITFIELD_H
