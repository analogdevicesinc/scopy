#ifndef SETTINGSINSTRUMENT_H
#define SETTINGSINSTRUMENT_H

#include "qcombobox.h"
#include <QWidget>
#include <QMap>

#define DEVICE_NAME "pqm"
namespace scopy::pqm {
class SettingsInstrument : public QWidget
{
	Q_OBJECT
public:
	SettingsInstrument(QWidget *parent = nullptr);
	~SettingsInstrument();

public Q_SLOTS:
	void attributeAvailable(QMap<QString, QMap<QString, QString>> configAttr);
Q_SIGNALS:
	void updateUi();
	void setAttributes(QMap<QString, QMap<QString, QString>> configAttr);
	void enableTool(bool en, QString tool = "settings");

private Q_SLOTS:
	void onSetBtnPressed();
	void onReadBtnPressed(bool en);

private:
	void initConfigSection(QWidget *parent);
	void initSystemTimeSection(QWidget *parent);
	void initTimestampSection(QWidget *parent);
	void initCalibSection(QWidget *parent);
	QWidget *createMenuCombo(QString name, QString attr);
	QWidget *createConfigEdit(QString name, QString attr);
	void updateCbValues(QComboBox *cb, QString attr);

	bool m_readEnabled;
	QMap<QString, QMap<QString, QString>> m_pqmAttr;
};
} // namespace scopy::pqm

#endif // SETTINGSINSTRUMENT_H
