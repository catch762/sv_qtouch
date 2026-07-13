#pragma once
#include "sv_qtcommon.h"
#include "PresetFileView.h"
#include <QCheckBox>

class PresetTab : public QWidget
{
    Q_OBJECT
public:
    PresetTab(QWidget* parent = nullptr) : QWidget(parent)
    {
        layout = new QGridLayout(this);

        initPresetMixArea();

        presetView = new PresetFileView("");
        layout->addWidget(presetView, 1, 0);

        savePresetButton = new QPushButton("Save preset", this);
        connect(savePresetButton, &QPushButton::clicked, this, &PresetTab::onSavePresetClicked);
        layout->addWidget(savePresetButton, 2, 0);

        connect(presetView, &PresetFileView::presetWasSelectedForMixing, this,
            [this](const QString& fileName, bool forA)
            {
                QLineEdit* nameWidget = forA ? mixPresetAName : mixPresetBName;
                nameWidget->setText(fileName);

                if (mixPresetModeIsEnabled())
                {
                    //refresh it
                    emitPresetMixingStateSignal();
                }
            });
    }

    PresetFileView* getPresetView()
    {
        return presetView;
    }

    void setMixPresetMode(bool enabled, bool emitActivation = true)
    {
        {
            QSignalBlocker block(mixModeOnButton);
            mixModeOnButton->setChecked(enabled);
        }

        iteratePresetMixAreaContentWidgets([enabled](QWidget* widget)
        {
            widget->setEnabled(enabled); 
        });

        if(emitActivation)
        {
            emitPresetMixingStateSignal();
        }
    }

    bool mixPresetModeIsEnabled()
    {
        return mixModeOnButton->isChecked();
    }
signals:
    void presetMixingActivated(const QString& presetFilenameA, const QString& presetFilenameB, double morphAtoB01);
    void presetMixingDeactivated();
    void presetSavingRequested(const QString& presetFilename);

private slots:
    void onSavePresetClicked();

private:
    void emitPresetMixingStateSignal()
    {
        if (mixPresetModeIsEnabled())
        {
            emit presetMixingActivated( mixPresetAName->text(), mixPresetBName->text(), getSliderValue01(mixSlider) );
        }
        else
        {
            emit presetMixingDeactivated();
        }
    }

    void initPresetMixArea()
    {
        //Master frame and layout of this entire area
        {
            mixPresetFrame = new QFrame(this);
            mixPresetFrame->setFrameStyle(QFrame::Panel | QFrame::Raised);
            mixPresetFrame->setLineWidth(1);
            mixPresetFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
            
            mixPresetLayout = new QGridLayout(mixPresetFrame);
            mixPresetLayout->setColumnStretch(0, 0);
            mixPresetLayout->setColumnStretch(1, 1);
        }
        
        //Button to enable morph mode
        {
            mixModeOnButton = new QCheckBox("Morph between presets:", mixPresetFrame);
            connect(mixModeOnButton, &QCheckBox::checkStateChanged, [this](Qt::CheckState state)
            {
                bool isOn = state==Qt::CheckState::Checked;
                setMixPresetMode(isOn);
            });
            
            mixPresetLayout->addWidget(mixModeOnButton,0,0,1,2, Qt::AlignLeft);
        }
        
        //Names of presets to morph
        {
            auto makeNameWidget = []()
            {
                auto w = new QLineEdit;
                w->setReadOnly(true);
                w->setFocusPolicy(Qt::NoFocus);
                w->setPlaceholderText("not selected");
                w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                return w;
            };
            mixPresetAName = makeNameWidget();
            mixPresetBName = makeNameWidget();
        }
        
        //Just static labels
        {
            auto makeLabel = [](const QString& text)
            {
                auto w = new QLabel(text);
                w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
                return w;
            };
            
            mixPresetALabel = makeLabel("Preset A:");
            mixPresetBLabel = makeLabel("Preset B:");
        }
        
        //Positioning of labels and names
        {
            mixPresetLayout->addWidget(mixPresetALabel,1,0, 1,1,Qt::AlignLeft);
            mixPresetLayout->addWidget(mixPresetAName,1,1,1,1);
            mixPresetLayout->addWidget(mixPresetBLabel,2,0,1,1,Qt::AlignLeft);
            mixPresetLayout->addWidget(mixPresetBName,2,1,1,1);
        }
        
        //Morph slider
        {
            mixSlider = new QSlider(Qt::Horizontal);
            mixSlider->setMinimum(0);
            mixSlider->setMaximum(10'000);
            mixSlider->setTickInterval(1);

            connect(mixSlider, &QSlider::valueChanged, [this]()
            {
                emitPresetMixingStateSignal();
            });

            mixPresetLayout->addWidget(mixSlider,3,0,1,2);
        }
        
        layout->addWidget(mixPresetFrame);

        setMixPresetMode(false);
    }

    void iteratePresetMixAreaContentWidgets(std::function<void(QWidget*)> visitor)
    {
        visitor(mixPresetAName);
        visitor(mixPresetBName);
        visitor(mixPresetALabel);
        visitor(mixPresetBLabel);
        visitor(mixSlider);
    }

private:
    QGridLayout* layout                         = nullptr;
    QFrame*         mixPresetFrame              = nullptr;
    QGridLayout*    mixPresetLayout             = nullptr;
    //QLabel*             mixModeLabel          = nullptr;
    QCheckBox*            mixModeOnButton       = nullptr;
        QLabel*             mixPresetALabel     = nullptr;
        QLineEdit*              mixPresetAName  = nullptr;
        QLabel*             mixPresetBLabel     = nullptr;
        QLineEdit*              mixPresetBName  = nullptr;
        QSlider*            mixSlider           = nullptr;
    PresetFileView* presetView                  = nullptr;
    QPushButton*    savePresetButton            = nullptr;

private:
    QString lastSavedPresetName;
};