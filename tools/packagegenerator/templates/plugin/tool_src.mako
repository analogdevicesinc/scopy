% if 'file_name' in config:
#include "${config['file_name']}.h"

% if 'namespace' in config:
using namespace ${config['namespace']};
% endif

%if 'class_name' in config:
${config['class_name']}::${config['class_name']}(QWidget *parent) {}

${config['class_name']}::~${config['class_name']}() {}
%endif
% endif