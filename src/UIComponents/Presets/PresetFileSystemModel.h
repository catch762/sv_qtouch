#pragma once
#include <QFileSystemModel>
#include "sv_qtcommon.h"

class PresetFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit PresetFileSystemModel(QObject* parent = nullptr);

    // Extra column index (after the default Name/Size/Type/Date)
    int exportColumn() const;

    // Accessor for the boolean value
    bool indexIsInPresetExportList(const QModelIndex& index) const;

    const std::set<QString> getPresetExportList() const;

    bool fileNameIsInPresetExportList(const QString& presetFilename) const;

    void addFileNameToPresetExportList(const QString& presetFilename);

    void removeFileNameFromPresetExportList(const QString& presetFilename);

    int columnCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role) const override;

    bool setData(const QModelIndex& index,
        const QVariant& value,
        int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex makeFirstColumnIndex(const QModelIndex& index) const;

private:
    std::set<QString> presetFileNamesToExport;
};