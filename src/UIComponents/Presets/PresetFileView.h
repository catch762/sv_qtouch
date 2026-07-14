#pragma once
#include "sv_qtcommon.h"
#include <QTableView>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include "PresetFileSystemModel.h"


class PresetFileView : public QWidget
{
    Q_OBJECT
public:
    PresetFileView(const QString& rootPath, QWidget* parent = nullptr);

    bool presetNameExists(const QString& presetFileNameWithExtension);

    void setRootPath(const QString& rootPath);

    void onContextMenu(const QPoint &pos);

signals:
    //selected for preset A or for preset B
    void presetWasSelectedForMixing(const QString& presetFilename, bool selectedForA);
    void presetLoadingRequested(const QString& presetFilename);

private:
    QVBoxLayout* layout = nullptr;

    QTableView       *view = nullptr;
    PresetFileSystemModel* model = nullptr;

};