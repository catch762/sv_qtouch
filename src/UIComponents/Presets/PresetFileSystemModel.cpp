#include "PresetFileSystemModel.h"


//todo
#include "Registrations/DefaultSerializers.h"
#include "Registrations/Utils/ContainerSerializers.h"

PresetFileSystemModel::PresetFileSystemModel(QObject* parent)
    : QFileSystemModel(parent) {
}

// Extra column index (after the default Name/Size/Type/Date)
int PresetFileSystemModel::exportColumn() const { return columnCount() - 1; }

// Accessor for the boolean value
bool PresetFileSystemModel::indexIsInPresetExportList(const QModelIndex& index) const
{
    if (!index.isValid() || isDir(index) || index.column() != exportColumn())
    {
        SV_WARN("Cant get export value for index");
        return false;
    }

    return fileNameIsInPresetExportList(fileName(makeFirstColumnIndex(index)));
}

const std::set<QString>& PresetFileSystemModel::getPresetExportList() const
{
    return presetFileNamesToExport;
}

void PresetFileSystemModel::setPresetExportList(std::set<QString> newPresetFileNamesToExport)
{
    // 1. Update the internal set data
    presetFileNamesToExport = std::move(newPresetFileNamesToExport);

    // 2. Identify the boundaries of the top-level items
    QModelIndex topLeft     = index(0,              exportColumn());
    QModelIndex bottomRight = index(rowCount() - 1, exportColumn());

    // 3. Notify all connected views to refresh this entire column
    emit dataChanged(topLeft, bottomRight, { Qt::CheckStateRole });
}

bool PresetFileSystemModel::fileNameIsInPresetExportList(const QString& presetFilename) const
{
    return presetFileNamesToExport.contains(presetFilename);
}

void PresetFileSystemModel::addFileNameToPresetExportList(const QString& presetFilename)
{
    presetFileNamesToExport.insert(presetFilename);
}

void PresetFileSystemModel::removeFileNameFromPresetExportList(const QString& presetFilename)
{
    presetFileNamesToExport.erase(presetFilename);
}

bool PresetFileSystemModel::savePresetExportListToFile(const QString& filePath)
{
    auto json = Serializer<PresetNamesSet>().toJson(getPresetExportList());

    return saveJsonValueToFile(json, filePath);
}

bool PresetFileSystemModel::loadPresetExportListFromFile(const QString& filePath)
{
    auto json = loadJsonFromFile(filePath);
    if (!json)
    {
        return false;
    }

    auto loadedSet = Serializer<PresetNamesSet>().fromJson(*json);
    if (!loadedSet)
    {
        return false;
    }

    setPresetExportList(std::move(*loadedSet));
}

int PresetFileSystemModel::columnCount(const QModelIndex& parent) const
{
    // QFileSystemModel has 4 columns by default (Name, Size, Type, Date)
    return QFileSystemModel::columnCount(parent) + 1;
}

QVariant PresetFileSystemModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && !isDir(index) && index.column() == exportColumn())
    {
        if (role == Qt::CheckStateRole)
        {
            return indexIsInPresetExportList(index) ? Qt::Checked : Qt::Unchecked;
        }

        return QVariant();
    }

    return QFileSystemModel::data(index, role);
}

bool PresetFileSystemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && !isDir(index) && index.column() == exportColumn() && role == Qt::CheckStateRole)
    {
        const auto filename = fileName(makeFirstColumnIndex(index));

        if (value.toInt() == Qt::Checked)
        {
            addFileNameToPresetExportList(filename);
        }
        else
        {
            removeFileNameFromPresetExportList(filename);
        }

        emit dataChanged(index, index, { Qt::CheckStateRole, Qt::DisplayRole });
        return true;
    }

    return QFileSystemModel::setData(index, value, role);
}

QVariant PresetFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == exportColumn())
    {
        return QString("Export");
    }

    return QFileSystemModel::headerData(section, orientation, role);
}

Qt::ItemFlags PresetFileSystemModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QFileSystemModel::flags(index);
    if (index.isValid() && index.column() == exportColumn()) {
        f |= Qt::ItemIsUserCheckable;
    }
    return f;
}

inline QModelIndex PresetFileSystemModel::makeFirstColumnIndex(const QModelIndex& index) const
{
    return index.sibling(index.row(), 0);
}
