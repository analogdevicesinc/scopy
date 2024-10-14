#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include "widgets/hoverwidget.h"
#include <QMap>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QTableWidget>
#include <QSplitter>
#include <QTreeView>

#include <scopy-gui_export.h>
#include <utils.h>

#include <QVBoxLayout>
#include <QComboBox>
#include <QListWidget>

// Forward declarations
namespace scopy {
class SmallOnOffSwitch;
class CustomSwitch;
class SpinBoxA;
class MeasurementLabel;
class StatsLabel;
class MeasurementSelectorItem;
class TitleSpinBox;
} // namespace scopy

namespace scopy {
class SCOPY_GUI_EXPORT StyleHelper : public QObject
{
	Q_OBJECT
protected:
	StyleHelper(QObject *parent = nullptr);
	~StyleHelper();

public:
	// singleton
	StyleHelper(StyleHelper &other) = delete;
	void operator=(const StyleHelper &) = delete;
	static StyleHelper *GetInstance();

public:
	static void initColorMap();
	static QString getColor(QString id);
	static QString getChannelColor(int index);
	static void SquareToggleButtonWithIcon(QPushButton *btn, QString objectName, bool checkable = false);
	static void BasicButton(QPushButton *btn, QString objectName = "");
	static void RefreshButton(QPushButton *btn, QString objectName = "");
	static void BasicSmallButton(QPushButton *btn, QString objectName = "");
	static void CollapseCheckbox(QCheckBox *chk, QString objectName = "");
	static void ColoredCircleCheckbox(QCheckBox *chk, QColor color, QString objectName = "");
	static void ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName = "");
	static void MenuMediumLabel(QLabel *lbl, QString objectName = "");
	static void MenuControlButton(QPushButton *btn, QString objectName = "", bool checkable = true);
	static void MenuControlWidget(QWidget *w, QColor color, QString objectName = "");
	static void MenuLargeLabel(QLabel *lbl, QString objectName = "");
	static void MenuHeaderLine(QFrame *line, QPen pen, QString objectName = "");
	static void MenuHeaderWidget(QWidget *w, QString objectName = "");
	static void MenuEditTextHeaderWidget(QWidget *w, QString objectName = "");
	static void MenuSmallLabel(QLabel *lbl, QString objectName = "");
	static void MenuComboWidget(QWidget *w, QString objectName = "");
	static void MenuSectionWidget(QWidget *w, QString objectName = "");
	static void MenuSpinComboBox(QComboBox *w, QString objectName = "");
	static void MenuOnOffSwitch(QWidget *w, QString objectName = "");
	static void MenuOnOffSwitchLabel(QLabel *w, QString objectName = "");
	static void MenuOnOffSwitchButton(SmallOnOffSwitch *w, QString objectName = "");
	static void MenuCollapseSection(QWidget *w, QString objectName = "");
	static void MenuCollapseHeaderLineEdit(QLineEdit *w, QString objectName = "");
	static void MenuComboLabel(QLabel *w, QString objectName = "");
	static void MenuHeaderLabel(QLabel *w, QString objectName = "");
	static void MenuBigSwitch(QWidget *w, QString objectName = "");
	static void MenuBigSwitchButton(CustomSwitch *w, QString objectName = "");
	static void MenuLineEdit(QLineEdit *m_edit, QString objectName = "");
	static void MenuLineEditWidget(QWidget *w, QString objectName = "");
	static void BlueGrayButton(QPushButton *btn, QString objectName = "");
	static void OverlayMenu(QWidget *w, QString objectName = "");
	static void BlueIconButton(QPushButton *w, QIcon icon, QString objectName = "");
	static void TabWidgetEastMenu(QTabWidget *w, QString objectName = "");
	static void BackgroundPage(QWidget *w, QString objectName = "");
	static void BackgroundWidget(QWidget *w, QString objectName = "");
	static void TabWidgetLabel(QLabel *w, QString objectName = "");
	static void TutorialChapterTitleLabel(QLabel *w, QString objectName = "");
	static void MeasurementPanelLabel(MeasurementLabel *w, QString objectName = "");
	static void StatsPanelLabel(StatsLabel *w, QString objectName = "");
	static void MeasurementSelectorItemWidget(QString iconPath, MeasurementSelectorItem *w,
						  QString objectName = "");
	static void FrameBackgroundShadow(QFrame *w, QString objectName = "");
	static void HoverWidget(QWidget *w, bool draggable = false, QString objectName = "");
	static void TransparentWidget(QWidget *w, QString objectName = "");
	static void ScopyStatusBar(QWidget *w, QString objectName = "");
	static void IIOWidget(QWidget *w, QString objectName = "");
	static void IIOWidgetElement(QWidget *w, QString objectName = "");
	static void IIOLineEdit(QLineEdit *w, QString objectName = "");
	static void TableViewWidget(QWidget *w, QString objectName = "");
	static void HoverToolTip(QWidget *w, QString info, QString objectName = "");
	static void WarningLabel(QLabel *w, QString objectName = "");
	static void NoBackgroundIconButton(QPushButton *w, QIcon icon, QString objectName = "");
	static void BackgroundAddPage(QWidget *w, QString objectName = "");
	static void BrowseButton(QPushButton *btn, QString objectName = "");
	static void SpinBoxUpButton(QPushButton *w, QString objectName = "");
	static void SpinBoxDownButton(QPushButton *w, QString objectName = "");
	static void MenuSpinboxLabel(QLabel *m_label, QString objectName = "");
	static void TabWidgetBarUnderline(QTabWidget *w, QString objectName = "");
	static void TableWidgetDebugger(QTableWidget *w, QString objectName = "");
	static void SplitterStyle(QSplitter *w, QString objectName = "");
	static void TreeViewDebugger(QTreeView *w, QString objectName = "");
	static void OrangeWidget(QWidget *w, QString objectName = "");
	static void ActiveStoredLabel(QLabel *w, QString objectName = "");
	static void FaultsFrame(QFrame *w, QString objectName = "");
	static void FaultsExplanation(QWidget *w, QString objectName = "");
	static void IIOCompactLabel(QLabel *label, QString objectName = "");
	static void GrayButton(QPushButton *btn, QString objectName = "");

	static void RoundedCornersWidget(QWidget *w, QString objectName = "");

private:
	QMap<QString, QString> colorMap;
	static StyleHelper *pinstance_;
};
} // namespace scopy

#endif // STYLEHELPER_H
