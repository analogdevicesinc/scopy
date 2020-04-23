#include "paintersaverestore.h"

#include <QPainter>

PainterSaveRestore::PainterSaveRestore(QPainter *painter)
: m_painter(painter)
{
	if (m_painter) {
		m_painter->save();
	}
}

PainterSaveRestore::~PainterSaveRestore()
{
	if (m_painter) {
		m_painter->restore();
	}
}
