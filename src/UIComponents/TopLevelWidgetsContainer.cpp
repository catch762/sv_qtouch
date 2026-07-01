#include "TopLevelWidgetsContainer.h"
#include "WidgetLogic/NodeWidget.h"

TopLevelWidgetsContainer::TopLevelWidgetsContainer(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout);

    {
        tabsScrollArea = new QScrollArea(this);
        tabsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tabsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tabsScrollArea->setWidgetResizable(true);
        
        auto tabsAreaWidget = new QWidget(tabsScrollArea);
        tabsScrollArea->setWidget(tabsAreaWidget);

        tabsLayout = new QHBoxLayout(tabsAreaWidget);
        tabsLayout->setAlignment(Qt::AlignLeft);
        initLayoutSpacing(tabsLayout, 0, 4);

        layout->addWidget(tabsScrollArea);
    }

    {
        addTabButton = new QPushButton("Add tab");
        connect(addTabButton, &QPushButton::clicked, this, &TopLevelWidgetsContainer::addTab);

        layout->addWidget(addTabButton);

    }
}

void TopLevelWidgetsContainer::addTab()
{
    auto tab = new TabOfTopLevelWidgets(this);
    assignTabName(tab, tabsCount());

    connect(tab, &TabOfTopLevelWidgets::requestedTabDelete, this, [this, tab]()
    {
        onTabDeleteRequested(tab);
    });

    tabsLayout->addWidget(tab);
}

int TopLevelWidgetsContainer::tabsCount()
{
    return tabsLayout->count();
}

TabOfTopLevelWidgets *TopLevelWidgetsContainer::getTab(int index)
{
    auto* item = tabsLayout->itemAt(index);
    if (!item) return nullptr;

    auto* widget = item->widget();
    if (!widget) return nullptr;

    return dynamic_cast<TabOfTopLevelWidgets*>(widget); //there cant be anything else, but just in case.
}

void TopLevelWidgetsContainer::setTopLevelWidgets(NodeWidgetVec &&newTopLevelWidgets)
{
    SV_LOG(std::format("Setting {} widgets", newTopLevelWidgets.size()));

    deleteAllTopLevelWidgets();
    topLevelWidgets = std::move(newTopLevelWidgets);

    auto tab = getOrMakeDefaultTab();
    if (!tab)
    {
        SV_MSGBOX_ERROR("Cant make default tab");
        return;
    }

    for(auto widget : topLevelWidgets)
    {
        if (widget)
        {
            SV_LOG("Adding widget to tab...");
            widget->show();
            tab->addWidget(widget);
        }
        else SV_ERROR("Null widget in topLevelWidgets list !");
    }
}

void TopLevelWidgetsContainer::deleteAllTopLevelWidgets()
{
    SV_LOG("TopLevelWidgetsContainer::deleteAllTopLevelWidgets()");

    for (auto widget : topLevelWidgets)
    {
        if (widget)
        {
            //if i just write 'delete widget;' it crashes and i dont fucking understand why.
            //it should be perfectly fine either way, but its not.
            
            //these two are not needed, apparently
                //widget->setParent(nullptr);
                //widget->hide();
            widget->deleteLater();
        }
    }

    topLevelWidgets.clear();
}

void TopLevelWidgetsContainer::assignTabName(TabOfTopLevelWidgets *tab, int index)
{
    bool isOneLetterName = index >= 0 && index <= ('Z'-'A');

    auto name = isOneLetterName ? QString(QChar('A' + index)) :
                                  QString("Tab %1").arg(index);

    tab->setName(name);
}

void TopLevelWidgetsContainer::reassignTabsNames()
{
    for (int i = 0; i < tabsCount(); ++i)
    {
        if (auto tab = getTab(i))
        {
            assignTabName(tab, i);
        }
    }
}

void TopLevelWidgetsContainer::onTabDeleteRequested(TabOfTopLevelWidgets* tab)
{
    auto index = tabIndex(tab);
    if (index == -1)
    {
        SV_ERROR("Cant delete tab which is not even in tab list");
        return;
    }
    deleteTabAndMoveWidgetsToOther(index);
}

void TopLevelWidgetsContainer::deleteTabAndMoveWidgetsToOther(int index)
{
    if (tabsCount() == 1)
    {
        SV_MSGBOX_WARN("Cant delete last tab!");
        return;
    }

    if (index < 0 || index >= tabsCount())
    {
        SV_ERROR(std::format("Cant delete tab with invalid index {}, tab count is {}", index, tabsCount()));
        return;
    }

    auto* tab = getTab(index);
    if (!tab)
    {
        SV_ERROR(std::format("Cant delete tab, couldnt even obtain it from index {} for some reason", index));
        return;
    }

    QList<QWidget*> extractedWidgets;
    tab->extractAllWidgets(extractedWidgets);
    tab->deleteLater();

    if (!extractedWidgets.empty())
    {
        int tabThatInheritsIndex = tabsCount()-1;
        TabOfTopLevelWidgets* tabThatInherits = getTab(tabThatInheritsIndex);
        if (!tabThatInherits)
        {
            SV_MSGBOX_ERROR(std::format("Couldnt obtain tab {} for some reason. Dont know what to do with widgets.", tabThatInheritsIndex));
            return;
        }

        for (auto widgetToMove : extractedWidgets)
        {
            tabThatInherits->addWidget(widgetToMove);
        }
    }

    reassignTabsNames();

    SV_LOG(std::format("Deleted tab and moved {} widgets to another.", extractedWidgets.size()));
}

int TopLevelWidgetsContainer::tabIndex(const TabOfTopLevelWidgets *tab)
{
    for (int i = 0; i < tabsCount(); ++i)
    {
        if (getTab(i) == tab) return i;
    }

    return -1;
}

TabOfTopLevelWidgets *TopLevelWidgetsContainer::getOrMakeDefaultTab()
{
    if (tabsCount() == 0)
    {
        addTab();
    }

    return getTab(tabsCount()-1);
}




//******************************//
//                              //
// class TabOfTopLevelWidgets:  //
//                              //
//******************************//

namespace{
    const auto WidgetTab_TopStripe_Height = 20;
    const auto WidgetTab_TopStripe_Margin = 2;
    const auto WidgetTab_TopStripe_ContentHeight = WidgetTab_TopStripe_Height - 2 * WidgetTab_TopStripe_Margin;
};

TabOfTopLevelWidgets::TabOfTopLevelWidgets(QWidget *parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout);

    //1. top stripe and its contents
    {
        topStripe = new QWidget(this);
        topStripe->setStyleSheet("QWidget {background-color: rgb(234, 234, 234); border: none; border-radius: 0; padding: 0; margin: 0;}");
        topStripe->setFixedHeight(WidgetTab_TopStripe_Height);

        auto topStripeLayout = new QHBoxLayout(topStripe);
        initLayoutSpacing(topStripeLayout, WidgetTab_TopStripe_Margin, 4);

        tabName = new QLabel(topStripe);
        tabName->setFixedHeight(WidgetTab_TopStripe_ContentHeight);
        topStripeLayout->addWidget(tabName);

        deleteTabButton = new QPushButton("X", topStripe);
        deleteTabButton->setFlat(true);
        deleteTabButton->setFixedSize(WidgetTab_TopStripe_ContentHeight,
                                      WidgetTab_TopStripe_ContentHeight);
        connect(deleteTabButton, &QPushButton::clicked, this, &TabOfTopLevelWidgets::requestedTabDelete);
        topStripeLayout->addWidget(deleteTabButton);

        layout->addWidget(topStripe);
    }

    //2. scroll area containing content widgets
    {
        contentScrollArea = new QScrollArea(this);
        contentScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        contentScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        contentScrollArea->setWidgetResizable(true);
        
        auto contentAreaWidget = new QWidget(contentScrollArea);
        contentScrollArea->setWidget(contentAreaWidget);

        contentLayout = new QVBoxLayout(contentAreaWidget);
        initLayoutSpacing(contentLayout);

        layout->addWidget(contentScrollArea);
    }

    setFixedWidth(400);
    setMinimumHeight(100);
}

void TabOfTopLevelWidgets::extractAllWidgets(QList<QWidget *> &outWidgets)
{
    extractAllWidgetsFromLayoutAndDeleteNestedLayouts(contentLayout, &outWidgets);
}

void TabOfTopLevelWidgets::addWidget(QWidget *item)
{
    if (contentLayout->count() == 0)
    {
        contentLayout->addStretch(1);
    }

    contentLayout->insertWidget(contentLayout->count() - 1, item, Qt::AlignTop);

    //printLayoutContents(contentLayout);
}

void TabOfTopLevelWidgets::setName(const QString &name)
{
    tabName->setText(name);
}

