#ifndef IIODEBUGINSTRUMENT_H
#define IIODEBUGINSTRUMENT_H

#include "scopy-iiodebugplugin_export.h"
#include <QWidget>

namespace scopy::iiodebugplugin {
class SCOPY_IIODEBUGPLUGIN_EXPORT IIODebugInstrument : public QWidget
{
	Q_OBJECT
public:
	IIODebugInstrument(QWidget *parent = nullptr);
	~IIODebugInstrument();
};
} // namespace scopy::iiodebugplugin
#endif // IIODEBUGINSTRUMENT_H
