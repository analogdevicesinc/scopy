% if 'class_name' in config:
#ifndef ${config['class_name'].upper()}_H
#define ${config['class_name'].upper()}_H

#include "scopy-${plugin_name}_export.h"
#include <QWidget>

% if 'namespace' in config:
namespace ${config['namespace']} {
class ${plugin_export_macro} ${config['class_name']} : public QWidget
{
	Q_OBJECT
public:
	${config['class_name']}(QWidget *parent = nullptr);
	~${config['class_name']}();
};
} // namespace ${config['namespace']}
%endif
#endif // ${config['class_name'].upper()}_H
% endif
