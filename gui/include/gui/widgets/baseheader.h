#ifndef BASEHEADER_H
#define BASEHEADER_H

#include <QString>

class BaseHeader
{
public:
	virtual void setTitle(QString title) = 0;
	virtual QString title() = 0;
};
#endif // BASEHEADER_H
