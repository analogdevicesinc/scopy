#ifndef SCOPYCODEEDITOR_H
#define SCOPYCODEEDITOR_H

#include <QPlainTextEdit>

namespace scopy {
namespace scripting {

class ScopyCodeEditor : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit ScopyCodeEditor(QWidget *parent = nullptr);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &rect, int dy);

private:
	QWidget *lineNumberArea;
};


class LineNumberArea : public QWidget
{
	Q_OBJECT

public:
	LineNumberArea(ScopyCodeEditor *editor) : QWidget(editor), codeEditor(editor)
	{}

	QSize sizeHint() const override
	{
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	ScopyCodeEditor *codeEditor;
};


} // namespace scripting
} // namespace scopy
#endif // SCOPYCODEEDITOR_H
