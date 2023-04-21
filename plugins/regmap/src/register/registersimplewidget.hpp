#ifndef REGISTERSIMPLEWIDGET_HPP
#define REGISTERSIMPLEWIDGET_HPP

#include <QColor>
#include <QWidget>
#include <qstring.h>

class QLabel;
class BitFieldSimpleWidget;
class QHBoxLayout;
class RegisterSimpleWidget : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterSimpleWidget(QString name,
				      QString address,
				      QString description,
				      QString notes,
				      int registerWidth,
				      QVector<BitFieldSimpleWidget*> *bitFields,
				      QWidget *parent = nullptr);

	~RegisterSimpleWidget();


	void valueUpdated(uint32_t value);
private:
	QHBoxLayout *layout;
	QLabel *value;
	QVector<BitFieldSimpleWidget*> *bitFields;
	QMap<QString, QColor> *m_colors;
	QColor getColor(QString description);
	QString address;
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
Q_SIGNALS:
    void registerSelected(uint32_t address);
};

#endif // REGISTERSIMPLEWIDGET_HPP
