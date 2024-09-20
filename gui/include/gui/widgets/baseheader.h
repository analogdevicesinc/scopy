#ifndef BASEHEADER_H
#define BASEHEADER_H

#include <QWidget>

class BaseHeader : public QWidget
{
public:
	BaseHeader(QWidget *parent)
		: QWidget(parent)
	{}
	~BaseHeader() {}
	virtual void setTitle(QString title) = 0;
	virtual QString title() = 0;
};
#endif // BASEHEADER_H
