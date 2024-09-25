#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "scopy-dac_export.h"
#include <QWidget>
#include <QPushButton>

#include <gui/widgets/progresslineedit.h>

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT FileBrowser : public QWidget
{
	Q_OBJECT
public:
	explicit FileBrowser(QWidget *parent = nullptr);
	~FileBrowser();
	QString getFilePath() const;
	void setDefaultDir(QString dir);
Q_SIGNALS:
	void load(QString path);
private Q_SLOTS:
	void chooseFile();
	void loadFile();

private:
	ProgressLineEdit *m_fileBufferPath;
	QPushButton *m_fileBufferBrowseBtn;
	QPushButton *m_fileBufferLoadBtn;
	QString m_filename;
	QString m_defaultDir;
};
} // namespace dac
} // namespace scopy

#endif // FILEBROWSER_H
