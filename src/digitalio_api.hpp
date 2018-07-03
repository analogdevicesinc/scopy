#ifndef DIGITALIO_API_HPP
#define DIGITALIO_API_HPP

#include "digitalio.hpp"

namespace adiscope {
class DigitalIO_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(QList<bool> group READ grouped WRITE setGrouped SCRIPTABLE false);
	Q_PROPERTY(QList<bool> dir READ direction WRITE setDirection SCRIPTABLE true);
	Q_PROPERTY(QList<bool> out READ output    WRITE setOutput SCRIPTABLE true);

	Q_PROPERTY(QList<bool> gpi READ gpi STORED false);
	Q_PROPERTY(QList<bool> locked READ locked STORED false);
	Q_PROPERTY(bool running READ running WRITE run STORED false);

public:
	explicit DigitalIO_API(DigitalIO *dio) : ApiObject(), dio(dio) {}
	~DigitalIO_API() {}

	QList<bool> direction() const;
	void setDirection(const QList<bool>& list);
	QList<bool> output() const;
	void setOutput(const QList<bool>& list);
	void setOutput(int ch, int direction);
	QList<bool> grouped() const;
	void setGrouped(const QList<bool>& grouped);

	QList<bool> gpi() const;
	QList<bool> locked() const;
	bool running() const;
	void run(bool en);

	Q_INVOKABLE void show();

private:
	DigitalIO *dio;
};
}

#endif // DIGITALIO_API_HPP
