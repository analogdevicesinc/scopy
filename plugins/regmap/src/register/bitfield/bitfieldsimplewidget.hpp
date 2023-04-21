#ifndef BITFIELDSIMPLEWIDGET_HPP
#define BITFIELDSIMPLEWIDGET_HPP

#include <QWidget>

class QVBoxLayout;
class QLabel;
class BitFieldSimpleWidget : public QWidget
{
	Q_OBJECT
public:
	explicit BitFieldSimpleWidget(QString name,
				      int defaultValue,
				      QString description,
				      int width,
				      QString notes,
				      int regOffset,
				      int streach,
				      QWidget *parent = nullptr);

	~BitFieldSimpleWidget();
	void updateValue(QString newValue);
	int getWidth() const;


	QString getDescription() const;

	int getStreach() const;

private:
	QVBoxLayout *layout;
	QLabel *value;
	int width;
	int streach;
	QString description;
Q_SIGNALS:

};

#endif // BITFIELDSIMPLEWIDGET_HPP
