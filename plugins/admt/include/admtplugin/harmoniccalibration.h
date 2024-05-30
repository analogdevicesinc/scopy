#ifndef HARMONICCALIBRATION_H
#define HARMONICCALIBRATION_H

#include "scopy-admtplugin_export.h"
#include <QWidget>

namespace scopy::admt {
class SCOPY_ADMTPLUGIN_EXPORT HarmonicCalibration : public QWidget
{
	Q_OBJECT
public:
	HarmonicCalibration(QWidget *parent = nullptr);
	~HarmonicCalibration();
};
} // namespace scopy::admt
#endif // HARMONICCALIBRATION_H
