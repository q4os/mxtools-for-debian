/**********************************************************************
 *  PreviewDialog.h
 **********************************************************************
 * Copyright (C) 2017-2025 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-conky.
 *
 * mx-conky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-conky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-conky.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QVBoxLayout>

#include "conkymanager.h"

class PreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDialog(ConkyManager *manager, ConkyItem *selectedItem = nullptr, QWidget *parent = nullptr);
    ~PreviewDialog() override;

signals:
    void conkyListNeedsRefresh();
    void conkyItemNeedsSelection(const QString &filePath);

private slots:
    void onAccepted();
    void generatePreviews();
    void onPreviewGenerated();
    void onAllPreviewsComplete();

private:
    ConkyManager *m_manager;
    ConkyItem *m_selectedItem;

    // UI components
    QRadioButton *m_generateCurrentRadio;
    QRadioButton *m_generateMissingRadio;
    QRadioButton *m_generateAllRadio;
    QCheckBox *m_highQualityCheck;

    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    // Generation state
    QStringList m_itemsToProcess;
    int m_currentIndex;
    bool m_isGenerating;
    QTimer *m_generationTimer;

    void setupUI();
    void generatePreviewForItem(ConkyItem *item);
    QString generatePreviewImage(ConkyItem *item);
    QStringList getItemsToProcess() const;
    ConkyItem *ensureConkyInUserDir(ConkyItem *item);
    void cleanupBeforeNextPreview();
};
