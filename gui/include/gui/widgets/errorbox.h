#ifndef SCOPY_ERRORBOX_H
#define SCOPY_ERRORBOX_H

#include <QFrame>

class ErrorBox : public QFrame
{
	Q_OBJECT
public:
	enum AvailableColors
	{
		Green,
		Yellow,
		Red,
		Transparent,
	};

	explicit ErrorBox(QWidget *parent = nullptr);

public Q_SLOTS:
	void changeColor(AvailableColors color);
};

#endif // SCOPY_ERRORBOX_H
