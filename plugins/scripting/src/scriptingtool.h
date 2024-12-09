#ifndef SCRIPTINGTOOL_H
#define SCRIPTINGTOOL_H

#include <QPlainTextEdit>
#include <QWidget>
#include "scopy-scripting_export.h"
#include "scopycodeeditor.h"

namespace scopy {
namespace scripting {

class SCOPY_SCRIPTING_EXPORT ScriptingTool : public QWidget
{
	Q_OBJECT
public:
	explicit ScriptingTool(QWidget *parent = nullptr);

signals:

private:
	QString fileName = "";
	ScopyCodeEditor *codeEditor;
	QPlainTextEdit *codeOutput;

	void loadFile();
	void saveToFile();
	void compileCode();
};

} // namespace scripting
} // namespace scopy
#endif // SCRIPTINGTOOL_H
