#include "PresetTab.h"
#include <QInputDialog>

void PresetTab::onSavePresetClicked()
{
    bool ok = false;
    QString presetName = QInputDialog::getText(
        nullptr,                    
        tr("Saving preset"),                                // dialog title
        tr("Please enter preset name without extension"),   // label text
        QLineEdit::Normal,
        lastSavedPresetName,                                // default text
        &ok                                                 // out: did user press OK?
    );

    if (!ok)
    {
        return;
    }

    if (presetName.isEmpty())
    {
        //todo maybe display a message
        return;
    }

    QString fullPresetName = presetName + ".json";

    bool alreadyExists = presetView->presetNameExists(fullPresetName);

    if (alreadyExists)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,                          
            "Confirm overwriting preset",                                           // window title
            QString("Do you want to overwrite preset [%1] ?").arg(fullPresetName),  // message text
            QMessageBox::Yes | QMessageBox::No,                                     // buttons
            QMessageBox::Yes                                                        // default button
        );

        if (reply != QMessageBox::Yes)
        {
            return;
        }
    }

    lastSavedPresetName = presetName;

    emit presetSavingRequested(fullPresetName);
}