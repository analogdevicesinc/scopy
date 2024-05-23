#ifndef IIODEBUGLOGGER_H
#define IIODEBUGLOGGER_H

#include <QWidget>
#include <QDialog>
#include <QTextBrowser>

namespace scopy::iiodebugplugin {
class IIODebugLogger : public QFrame
{
	Q_OBJECT
public:
	explicit IIODebugLogger(QWidget *parent = nullptr);
	void setupUi();

public Q_SLOTS:
	void appendLog(QString log);

private:
	QTextBrowser *m_textBrowser;
};
} // namespace scopy::iiodebugplugin

#endif // IIODEBUGLOGGER_H
