#ifndef PAINTERSAVERESTORE_H
#define PAINTERSAVERESTORE_H

#include <QtGlobal>

class QPainter;

/*
* RAII class for saving and restoring a QPainter
*/
class PainterSaveRestore
{
	Q_DISABLE_COPY_MOVE(PainterSaveRestore)
public:
	PainterSaveRestore(QPainter *painter);
	~PainterSaveRestore();

private:
	QPainter *m_painter;
};

#endif // PAINTERSAVERESTORE_H
