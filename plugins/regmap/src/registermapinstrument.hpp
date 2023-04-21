#ifndef REGISTERMAPINSTRUMENT_HPP
#define REGISTERMAPINSTRUMENT_HPP

#include <QWidget>
#include "scopyregmap_export.h"

class QVBoxLayout;
class QTabWidget;

class SCOPYREGMAP_EXPORT RegisterMapInstrument : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterMapInstrument(QWidget *parent = nullptr);
    ~RegisterMapInstrument();
    void addTab(QWidget *widget, QString title);
    void addTab(struct iio_device *dev, QString title);
    void addTab(QString filePath, QString title);
signals:

private:
    QVBoxLayout *layout;
    QTabWidget *tabWidget;
};

#endif // REGISTERMAPINSTRUMENT_HPP
