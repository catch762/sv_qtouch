#pragma once
#include "sv_qtcommon.h"
#include "WidgetLogic/WidgetDefs.h"

//***********************************************************************************
// So, when we make widgets for a data tree, we typically operate on
// a list of top-level widgets.
// 
//  Lets say, your data is:
//  {
//      struct A with 500 inner fields and widgets for them;
//      struct B;
//      int C;
//  }
//
// Your top level widgets will be only these three: widget for A, widget for B, widget for C.
// You will be able to reorder these three visually (drag and drop between tabs)
// You will not be able to drag and drop any other widgets that live within these three.
// 
// class TopLevelWidgetsContainer:
//      - Holds this list of top level widgets (visual reordering doesnt change the order)
//      - Puts these widgets into tabs (class TabOfTopLevelWidgets) 
//***********************************************************************************

class TabOfTopLevelWidgets;
class TopLevelWidgetsContainer : public QWidget
{
    Q_OBJECT
public:
    TopLevelWidgetsContainer(QWidget* parent = nullptr);

    void setTopLevelWidgets(NodeWidgetVec&& newTopLevelWidgets);
    void deleteAllTopLevelWidgets();

    void addTab();
    int tabsCount();
    TabOfTopLevelWidgets* getTab(int index);

private:
    void assignTabName(TabOfTopLevelWidgets* tab, int index);
    void reassignTabsNames();
    void onTabDeleteRequested(TabOfTopLevelWidgets* tab);
    void deleteTabAndMoveWidgetsToOther(int index);
    int tabIndex(const TabOfTopLevelWidgets* tab); //-1 if not found
    TabOfTopLevelWidgets* getOrMakeDefaultTab();

private:
    QVBoxLayout*    layout           = nullptr;
    QScrollArea*    tabsScrollArea   = nullptr;
    QHBoxLayout*        tabsLayout   = nullptr; //all TabOfTopLevelWidgets belong here
    QPushButton*    addTabButton     = nullptr;

private:
    NodeWidgetVec topLevelWidgets;
};

//******************************************************
// The most primitive holder of those top-level widgets:
// simply keeps them in a vertical layout.
//******************************************************
class TabOfTopLevelWidgets : public QWidget
{
    Q_OBJECT
public:
    TabOfTopLevelWidgets(QWidget* parent = nullptr);

    void extractAllWidgets(QList<QWidget*> &outWidgets);
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