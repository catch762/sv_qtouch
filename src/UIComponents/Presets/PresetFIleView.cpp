#include "PresetFileView.h"
#include <QHeaderView>
#include <QStyledItemDelegate>

//unused but if u wanted to remove that json file icon:
class NoIconDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
        QStyledItemDelegate::initStyleOption(option, index);

        // Remove the decoration (icon) features and clear the actual icon
        option->features &= ~QStyleOptionViewItem::HasDecoration;
        option->icon = QIcon();
    }
};

PresetFileView::PresetFileView(const QString& rootPath, QWidget* parent) : QWidget(parent)
{
    layout = new QVBoxLayout(this);

    model = new PresetFileSystemModel(this);
    model->setRootPath(rootPath);
    model->setReadOnly(false);                   // enable rename/delete
    model->setNameFilters({ "*.json" });
    model->setNameFilterDisables(false);

    view = new QTableView(this);
    view->setModel(model);
    view->setRootIndex(model->index(model->rootPath()));
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setShowGrid(false);

    //Disable json icons:
    //NoIconDelegate* delegate = new NoIconDelegate(view);
    //view->setItemDelegateForColumn(0, delegate);

    //im only doing it because there are some fucking retarded rules on drawing borders
    //on focused fields of a row, that i cant remove via stylesheet or anything else.
    //i also dont see any fucking documentation on this.
    //but if we just disable focus logic for the widget, borders go away:
    view->setFocusPolicy(Qt::NoFocus);

    //changes on header view:       disable font becoming bold on click
    //changes on table view item:   fucking retarded shit, figured by trial and error, i wont even describe.
    //                              theres no describing this shit and fucking ugly undocumented visual
    //                              "features" with no clear way to disable them
    view->setStyleSheet(R"(
            QHeaderView::section {
                font-weight: normal;
            }
            QHeaderView::section:selected {
                font-weight: normal;
            }

            QTableView::item {
                border: none;
                color: black;
            }
            QTableView::item:selected {
                border: none;
                background: rgb(230, 235, 240);
            }
            )"
    );

    /*
    int exportCol = model->exportColumn();
    for (int col = 0; col < model->columnCount(); ++col) {
    if (col != 0 && col != exportCol) {
    view->hideColumn(col);
    }
    }
    */

    //Set up header:
    {
        const int columnName = 0;
        const int columnExport = model->exportColumn();

        view->verticalHeader()->hide();

        QHeaderView* header = view->horizontalHeader();

        header->setSectionsMovable(false);

        header->setSectionResizeMode(columnName, QHeaderView::Stretch); //takes all remaining space

        header->setSectionResizeMode(columnExport, QHeaderView::Fixed);
        view->setColumnWidth(columnExport, 45);

        //hide all other columns
        view->setColumnHidden(1, true);
        view->setColumnHidden(2, true);
        view->setColumnHidden(3, true);
    }

    connect(view, &QTableView::doubleClicked, this, [this](const QModelIndex& index)
        {
            emit presetLoadingRequested(model->fileName(index));
        });

    connect(view, &QTableView::customContextMenuRequested, this, &PresetFileView::onContextMenu);

    layout->addWidget(view);
}

bool PresetFileView::presetNameExists(const QString& presetFileNameWithExtension)
{
    bool exists = QFileInfo(model->rootDirectory().absoluteFilePath(presetFileNameWithExtension)).isFile();
    return exists;
}

void PresetFileView::setRootPath(const QString& rootPath)
{
    if (!rootPath.isEmpty()) {
        const QModelIndex rootIndex = model->setRootPath(rootPath);

        view->setRootIndex(rootIndex);
    }
}

void PresetFileView::deleteSelectedFiles(const QModelIndexList& selectedRows)
{
    if (selectedRows.empty()) return;

    QString allPathsInfo;

    QStringList pathsToDelete;
    int filesCount = 0;
    int dirsCount = 0;
    for (const QModelIndex& index : selectedRows)
    {
        auto path = model->filePath(index);

        if (QFileInfo(path).isDir())
        {
            dirsCount++;
        }
        else filesCount++;

        allPathsInfo += QString("\t%1\n").arg(path);

        pathsToDelete.append(path);
    }

    QString msg = QString("Are you sure you want to delete %1%2%3 ?\n\n%4")
                    .arg(filesCount ? QString("%1 files").arg(filesCount) : "")
                    .arg(filesCount && dirsCount ? " and " : "")
                    .arg(dirsCount ? QString("%1 folders").arg(dirsCount) : "")
                    .arg(allPathsInfo);


    auto reply = QMessageBox::question(this, "Confirm Delete", msg,
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    for (const QString& path : pathsToDelete)
    {
        const QFileInfo fi(path);
        if (fi.isDir()) {
            QDir dir(path);
            if (!dir.removeRecursively()) {
                QMessageBox::warning(this, "Error", "Failed to delete directory.");
            }
        }
        else {
            if (!QFile::remove(path)) {
                QMessageBox::warning(this, "Error", "Failed to delete file.");
            }
        }
    }
}

void PresetFileView::onContextMenu(const QPoint& pos)
{
    //we rely on index being at 0 column strictly. If we rightclicked on other column,
    //we wont be able to get data from that index with non-zero column.

    QModelIndexList selectedRows = view->selectionModel()->selectedRows(0);

    const QModelIndex index = selectedRows.first();
    if (!index.isValid())
        return;

    const QString filePath = model->filePath(index);
    const QString fileName = model->fileName(index);

    QMenu menu(this);

    // Open action
    QAction* openAction = menu.addAction("Load");
    openAction->setEnabled(selectedRows.size() == 1);
    connect(openAction, &QAction::triggered, this, [fileName, this]()
        {
            //QMessageBox::information(this, "Ok", "Open");
            emit presetLoadingRequested(fileName);
        });

    // Rename action
    QAction* renameAction = menu.addAction("Rename");
    renameAction->setEnabled(selectedRows.size() == 1);
    connect(renameAction, &QAction::triggered, this, [this, index]() {
        //const QModelIndex index = view->currentIndex();
        if (index.isValid()) {
            view->edit(index);
        }
        });

    // Delete action
    QAction* deleteAction = menu.addAction("Delete");
    connect(deleteAction, &QAction::triggered, this, [this, selectedRows]()
    {
        deleteSelectedFiles(selectedRows);
        return;

        /*
        const QFileInfo fi(filePath);
        const QString what = fi.isDir() ? "directory" : "file";
        const int ret = QMessageBox::question(
            this,
            "Confirm",
            QString("Delete %1 \"%2\"?").arg(what).arg(fi.fileName())
        );

        if (ret != QMessageBox::Yes)
            return;

        if (fi.isDir()) {
            QDir dir(filePath);
            if (!dir.removeRecursively()) {
                QMessageBox::warning(this, "Error", "Failed to delete directory.");
            }
        }
        else {
            if (!QFile::remove(filePath)) {
                QMessageBox::warning(this, "Error", "Failed to delete file.");
            }
        }
        */

        });

    // Custom actions
    QAction* selectAsA = menu.addAction("Select as mixing preset A");
    selectAsA->setEnabled(selectedRows.size() == 1);
    connect(selectAsA, &QAction::triggered, this, [&] {
        emit presetWasSelectedForMixing(fileName, true);
        });

    QAction* selectAsB = menu.addAction("Select as mixing preset B");
    selectAsB->setEnabled(selectedRows.size() == 1);
    connect(selectAsB, &QAction::triggered, this, [&] {
        emit presetWasSelectedForMixing(fileName, false);
        });

    menu.exec(view->viewport()->mapToGlobal(pos));
}
