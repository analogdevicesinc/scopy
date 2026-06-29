#pragma once

#include "scopy-core_export.h"
#include "GenalyzerConfig.h"

#include <QWidget>

class QSpinBox;
class QComboBox;
class QLineEdit;

namespace scopy {
namespace acq {

class SCOPY_CORE_EXPORT GenalyzerSettings : public QWidget
{
	Q_OBJECT
public:
	explicit GenalyzerSettings(QWidget *parent = nullptr);
	~GenalyzerSettings() override;

	GenalyzerConfig getConfig() const;
	void            setConfig(const GenalyzerConfig &config);
	void            enableAnalysis(bool en);

Q_SIGNALS:
	void configChanged(const scopy::acq::GenalyzerConfig &config);

private Q_SLOTS:
	void onUIChanged();
	void updateUIFromConfig();

private:
	void setupUI();

	QComboBox *m_modeCombo{nullptr};
	QSpinBox  *m_ssbWidthSpinbox{nullptr};
	QWidget   *m_autoModeContainer{nullptr};
	QWidget   *m_fixedToneContainer{nullptr};

	QLineEdit *m_expectedFreqEdit{nullptr};
	QSpinBox  *m_harmonicOrderSpinbox{nullptr};
	QSpinBox  *m_ssbFundamentalSpinbox{nullptr};
	QSpinBox  *m_ssbDefaultSpinbox{nullptr};

	GenalyzerConfig m_config;
};

} // namespace acq
} // namespace scopy
