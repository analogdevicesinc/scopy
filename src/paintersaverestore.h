#ifndef PAINTERSAVERESTORE_H
#define PAINTERSAVERESTORE_H

#include <QtGlobal>
#include <QPainter>

/*
* RAII class for saving and restoring a QPainter
*/
class PainterSaveRestore
{
public:
	PainterSaveRestore(QPainter *painter);
	~PainterSaveRestore();

private:
	QPainter *m_painter;
};

#endif // PAINTERSAVERESTORE_H
