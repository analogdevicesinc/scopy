#ifndef BITFIELDSIMPLEWIDGET_HPP
#define BITFIELDSIMPLEWIDGET_HPP

#include <QFrame>
#include <QWidget>

class QHBoxLayout;

class QVBoxLayout;
class QLabel;
namespace scopy::regmap {
class BitFieldSimpleWidget : public QFrame
{
	friend class RegmapStyleHelper;
	Q_OBJECT
public:
	explicit BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width, QString notes,
				      int regOffset, int streach, QWidget *parent = nullptr);

	~BitFieldSimpleWidget();
	void updateValue(QString newValue);
	int getWidth() const;

	QString getDescription() const;

	int getStreach() const;
	QString getName() const;

public Q_SLOTS:
	void applyStyle();
	void setSelected(bool selected);

private:
	QHBoxLayout *layout;
	QLabel *value;
	int width;
	int streach;
	QString description;
	QString name;
	QFrame *mainFrame;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDSIMPLEWIDGET_HPP
