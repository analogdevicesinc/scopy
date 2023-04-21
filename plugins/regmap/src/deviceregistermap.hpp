#ifndef DEVICEREGISTERMAP_HPP
#define DEVICEREGISTERMAP_HPP

#include <QMap>
#include <QObject>
#include <QWidget>
#include "scopyregmap_export.h"

class RecyclerViewAdapter;
class QVBoxLayout;
class RegisterModel;
class RegisterDetailedWidget;
class RegisterController;
class RegisterMapValues;
class RegisterMapTemplate;
class SearchBarWidget;
//class RegisterMapTable;

class SCOPYREGMAP_EXPORT DeviceRegisterMap : public QWidget
{
	Q_OBJECT
public:
	explicit DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate = nullptr, RegisterMapValues *registerMapValues = nullptr,  QWidget *parent = nullptr);
	~DeviceRegisterMap();

	void registerChanged(RegisterModel *regModel);

private:
	QVBoxLayout *layout;
	RegisterMapTemplate *registerMapTemplate ;
	RegisterMapValues *registerMapValues;
	RegisterController *registerController = nullptr;

	SearchBarWidget *searchBarWidget = nullptr;
//	RegisterMapTable *registerMapTableWidget = nullptr;
    RecyclerViewAdapter *recycerViewAdapter = nullptr;

	RegisterDetailedWidget *registerDetailedWidget = nullptr;
Q_SIGNALS:

};

#endif // DEVICEREGISTERMAP_HPP
