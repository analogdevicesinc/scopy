#include "scopycodeeditor.h"

#include <QPainter>
#include <QTextBlock>
#include <style.h>

using namespace scopy;
using namespace scripting;

ScopyCodeEditor::ScopyCodeEditor(QWidget *parent)
	: QPlainTextEdit{parent}
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, &ScopyCodeEditor::blockCountChanged, this, &ScopyCodeEditor::updateLineNumberAreaWidth);
	connect(this, &ScopyCodeEditor::updateRequest, this, &ScopyCodeEditor::updateLineNumberArea);
	connect(this, &ScopyCodeEditor::cursorPositionChanged, this, &ScopyCodeEditor::highlightCurrentLine);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}

void ScopyCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
	int bottom = top + qRound(blockBoundingRect(block).height());
	while(block.isValid() && top <= event->rect().bottom()) {
		if(block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight,
					 number);
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound(blockBoundingRect(block).height());
		++blockNumber;
	}
}

int ScopyCodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while(max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return space;
}

void ScopyCodeEditor::resizeEvent(QResizeEvent *event)
{
	QPlainTextEdit::resizeEvent(event);
	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScopyCodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScopyCodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if(!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = Style::getColor(json::theme::interactive_subtle_hover);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void ScopyCodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if(dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if(rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}
