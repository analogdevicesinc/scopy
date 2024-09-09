#include <QWidget>
#include <QPushButton>
#include <QList>

namespace scopy {
class PathTitle : public QWidget
{
	Q_OBJECT
public:
	PathTitle(QWidget *parent = nullptr);
	PathTitle(QString title, QWidget *parent = nullptr);

	void setTitle(QString title);
	QString title() const;

Q_SIGNALS:
	void pathSelected(QString path);

private:
	void setupUi();
	QString m_titlePath;
};
} // namespace scopy
