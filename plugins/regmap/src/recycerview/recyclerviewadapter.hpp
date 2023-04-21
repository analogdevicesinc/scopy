#ifndef RECYCLERVIEWADAPTER_HPP
#define RECYCLERVIEWADAPTER_HPP

#include "irecyclerviewadapter.hpp"

#include <QMap>
#include <QObject>

class RegisterSimpleWidget;
class RegisterModel;
class RecyclerView;
class RecyclerViewAdapter : public IRecyclerViewAdapter
{
    Q_OBJECT
public:
    RecyclerViewAdapter(QMap<uint32_t, RegisterModel*> *registerModels);

    QWidget* getWidget();
    void setFilters(QList<uint32_t> filters);
    void valueUpdated(uint32_t address, uint32_t value);

    // IRecyclerViewAdapter interface
    void generateWidget(int index);

Q_SIGNALS:
    void requestWidget(int index);
    void registerSelected(uint32_t address);

private:
    RecyclerView * recyclerView;
    QMap<uint32_t, RegisterModel*> *registerModels;
    QMap<uint32_t, RegisterSimpleWidget*> *registersMap;
};

#endif // RECYCLERVIEWADAPTER_HPP
