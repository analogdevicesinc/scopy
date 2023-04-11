#ifndef TOOLSTACK_H
#define TOOLSTACK_H

#include "mapstackedwidget.h"
#include <QMap>
#include "scopycore_export.h"

namespace Ui {
class SCOPYCORE_EXPORT ToolStack;
}
/**
 * @brief The ToolStack class
 */
namespace adiscope {
class SCOPYCORE_EXPORT ToolStack : public MapStackedWidget
{
	Q_OBJECT

public:
	explicit ToolStack(QWidget *parent = nullptr);
	~ToolStack();
};
}

#endif // TOOLSTACK_H
