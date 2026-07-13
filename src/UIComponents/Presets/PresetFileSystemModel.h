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
    int exportColumn() const { return columnCount() - 1; }

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
        }

        return QFileSystemModel::data(index, role);
    }

    bool setData(const QModelIndex& index,
        const QVariant& value,
        int role) override
    {
        if (!index.isValid() || index.column() != exportColumn())
            return QFileSystemModel::setData(index, value, role);

        if (role == Qt::CheckStateRole) {
            bool checked = value.toInt() == Qt::Checked;
            const QString& path = filePath(index);
            m_exportValues[path] = checked;

            emit dataChanged(index, index, { Qt::CheckStateRole, Qt::DisplayRole });
            return true;
        }

        return QFileSystemModel::setData(index, value, role);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == exportColumn()) {
                return QString("Export");
            }
        }
        return QFileSystemModel::headerData(section, orientation, role);
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