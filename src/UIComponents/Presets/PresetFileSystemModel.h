#pragma once
#include <QFileSystemModel>
#include "sv_qtcommon.h"

// This exists for the single reason: to add extra field to items of the model,
// the field being essentially 'bool exportEnabled' - to track presets that
// we are sending to Touchdesigner.

class PresetFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    using PresetNamesSet = std::set<QString>;
    SV_DECL_OPT(PresetNamesSet);

    explicit PresetFileSystemModel(QObject* parent = nullptr);

    const PresetNamesSet& getPresetExportList() const;
    void setPresetExportList(PresetNamesSet newPresetFileNamesToExport);

    bool fileNameIsInPresetExportList(const QString& presetFilename) const;
    bool indexIsInPresetExportList(const QModelIndex& index) const;
    void addFileNameToPresetExportList(const QString& presetFilename);
    void removeFileNameFromPresetExportList(const QString& presetFilename);

    bool savePresetExportListToFile(const QString& filePath);
    bool loadPresetExportListFromFile(const QString& filePath);

    // Extra column index (after the default Name/Size/Type/Date)
    int exportColumn() const;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex makeFirstColumnIndex(const QModelIndex& index) const;

private:
    std::set<QString> presetFileNamesToExport;
};