#ifndef TOOLSTACK_H
#define TOOLSTACK_H

#include "gui/mapstackedwidget.h"
#include "scopy-core_export.h"

#include <QMap>

namespace Ui {
class SCOPY_CORE_EXPORT ToolStack;
}
/**
 * @brief The ToolStack class
 */
namespace scopy {
class SCOPY_CORE_EXPORT ToolStack : public MapStackedWidget
{
	Q_OBJECT

public:
	explicit ToolStack(QWidget *parent = nullptr);
	~ToolStack();
};
} // namespace scopy

#endif // TOOLSTACK_H
