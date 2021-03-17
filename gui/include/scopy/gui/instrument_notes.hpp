#ifndef INSTRUMENTNOTES_H
#define INSTRUMENTNOTES_H

#include <QWidget>

namespace Ui {
class InstrumentNotes;
}
namespace scopy {
namespace gui {

class InstrumentNotes : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentNotes(QWidget* parent = nullptr);
	~InstrumentNotes();

public Q_SLOTS:
	QString getNotes();
	void setNotes(const QString&);

private:
	Ui::InstrumentNotes* m_ui;
};
} // namespace gui
} // namespace scopy

#endif // INSTRUMENTNOTES_H
