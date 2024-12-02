#ifndef INSTRUMENTNOTES_H
#define INSTRUMENTNOTES_H

#include <QWidget>
#include "scopy-gui_export.h"

namespace Ui {
class InstrumentNotes;
}
namespace scopy {
class SCOPY_GUI_EXPORT InstrumentNotes : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentNotes(QWidget *parent = nullptr);
	~InstrumentNotes();

public Q_SLOTS:
	QString getNotes();
	void setNotes(QString);

private:
	Ui::InstrumentNotes *ui;
};
}

#endif // INSTRUMENTNOTES_H
