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

void TopLevelWidgetsContainer::setTopLevelWidgets(NodeWidgetQPointerVec &&newTopLevelWidgets, ExistingWidgetsPolicy existingWidgetsPolicy)
{
    SV_LOG(std::format("Setting {} widgets", newTopLevelWidgets.size()));

    clearEverything(existingWidgetsPolicy);

    topLevelWidgets = std::move(newTopLevelWidgets);

    for(auto widget : topLevelWidgets)
    {
        if (widget)
        {
            //its 1-based originally
            TabIndex tabIndex = widget->getTabIndex().value_or(1) - 1;

            auto* tab = getOrMakeTab(tabIndex);
            if (!tab)
            {
                SV_ERROR(std::format("Couldnt create tab with index = [{}]", tabIndex));
                continue;
            }

            //SV_LOG("Adding widget to tab...");

            tab->addWidget(widget);
        }
        else SV_ERROR("Null widget in topLevelWidgets list !");
    }

    update();
}

void TopLevelWidgetsContainer::clearEverything(ExistingWidgetsPolicy existingWidgetsPolicy)
{
    SV_LOG("TopLevelWidgetsContainer::clearEverything()");

    //1. move existing top level widget pointer to a set

    std::set<QPointer<NodeWidget>> topLevelWidgetsSet;
    for (QPointer<NodeWidget> widgetQPointer : topLevelWidgets)
    {
        topLevelWidgetsSet.insert(widgetQPointer);
    }
    topLevelWidgets.clear();

    /*
    for (auto widget : topLevelWidgets)
    {
        if (widget)
        {
            //if i just write 'delete widget;' it crashes and i dont fucking understand why.
            //it should be perfectly fine either way, but its not.
            
            widget->setParent(nullptr);

            if (existingWidgetsPolicy == ExistingWidgetsPolicy::Delete)
            {
                widget->deleteLater();
            }
        }
    }
    topLevelWidgets.clear();
    */

    //2. extract all tabs

    std::vector<TabOfTopLevelWidgets*> extractedTabs;
    {
        QList<QWidget*> tabsAsQWidgets;
        extractAllWidgetsFromLayoutAndDeleteNestedLayouts(tabsLayout, &tabsAsQWidgets);

        for (QWidget* tabQWidget : tabsAsQWidgets)
        {
            TabOfTopLevelWidgets* tab = dynamic_cast<TabOfTopLevelWidgets*>(tabQWidget);
            SV_ASSERT(tab && "we are not expecting any other type here");

            extractedTabs.push_back(tab);
        }
    }

    //3. check that infact all tabs were extracted

    SV_ASSERT(tabsCount() == 0);

    //4. extract all widgets from tabs

    std::set<QPointer<NodeWidget>> extractedTopLevelWidgetsSet;
    for (TabOfTopLevelWidgets* tab : extractedTabs)
    {
        QList<QWidget*> extractedWidgets;
        tab->extractAllWidgets(&extractedWidgets);

        for (QWidget* extractedQWidget : extractedWidgets)
        {
            NodeWidget* extractedNodeWidget = dynamic_cast<NodeWidget*>(extractedQWidget);
            SV_ASSERT(extractedNodeWidget && "we are not expecting any other type here");
            extractedTopLevelWidgetsSet.insert(QPointer<NodeWidget>(extractedNodeWidget));
        }
    }

    //5. check that initial list of top level widgets is exact same as what we extracted

    SV_ASSERT(topLevelWidgetsSet == extractedTopLevelWidgetsSet);

    //6. i guess lets go bottom-first, and first delete the widgets (if we need to)

    if (existingWidgetsPolicy == ExistingWidgetsPolicy::Delete)
    {
        for (auto widget : extractedTopLevelWidgetsSet)
        {
            widget->deleteLater();
        }
    }
    topLevelWidgetsSet.clear();
    extractedTopLevelWidgetsSet.clear();

    //7. finally, delete tabs themselves (they are empty now)

    for (auto* tab : extractedTabs)
    {
        tab->deleteLater();
    }
    extractedTabs.clear();
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

void TopLevelWidgetsContainer::reassignTabIndexes()
{
    for (int tabIndex = 0; tabIndex < tabsCount(); ++tabIndex)
    {
        auto tab = getTab(tabIndex);
        SV_ASSERT(tab);

        tab->visitAllWidgets([tabIndex](QWidget* w)
        {
            if (auto nodeWidget = dynamic_cast<NodeWidget*>(w))
            {
                //if its default value of 0, just reset it
                nodeWidget->setTabIndex(tabIndex != 0 ? tabIndex : TabIndexOpt());
            }
        });
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
    tab->extractAllWidgets(&extractedWidgets);
    tab->deleteLater(); //crucial

    if (!extractedWidgets.empty())
    {
        // note that due to deleteLater, the deleted tab still exists under same index.
        // so we must make sure we are not adding widgets back to it:
        int tabThatInheritsIndex = index == 0 ? 1 : 0;

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
    reassignTabIndexes();

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

TabOfTopLevelWidgets *TopLevelWidgetsContainer::getOrMakeTab(const int index)
{
    SV_ASSERT(index >= 0);

    const int initialTabsCount = tabsCount();

    if (!isValidIndex(index, initialTabsCount))
    {
        const int tabsToMake = index - initialTabsCount + 1;

        for (int i = 0; i < tabsToMake; ++i)
        {
            addTab();
        }
    }

    return getTab(index);
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

void TabOfTopLevelWidgets::extractAllWidgets(QList<QWidget *> *outWidgets)
{
    extractAllWidgetsFromLayoutAndDeleteNestedLayouts(contentLayout, outWidgets);
}

void TabOfTopLevelWidgets::visitAllWidgets(const std::function<void(QWidget*)>& visitor)
{
    for (int i = 0; i < contentLayout->count(); ++i)
    {
        if (auto item = layout->itemAt(i))
        {
            if (auto widget = item->widget())
            {
                visitor(widget);
            }
        }
    }
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

