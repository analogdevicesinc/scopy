#ifndef DEBUGGERLOGGINGCATEGORIES_H
#define DEBUGGERLOGGINGCATEGORIES_H

#include <QLoggingCategory>

#ifndef QT_NO_DEBUG_OUTPUT
// Used fo the tree view visual part
Q_DECLARE_LOGGING_CATEGORY(CAT_TREEVIEW)

// Used for the details view visual part
Q_DECLARE_LOGGING_CATEGORY(CAT_DETAILSVIEW)

// Used for the watch list view visual part
Q_DECLARE_LOGGING_CATEGORY(CAT_WATCHLIST)

// Used for the creation / access of the internal model or any internal configurations
Q_DECLARE_LOGGING_CATEGORY(CAT_DEBUGGERIIOMODEL)
#else
#define CAT_TREEVIEW
#define CAT_DETAILSVIEW
#define CAT_WATCHLIST
#define CAT_DEBUGGERIIOMODEL
#endif

#endif // DEBUGGERLOGGINGCATEGORIES_H
