#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <QWidget>
#include <analysismenu.h>
#include <buffermenu.h>

namespace scopy::qiqplugin {

class SettingsMenu : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsMenu(QWidget *parent = nullptr);

	void setAvailableChannels(const QMap<QString, QStringList> &channels);
	void setAnalysisTypes(const QStringList &types);
	void setAnalysisParams(const QString &type, const QVariantMap &params);

Q_SIGNALS:
	void analysisChanged(const QString &type);
	void analysisConfig(const QString &type, const QVariantMap &inConfig);
	void bufferParamsChanged(const BufferParams &params);

private Q_SLOTS:
	void onAnalysisApply();

private:
	BufferMenu *m_bufferMenu;
	AnalysisMenu *m_analysisMenu;
	MenuCombo *m_analysisCb;

	void setupUI();
};
} // namespace scopy::qiqplugin
#endif // SETTINGSMENU_H
