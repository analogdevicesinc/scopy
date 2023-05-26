#ifndef REGISTERMAPTABLE_H
#define REGISTERMAPTABLE_H

#include "irecyclerviewadapter.hpp"

#include <QMap>
#include <QObject>

class RegisterSimpleWidget;
class RegisterModel;
class RecyclerView;
class RegisterMapTable : public IRecyclerViewAdapter
{
    Q_OBJECT
public:
    RegisterMapTable(QMap<uint32_t, RegisterModel*> *registerModels);

    QWidget* getWidget();
    void setFilters(QList<uint32_t> filters);
    void valueUpdated(uint32_t address, uint32_t value);
    void scrollTo(uint32_t index);
    void setRegisterSelected(uint32_t address, bool selected);

    // IRecyclerViewAdapter interface
    void generateWidget(int index);

Q_SIGNALS:
    void requestWidget(int index);
    void registerSelected(uint32_t address);

private:
    RecyclerView * recyclerView;
    QMap<uint32_t, RegisterModel*> *registerModels;
    QMap<uint32_t, RegisterSimpleWidget*> *registersMap;
    uint32_t selectedAddress = 0;
};

#endif // REGISTERMAPTABLE_H
