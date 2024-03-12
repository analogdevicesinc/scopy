#ifndef FAULTS_H
#define FAULTS_H

#include "scopy-swiotrefactor_export.h"
#include <QWidget>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT Faults : public QWidget
{
	Q_OBJECT
public:
	Faults(QWidget *parent = nullptr);
	~Faults();
};
} // namespace scopy::swiotrefactor
#endif // FAULTS_H
