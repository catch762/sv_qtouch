#pragma once
#include "sv_qtcommon.h"
#include <QTableView>
#include <QFileSystemModel>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include "PresetFileSystemModel.h"
#include <QHeaderView>
#include <QStyledItemDelegate>

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

class PresetFileView : public QWidget
{
    Q_OBJECT
public:
    PresetFileView(const QString& rootPath, QWidget* parent = nullptr) : QWidget(parent)
    {
        layout = new QVBoxLayout(this);

        model = new PresetFileSystemModel(this);
        model->setRootPath(rootPath);        // start from user home
        model->setReadOnly(false);                   // enable rename/delete
        model->setNameFilters({"*.json"});
        model->setNameFilterDisables(false);

        view = new QTableView(this);
        view->setModel(model);
        view->setRootIndex(model->index(model->rootPath()));
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->verticalHeader()->hide();
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setShowGrid(false);

        //NoIconDelegate* delegate = new NoIconDelegate(view);

        // Set it only for column 0 (The Name column in QFileSystemModel)
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

        //Set up horizontal header:
        {
            // 1. Get a pointer to the horizontal header
            QHeaderView* header = view->horizontalHeader();

            // 2. Prevent the user from dragging/reordering columns manually
            header->setSectionsMovable(false);

            // 3. Configure Column 4 (The Checkbox Column on the right)
            header->setSectionResizeMode(4, QHeaderView::Fixed); // Lock resize behavior
            view->setColumnWidth(4, 45);                         // Force width to exactly 50px

            // 4. Configure Column 0 (The Name Column on the left)
            header->setSectionResizeMode(0, QHeaderView::Stretch); // Automatically takes all remaining space

            // 5. Explicitly hide columns 1, 2, and 3 if they are in your way
            view->setColumnHidden(1, true);
            view->setColumnHidden(2, true);
            view->setColumnHidden(3, true);
        }


        //view->setSelectionMode(QAbstractItemView::MultiSelection);
        //view->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(view, &QTableView::doubleClicked, this, [this](const QModelIndex &index)
        {
            emit presetLoadingRequested(model->fileName(index));
        });

        connect(view, &QTableView::customContextMenuRequested, this, &PresetFileView::onContextMenu);

        layout->addWidget(view);
    }

    bool presetNameExists(const QString& presetFileNameWithExtension)
    {
        bool exists = QFileInfo(model->rootDirectory().absoluteFilePath(presetFileNameWithExtension)).isFile();
        return exists;
    }

    void setRootPath(const QString& rootPath)
    {
        if (!rootPath.isEmpty()) {
        const QModelIndex rootIndex = model->setRootPath(rootPath);

        view->setRootIndex(rootIndex);
    }
    }

    void onContextMenu(const QPoint &pos)
    {
        const QModelIndex index = view->indexAt(pos);
        if (!index.isValid())
            return;

        const QString filePath = model->filePath(index);
        const QString fileName = model->fileName(index);

        QMenu menu(this);

        // Open action
        QAction *openAction = menu.addAction("Load");
        connect(openAction, &QAction::triggered, this, [fileName, this]()
        {
            //QMessageBox::information(this, "Ok", "Open");
            emit presetLoadingRequested(fileName);
        });

        // Rename action
        QAction *renameAction = menu.addAction("Rename");
        connect(renameAction, &QAction::triggered, this, [this, index]() {
            const QModelIndex index = view->currentIndex();
            if (index.isValid()) {
                view->edit(index);
            }
        });

        // Delete action
        QAction *deleteAction = menu.addAction("Delete");
        connect(deleteAction, &QAction::triggered, this, [this, filePath, index]() {
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
            } else {
                if (!QFile::remove(filePath)) {
                    QMessageBox::warning(this, "Error", "Failed to delete file.");
                }
            }
        });

        // Custom actions
        QAction *selectAsA = menu.addAction("Select as mixing preset A");
        connect(selectAsA, &QAction::triggered, this, [&] {
            emit presetWasSelectedForMixing(fileName, true);
        });

        QAction *selectAsB = menu.addAction("Select as mixing preset B");
        connect(selectAsB, &QAction::triggered, this, [&] {
            emit presetWasSelectedForMixing(fileName, false);
        });

        menu.exec(view->viewport()->mapToGlobal(pos));
    }

signals:
    //selected for preset A or for preset B
    void presetWasSelectedForMixing(const QString& presetFilename, bool selectedForA);
    void presetLoadingRequested(const QString& presetFilename);

private:
    QVBoxLayout* layout = nullptr;

    QTableView       *view = nullptr;
    PresetFileSystemModel* model = nullptr;

};