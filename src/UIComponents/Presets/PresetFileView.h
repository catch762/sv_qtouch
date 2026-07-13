#pragma once
#include "sv_qtcommon.h"
#include <QListView>
#include <QFileSystemModel>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>

class PresetFileView : public QWidget
{
    Q_OBJECT
public:
    PresetFileView(const QString& rootPath, QWidget* parent = nullptr) : QWidget(parent)
    {
        layout = new QVBoxLayout(this);

        model = new QFileSystemModel(this);
        model->setRootPath(rootPath);        // start from user home
        model->setReadOnly(false);                   // enable rename/delete
        model->setNameFilters({"*.json"});
        model->setNameFilterDisables(false);

        listView = new QListView(this);
        listView->setModel(model);
        listView->setRootIndex(model->index(model->rootPath()));
        listView->setContextMenuPolicy(Qt::CustomContextMenu);
        listView->setSelectionMode(QAbstractItemView::MultiSelection);
        listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(listView, &QListView::doubleClicked, this, [this](const QModelIndex &index)
        {
            emit presetLoadingRequested(model->fileName(index));
        });

        connect(listView, &QListView::customContextMenuRequested, this, &PresetFileView::onContextMenu);

        layout->addWidget(listView);
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

        listView->setRootIndex(rootIndex);
    }
    }

    void onContextMenu(const QPoint &pos)
    {
        const QModelIndex index = listView->indexAt(pos);
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
            const QModelIndex index = listView->currentIndex();
            if (index.isValid()) {
                listView->edit(index);
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

        menu.exec(listView->viewport()->mapToGlobal(pos));
    }

signals:
    //selected for preset A or for preset B
    void presetWasSelectedForMixing(const QString& presetFilename, bool selectedForA);
    void presetLoadingRequested(const QString& presetFilename);

private:
    QVBoxLayout* layout = nullptr;

    QListView       *listView = nullptr;
    QFileSystemModel *model = nullptr;

};