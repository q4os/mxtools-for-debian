#include "selectlayoutdialog.h"
#include "ui_selectlayoutdialog.h"

SelectLayoutDialog::SelectLayoutDialog(const KeyboardInfo &info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectLayoutDialog),
    m_info(info),
    m_good(false)
{
    ui->setupUi(this);
    for(auto layout : m_info.layouts())
    {
        ui->comboBox_SelectedLayout->addItem(keyboardtr(layout.config.description));
        int index = ui->comboBox_SelectedLayout->count() - 1;
        ui->comboBox_SelectedLayout->setItemIcon(index, m_info.layoutIcon(layout.config.name));
        ui->comboBox_SelectedLayout->setItemData(index, layout.config.name, NameRole);
    }
    refreshSelectedVariants();
    ui->comboBox_SelectedLayout->model()->sort(0);

    connect(ui->comboBox_SelectedLayout, &QComboBox::currentTextChanged, [this](QString){
        refreshSelectedVariants();
    });
}

SelectLayoutDialog::~SelectLayoutDialog()
{
    delete ui;
}

QPair<KeyboardConfigItem, KeyboardConfigItem> SelectLayoutDialog::selectedLayout() const
{
    for(auto layout : m_info.layouts())
    {
        if(layout.config.name == ui->comboBox_SelectedLayout->currentData(NameRole))
        {
            for(auto variant : layout.variants)
            {
                if(variant.name == ui->comboBox_SelectedVariant->currentData(NameRole))
                {
                    return {layout.config, variant};
                }
            }
            return {layout.config, {}};
        }
    }
    return {{}, {}};
}

void SelectLayoutDialog::refreshSelectedVariants()
{
    for(auto layout : m_info.layouts())
    {
        if(layout.config.name == ui->comboBox_SelectedLayout->currentData(NameRole))
        {
            ui->comboBox_SelectedVariant->clear();
            ui->comboBox_SelectedVariant->addItem(tr("No Variant"));
            ui->comboBox_SelectedVariant->setItemData(0, "system-keyboard-qt-no-variant", NameRole);
            for(auto variant : layout.variants)
            {
                ui->comboBox_SelectedVariant->addItem(keyboardtr(variant.description));
                int index = ui->comboBox_SelectedVariant->count() - 1;
                ui->comboBox_SelectedVariant->setItemData(index, variant.name, NameRole);
            }
            break;
        }
    }
    ui->comboBox_SelectedVariant->model()->sort(0);
}
