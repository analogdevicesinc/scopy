#ifndef CONTEXTSCANNER_H
#define CONTEXTSCANNER_H

#include <QObject>

class ContextScanner : public QObject
{
	Q_OBJECT
public:
	ContextScanner(QObject *parent = nullptr);
};

#endif // CONTEXTSCANNER_H
