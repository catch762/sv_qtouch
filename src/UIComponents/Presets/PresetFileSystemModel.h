#pragma once
#include <QFileSystemModel>
#include "sv_qtcommon.h"

class PresetFileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit PresetFileSystemModel(QObject* parent = nullptr)
        : QFileSystemModel(parent) {
    }

    // Extra column index (after the default Name/Size/Type/Date)
    static int exportColumn() { return columnCount() - 1; }

    // Accessor for the boolean value
    bool exportValue(const QModelIndex& index) const
    {
        if (!index.isValid() || index.column() != exportColumn())
            return false;
        const QString& path = filePath(index);
        return m_exportValues.value(path, false);
    }

    void setExportValue(const QModelIndex& index, bool value)
    {
        if (!index.isValid() || index.column() != exportColumn())
            return;
        const QString& path = filePath(index);
        m_exportValues[path] = value;
        emit dataChanged(index, index, { Qt::CheckStateRole, Qt::DisplayRole });
    }

protected:
    int columnCount(const QModelIndex& parent = {}) const override
    {
        // QFileSystemModel has 4 columns by default (Name, Size, Type, Date)
        return QFileSystemModel::columnCount(parent) + 1;
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid())
            return QFileSystemModel::data(index, role);

        if (index.column() == exportColumn()) {
            const QString& path = filePath(index);
            bool value = m_exportValues.value(path, false);

            if (role == Qt::DisplayRole) {
                return value ? QString("Yes") : QString("No");
            }
            if (role == Qt::CheckStateRole) {
                return value ? Qt::Checked : Qt::Unchecked;
            }
            if (role == Qt::AlignmentRole) {
                return Qt::AlignHCenter | Qt::AlignVCenter;
            }
        }

        return QFileSystemModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        Qt::ItemFlags f = QFileSystemModel::flags(index);
        if (index.isValid() && index.column() == exportColumn()) {
            f |= Qt::ItemIsUserCheckable;
        }
        return f;
    }

private:
    QHash<QString, bool> m_exportValues;
};