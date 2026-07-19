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

    return presetNameIsInPresetExportList(fileName(makeFirstColumnIndex(index)));
}

const std::set<QString>& PresetFileSystemModel::getPresetExportList() const
{
    return presetNamesToExport;
}

void PresetFileSystemModel::setPresetExportList(std::set<QString> newPresetFileNamesToExport)
{
    // 1. Update the internal set data
    presetNamesToExport = std::move(newPresetFileNamesToExport);

    // 2. Identify the boundaries of the top-level items
    QModelIndex topLeft     = index(0,              exportColumn());
    QModelIndex bottomRight = index(rowCount() - 1, exportColumn());

    // 3. Notify all connected views to refresh this entire column
    emit dataChanged(topLeft, bottomRight, { Qt::CheckStateRole });
}

bool PresetFileSystemModel::presetNameIsInPresetExportList(const PresetNameString& presetName) const
{
    return presetNamesToExport.contains(presetName);
}

void PresetFileSystemModel::addPresetNameToPresetExportList(const PresetNameString& presetName)
{
    presetNamesToExport.insert(presetName);
}

void PresetFileSystemModel::removePresetNameFromPresetExportList(const PresetNameString& presetName)
{
    presetNamesToExport.erase(presetName);
}

QJsonValue PresetFileSystemModel::savePresetExportListToJson() const
{
    return Serializer<PresetNamesSet>().toJson(getPresetExportList());
}

bool PresetFileSystemModel::loadPresetExportListFromJson(const QJsonValue& json)
{
    auto loadedSet = Serializer<PresetNamesSet>().fromJson(json);
    if (!loadedSet)
    {
        return false;
    }

    setPresetExportList(std::move(*loadedSet));
    return true;
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
        const auto presetName = getFileNameWithoutExtension( fileName(makeFirstColumnIndex(index)) );

        if (value.toInt() == Qt::Checked)
        {
            addPresetNameToPresetExportList(presetName);
        }
        else
        {
            removePresetNameFromPresetExportList(presetName);
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
