#include "PresetFileSystemModel.h"

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

    return fileNameIsInPresetExportList(fileName(index));
}

const std::set<QString> PresetFileSystemModel::getPresetExportList() const
{
    return presetFileNamesToExport;
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
        if (value.toInt() == Qt::Checked)
        {
            addFileNameToPresetExportList(fileName(index));
        }
        else
        {
            removeFileNameFromPresetExportList(fileName(index));
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
