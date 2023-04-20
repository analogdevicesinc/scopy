#ifndef FAULTSGROUP_HPP
#define FAULTSGROUP_HPP

#include <set>

#include "faultwidget.hpp"
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

	const std::vector<FaultWidget *> &getFaults() const;

	const QString &getName() const;
	void setName(const QString &name);

	void clearSelection();
	void update(uint32_t faults_numeric);
	QString getExplanations();

Q_SIGNALS:
	void selectionUpdated();
	void minimumSizeChanged();

private:
	Ui::FaultsGroup *ui;
	QString m_name;
	std::vector<FaultWidget *> m_faults;
	std::set<unsigned int> m_currentlySelected;
	int m_max_faults;
	FlexGridLayout* m_customColGrid;

protected:
	void setupDynamicUi();
	QWidget* buildActiveStoredWidget();
};
}

#endif // FAULTSGROUP_HPP
