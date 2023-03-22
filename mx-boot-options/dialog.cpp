#include "dialog.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

CustomDialog::CustomDialog(const QStringList &items, QDialog *parent)
    : QDialog(parent)
{
    auto *layout = new QGridLayout();
    setLayout(layout);

    box = new QComboBox;
    box->addItems(items);

    auto *label = new QLabel(tr("Live environment detected. Please select the root partition of the\n"
                                " system you want to modify (only Linux partitions are displayed)"));

    layout->addWidget(label, 0, 0, 1, 3);
    layout->addWidget(box, 1, 0, 1, 3);

    auto *ok = new QPushButton(tr("OK"));
    auto *cancel = new QPushButton(tr("Cancel"));
    ok->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
    cancel->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));

    layout->addWidget(ok, 2, 1, 1, 1);
    layout->addWidget(cancel, 2, 2, 1, 1);

    connect(ok, &QPushButton::clicked, this, &CustomDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &CustomDialog::reject);
}

QComboBox *CustomDialog::comboBox() { return box; }
