#include "ComboBoxLineEdit.h"

using namespace adiscope;

ComboBoxLineEdit::ComboBoxLineEdit(QWidget *parent) : QLineEdit(parent) {}

void ComboBoxLineEdit::mouseReleaseEvent(QMouseEvent *event) { QWidget::mouseReleaseEvent(event); }

void ComboBoxLineEdit::mousePressEvent(QMouseEvent *event) { QWidget::mousePressEvent(event); }

void ComboBoxLineEdit::enterEvent(QEvent *event) { QWidget::enterEvent(event); }

void ComboBoxLineEdit::leaveEvent(QEvent *event) { QWidget::leaveEvent(event); }
