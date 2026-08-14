#pragma once
#include "sv_qtcommon.h"
#include "WidgetLogic/WidgetDefs.h"

// todo ! make em qpointer bro

//***********************************************************************************
// So, when we make widgets for a data tree, we typically operate on
// a list of top-level widgets.
// 
//  Lets say, your data is:
//  {
//      struct A with 500 inner fields and widgets for them;
//      struct B;
//      int C; + uimacrostring "tab=2"
//  }
//
// Your top level widgets will be only these three: widget for A, widget for B, widget for C.
// You will be able to reorder these three visually (drag and drop between tabs)
// You will not be able to drag and drop any other widgets that live within these three.
// 
// class TopLevelWidgetsContainer:
//      - Holds this list of top level widgets (visual reordering doesnt change the order)
//      - Puts these widgets into appropriate tabs (class TabOfTopLevelWidgets) 
//        Every top-level widget may also contain 1-based tab index representing
//        which tab it should get added at. By default, everything goes to first tab.
// 
//***********************************************************************************

class TabOfTopLevelWidgets;
class TopLevelWidgetsContainer : public QWidget
{
    Q_OBJECT
public:
    TopLevelWidgetsContainer(QWidget* parent = nullptr);

    enum ExistingWidgetsPolicy
    {
        Delete,
        UnparentButKeepAlive
    };

    void setTopLevelWidgets(NodeWidgetQPointerVec&& newTopLevelWidgets, 
                            ExistingWidgetsPolicy   existingWidgetsPolicy = ExistingWidgetsPolicy::Delete);

    
    void clearEverything(ExistingWidgetsPolicy existingWidgetsPolicy = ExistingWidgetsPolicy::Delete);

    void addTab();
    int tabsCount();
    TabOfTopLevelWidgets* getTab(int index);

private:
    void assignTabName(TabOfTopLevelWidgets* tab, int index);
    void reassignTabsNames();
    void reassignTabIndexes();
    void onTabDeleteRequested(TabOfTopLevelWidgets* tab);
    void deleteTabAndMoveWidgetsToOther(int index);
    int tabIndex(const TabOfTopLevelWidgets* tab); //-1 if not found
    TabOfTopLevelWidgets* getOrMakeTab(int index);

private:
    QVBoxLayout*    layout           = nullptr;
    QScrollArea*    tabsScrollArea   = nullptr;
    QHBoxLayout*        tabsLayout   = nullptr; //all TabOfTopLevelWidgets belong here
    QPushButton*    addTabButton     = nullptr;

private:
    NodeWidgetQPointerVec topLevelWidgets;
};

//******************************************************
// The most primitive holder of those top-level widgets:
// simply keeps them in a vertical layout.
// 
// As of now it only holds NodeWidget*'s, but im not 
// hardcoding it in this class
//******************************************************
class TabOfTopLevelWidgets : public QWidget
{
    Q_OBJECT
public:
    TabOfTopLevelWidgets(QWidget* parent = nullptr);

    void extractAllWidgets(QList<QWidget*> *outWidgets = nullptr); //removes them from layout and unparents

    void visitAllWidgets(const std::function<void(QWidget*)>& visitor);

    void addWidget(QWidget *item);

    void setName(const QString& name);

signals:
    void requestedTabDelete();

private:
    QVBoxLayout*    layout                  = nullptr;
    QWidget*            topStripe           = nullptr;
    QLabel*                 tabName         = nullptr;
    QPushButton*            deleteTabButton = nullptr;
    QScrollArea*        contentScrollArea   = nullptr;
    QVBoxLayout*            contentLayout   = nullptr;
};