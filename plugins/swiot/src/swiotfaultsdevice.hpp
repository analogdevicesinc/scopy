#ifndef SCOPY_SWIOTFAULTSDEVICE_HPP
#define SCOPY_SWIOTFAULTSDEVICE_HPP

#include <QWidget>
#include <QTextEdit>
#include "ui_faultsdevice.h"
#include "faultsgroup.hpp"
#include "gui/subsection_separator.hpp"


namespace adiscope::gui {
class FaultsGroup;

class FaultsDevice : public QWidget {
	Q_OBJECT
public:
	explicit FaultsDevice(QString name, QString path, QWidget* parent);
	~FaultsDevice() override;

	void update(uint32_t faults_numeric);

public Q_SLOTS:
	void resetStored();
	void updateExplanations();

private:
	Ui::FaultsDevice *ui;
	QTextEdit *m_faults_explanation;
	SubsectionSeparator *m_subsectionSeparator;

	FaultsGroup* m_faultsGroup;

	QString m_name;
	QString m_path;

	void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
	void updateMinimumHeight();
};

} // adiscope::gui

#endif //SCOPY_SWIOTFAULTSDEVICE_HPP
