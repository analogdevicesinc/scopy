#ifndef FAULTSGROUP_H
#define FAULTSGROUP_H

#include <set>

#include <QVector>

#include "faultwidget.h"
#include <gui/flexgridlayout.hpp>
#include "ui_faultsgroup.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FaultsGroup; }
QT_END_NAMESPACE

namespace scopy::swiot {
class FaultsGroup : public QWidget {
	Q_OBJECT
public:
	explicit FaultsGroup(QString name, const QString& path, QWidget *parent = nullptr);
	~FaultsGroup() override;

	const QVector<FaultWidget *> &getFaults() const;

	const QString &getName() const;
	void setName(const QString &name);

	void clearSelection();
	void update(uint32_t faults_numeric);
	QStringList getExplanations();
	std::set<unsigned int> getSelectedIndexes();
	std::set<unsigned int> getActiveIndexes();

Q_SIGNALS:
	void selectionUpdated();
	void minimumSizeChanged();

private:
	Ui::FaultsGroup *ui;
	QString m_name;
	QVector<FaultWidget *> m_faults;
	std::set<unsigned int> m_currentlySelected;
	std::set<unsigned int> m_actives;
	int m_max_faults;
	FlexGridLayout* m_customColGrid;

	void resizeEvent(QResizeEvent *event) override;

protected:
	void setupDynamicUi();
	QWidget* buildActiveStoredWidget();
	QJsonArray* getJsonArray(const QString& path);
};
}

#endif // FAULTSGROUP_H
