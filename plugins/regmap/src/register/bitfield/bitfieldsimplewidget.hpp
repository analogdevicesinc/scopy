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
	Q_OBJECT
public:
	explicit BitFieldSimpleWidget(QString name, int defaultValue, QString description, int width, QString notes,
				      int regOffset, int streach, QWidget *parent = nullptr);

	~BitFieldSimpleWidget();
	void updateValue(QString newValue);
	int getWidth() const;

	QString getDescription() const;

	int getStreach() const;
public Q_SLOTS:
	void checkPreferences();
	void setSelected(bool selected);

private:
	QHBoxLayout *layout;
	QLabel *value;
	int width;
	int streach;
	QString description;
	QFrame *mainFrame;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDSIMPLEWIDGET_HPP
