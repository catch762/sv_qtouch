#pragma once
#include "sv_qtcommon.h"
#include <QTableView>
#include <QFileSystemModel>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include "PresetFileSystemModel.h"
#include <QHeaderView>

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
        
        int a = 1;

        if (a == 1)
        {
            view->setStyleSheet(
                "QHeaderView::section {"
                "    font-weight: normal;"
                "}"
                "QHeaderView::section:selected {"
                "    font-weight: normal;"
                "}"
                ""
                "QTableView::item {"
                "    border: none;"
                "}"
                "QTableView::item:selected {"
                "    background: #f5f5f5;"
                "    border: none;"
                "    outline: 0;"
                "}"
            );
        }
        else if (a == 2)
        {
            view->setStyleSheet(
                "QTableView{\
                    background-color: #242526;\
                    gridline-color: #3f4042;\
                    color: #f0f0f0;\
                    font-size: 13px;\
                    selection-background - color: #1a73e8;\
                    selection-color: #ffffff;\
                    border: 1px solid #32414B;\
                }\
                QTableView::item{\
                    padding: 6px;\
                    border-bottom: 1px solid #32414B;\
                }\
                QTableView::item:hover{\
                    background-color: #303134;\
                }\
                QTableView::item : selected{\
                    background-color: #1a73e8;\
                    color: #ffffff;\
                }"
            );
        }
        

        int exportCol = model->exportColumn();
        for (int col = 0; col < model->columnCount(); ++col) {
            if (col != 0 && col != exportCol) {
                view->hideColumn(col);
            }
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