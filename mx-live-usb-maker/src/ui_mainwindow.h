/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *buttonBar;
    QPushButton *pushCancel;
    QSpacerItem *horizontalSpacer2;
    QPushButton *pushHelp;
    QPushButton *pushBack;
    QSpacerItem *horizontalSpacer1;
    QLabel *labelLogo;
    QPushButton *pushNext;
    QPushButton *pushAbout;
    QPushButton *pushLumLogFile;
    QStackedWidget *stackedWidget;
    QWidget *selectionPage;
    QGridLayout *gridLayout;
    QPushButton *pushSelectSource;
    QLabel *label_2;
    QLabel *label_3;
    QGroupBox *groupAdvOptions;
    QGridLayout *gridLayout_5;
    QFrame *frame;
    QGridLayout *gridLayout_4;
    QCheckBox *checkForceMakefs;
    QCheckBox *checkSaveBoot;
    QCheckBox *checkGpt;
    QCheckBox *checkUpdate;
    QCheckBox *checkKeep;
    QCheckBox *checkForceUsb;
    QCheckBox *checkForceAutomount;
    QCheckBox *checkSetPmbrBoot;
    QCheckBox *checkForceNofuse;
    QSpacerItem *horizontalSpacer_2;
    QLabel *labelSizeEsp;
    QSpinBox *spinBoxEsp;
    QLabel *labelVerbosity;
    QSlider *sliderVerbosity;
    QComboBox *comboBoxDataFormat;
    QSpinBox *spinBoxDataSize;
    QCheckBox *checkDataFirst;
    QLabel *labelFormat;
    QPushButton *pushRefresh;
    QPushButton *pushOptions;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QFrame *frame_3;
    QVBoxLayout *verticalLayout_2;
    QRadioButton *radioNormal;
    QRadioButton *radioDd;
    QLabel *label_4;
    QSpacerItem *verticalSpacer;
    QComboBox *comboUsb;
    QGroupBox *groupOptions;
    QGridLayout *gridLayout_6;
    QSpacerItem *horizontalSpacer_3;
    QSpinBox *spinBoxSize;
    QLabel *label_percent;
    QLabel *label_part_label;
    QSpacerItem *horizontalSpacer;
    QLineEdit *textLabel;
    QFrame *frame_2;
    QGridLayout *gridLayout_7;
    QCheckBox *checkPretend;
    QCheckBox *checkCloneMode;
    QCheckBox *checkEncrypt;
    QCheckBox *checkCloneLive;
    QSpacerItem *verticalSpacer_2;
    QWidget *outputPage;
    QGridLayout *gridLayout_3;
    QProgressBar *progBar;
    QPlainTextEdit *outputBox;

    void setupUi(QDialog *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(937, 789);
        gridLayout_2 = new QGridLayout(MainWindow);
        gridLayout_2->setObjectName("gridLayout_2");
        buttonBar = new QGridLayout();
        buttonBar->setSpacing(5);
        buttonBar->setObjectName("buttonBar");
        buttonBar->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        buttonBar->setContentsMargins(0, 0, 0, 0);
        pushCancel = new QPushButton(MainWindow);
        pushCancel->setObjectName("pushCancel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushCancel->sizePolicy().hasHeightForWidth());
        pushCancel->setSizePolicy(sizePolicy);
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("window-close")));
        pushCancel->setIcon(icon);
        pushCancel->setAutoDefault(false);

        buttonBar->addWidget(pushCancel, 1, 11, 1, 1);

        horizontalSpacer2 = new QSpacerItem(100, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonBar->addItem(horizontalSpacer2, 1, 6, 1, 1);

        pushHelp = new QPushButton(MainWindow);
        pushHelp->setObjectName("pushHelp");
        sizePolicy.setHeightForWidth(pushHelp->sizePolicy().hasHeightForWidth());
        pushHelp->setSizePolicy(sizePolicy);
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("help-contents")));
        pushHelp->setIcon(icon1);
        pushHelp->setAutoDefault(false);

        buttonBar->addWidget(pushHelp, 1, 1, 1, 1);

        pushBack = new QPushButton(MainWindow);
        pushBack->setObjectName("pushBack");
        sizePolicy.setHeightForWidth(pushBack->sizePolicy().hasHeightForWidth());
        pushBack->setSizePolicy(sizePolicy);
        QIcon icon2(QIcon::fromTheme(QString::fromUtf8("go-previous")));
        pushBack->setIcon(icon2);
        pushBack->setAutoDefault(false);

        buttonBar->addWidget(pushBack, 1, 8, 1, 1);

        horizontalSpacer1 = new QSpacerItem(100, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonBar->addItem(horizontalSpacer1, 1, 3, 1, 1);

        labelLogo = new QLabel(MainWindow);
        labelLogo->setObjectName("labelLogo");
        labelLogo->setMaximumSize(QSize(32, 32));
        labelLogo->setMidLineWidth(0);
        labelLogo->setPixmap(QPixmap(QString::fromUtf8(":/icons/images/logo.svg")));
        labelLogo->setScaledContents(true);

        buttonBar->addWidget(labelLogo, 1, 4, 1, 1);

        pushNext = new QPushButton(MainWindow);
        pushNext->setObjectName("pushNext");
        sizePolicy.setHeightForWidth(pushNext->sizePolicy().hasHeightForWidth());
        pushNext->setSizePolicy(sizePolicy);
        QIcon icon3(QIcon::fromTheme(QString::fromUtf8("go-next")));
        pushNext->setIcon(icon3);
        pushNext->setAutoDefault(true);

        buttonBar->addWidget(pushNext, 1, 9, 1, 1);

        pushAbout = new QPushButton(MainWindow);
        pushAbout->setObjectName("pushAbout");
        sizePolicy.setHeightForWidth(pushAbout->sizePolicy().hasHeightForWidth());
        pushAbout->setSizePolicy(sizePolicy);
        QIcon icon4(QIcon::fromTheme(QString::fromUtf8("help-about")));
        pushAbout->setIcon(icon4);
        pushAbout->setAutoDefault(false);

        buttonBar->addWidget(pushAbout, 1, 0, 1, 1);

        pushLumLogFile = new QPushButton(MainWindow);
        pushLumLogFile->setObjectName("pushLumLogFile");
        sizePolicy.setHeightForWidth(pushLumLogFile->sizePolicy().hasHeightForWidth());
        pushLumLogFile->setSizePolicy(sizePolicy);
        QIcon icon5(QIcon::fromTheme(QString::fromUtf8("emblem-documents")));
        pushLumLogFile->setIcon(icon5);
        pushLumLogFile->setAutoDefault(false);

        buttonBar->addWidget(pushLumLogFile, 1, 2, 1, 1);


        gridLayout_2->addLayout(buttonBar, 1, 0, 1, 1);

        stackedWidget = new QStackedWidget(MainWindow);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setMinimumSize(QSize(0, 0));
        selectionPage = new QWidget();
        selectionPage->setObjectName("selectionPage");
        gridLayout = new QGridLayout(selectionPage);
        gridLayout->setObjectName("gridLayout");
        pushSelectSource = new QPushButton(selectionPage);
        pushSelectSource->setObjectName("pushSelectSource");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushSelectSource->sizePolicy().hasHeightForWidth());
        pushSelectSource->setSizePolicy(sizePolicy1);
        QIcon icon6(QIcon::fromTheme(QString::fromUtf8("user-home")));
        pushSelectSource->setIcon(icon6);

        gridLayout->addWidget(pushSelectSource, 1, 1, 1, 1);

        label_2 = new QLabel(selectionPage);
        label_2->setObjectName("label_2");
        label_2->setTextFormat(Qt::TextFormat::RichText);

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        label_3 = new QLabel(selectionPage);
        label_3->setObjectName("label_3");
        label_3->setTextFormat(Qt::TextFormat::RichText);

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        groupAdvOptions = new QGroupBox(selectionPage);
        groupAdvOptions->setObjectName("groupAdvOptions");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupAdvOptions->sizePolicy().hasHeightForWidth());
        groupAdvOptions->setSizePolicy(sizePolicy2);
        groupAdvOptions->setFlat(false);
        gridLayout_5 = new QGridLayout(groupAdvOptions);
        gridLayout_5->setObjectName("gridLayout_5");
        frame = new QFrame(groupAdvOptions);
        frame->setObjectName("frame");
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Plain);
        gridLayout_4 = new QGridLayout(frame);
        gridLayout_4->setObjectName("gridLayout_4");
        checkForceMakefs = new QCheckBox(frame);
        checkForceMakefs->setObjectName("checkForceMakefs");

        gridLayout_4->addWidget(checkForceMakefs, 1, 1, 1, 1);

        checkSaveBoot = new QCheckBox(frame);
        checkSaveBoot->setObjectName("checkSaveBoot");

        gridLayout_4->addWidget(checkSaveBoot, 2, 0, 1, 1);

        checkGpt = new QCheckBox(frame);
        checkGpt->setObjectName("checkGpt");

        gridLayout_4->addWidget(checkGpt, 0, 0, 1, 1);

        checkUpdate = new QCheckBox(frame);
        checkUpdate->setObjectName("checkUpdate");

        gridLayout_4->addWidget(checkUpdate, 1, 0, 1, 1);

        checkKeep = new QCheckBox(frame);
        checkKeep->setObjectName("checkKeep");

        gridLayout_4->addWidget(checkKeep, 3, 0, 1, 1);

        checkForceUsb = new QCheckBox(frame);
        checkForceUsb->setObjectName("checkForceUsb");

        gridLayout_4->addWidget(checkForceUsb, 2, 1, 1, 1);

        checkForceAutomount = new QCheckBox(frame);
        checkForceAutomount->setObjectName("checkForceAutomount");

        gridLayout_4->addWidget(checkForceAutomount, 3, 1, 1, 1);

        checkSetPmbrBoot = new QCheckBox(frame);
        checkSetPmbrBoot->setObjectName("checkSetPmbrBoot");

        gridLayout_4->addWidget(checkSetPmbrBoot, 0, 1, 1, 1);

        checkForceNofuse = new QCheckBox(frame);
        checkForceNofuse->setObjectName("checkForceNofuse");

        gridLayout_4->addWidget(checkForceNofuse, 4, 0, 1, 1);


        gridLayout_5->addWidget(frame, 2, 0, 1, 5);

        horizontalSpacer_2 = new QSpacerItem(500, 20, QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_2, 10, 3, 1, 1);

        labelSizeEsp = new QLabel(groupAdvOptions);
        labelSizeEsp->setObjectName("labelSizeEsp");

        gridLayout_5->addWidget(labelSizeEsp, 14, 0, 1, 1);

        spinBoxEsp = new QSpinBox(groupAdvOptions);
        spinBoxEsp->setObjectName("spinBoxEsp");
        spinBoxEsp->setSuffix(QString::fromUtf8(" MB"));
        spinBoxEsp->setMinimum(50);
        spinBoxEsp->setMaximum(1000);
        spinBoxEsp->setSingleStep(10);

        gridLayout_5->addWidget(spinBoxEsp, 14, 1, 1, 1);

        labelVerbosity = new QLabel(groupAdvOptions);
        labelVerbosity->setObjectName("labelVerbosity");

        gridLayout_5->addWidget(labelVerbosity, 10, 0, 1, 1);

        sliderVerbosity = new QSlider(groupAdvOptions);
        sliderVerbosity->setObjectName("sliderVerbosity");
        sliderVerbosity->setMinimum(0);
        sliderVerbosity->setMaximum(2);
        sliderVerbosity->setPageStep(1);
        sliderVerbosity->setValue(0);
        sliderVerbosity->setOrientation(Qt::Orientation::Horizontal);
        sliderVerbosity->setInvertedAppearance(false);
        sliderVerbosity->setInvertedControls(false);
        sliderVerbosity->setTickPosition(QSlider::TickPosition::TicksBelow);
        sliderVerbosity->setTickInterval(1);

        gridLayout_5->addWidget(sliderVerbosity, 10, 1, 1, 1);

        comboBoxDataFormat = new QComboBox(groupAdvOptions);
        comboBoxDataFormat->addItem(QString());
        comboBoxDataFormat->addItem(QString());
        comboBoxDataFormat->addItem(QString());
        comboBoxDataFormat->addItem(QString());
        comboBoxDataFormat->setObjectName("comboBoxDataFormat");

        gridLayout_5->addWidget(comboBoxDataFormat, 16, 1, 1, 1);

        spinBoxDataSize = new QSpinBox(groupAdvOptions);
        spinBoxDataSize->setObjectName("spinBoxDataSize");
        spinBoxDataSize->setSuffix(QString::fromUtf8("%"));
        spinBoxDataSize->setMinimum(5);
        spinBoxDataSize->setMaximum(90);
        spinBoxDataSize->setSingleStep(5);

        gridLayout_5->addWidget(spinBoxDataSize, 15, 1, 1, 1);

        checkDataFirst = new QCheckBox(groupAdvOptions);
        checkDataFirst->setObjectName("checkDataFirst");

        gridLayout_5->addWidget(checkDataFirst, 15, 0, 1, 1);

        labelFormat = new QLabel(groupAdvOptions);
        labelFormat->setObjectName("labelFormat");

        gridLayout_5->addWidget(labelFormat, 16, 0, 1, 1, Qt::AlignmentFlag::AlignRight);


        gridLayout->addWidget(groupAdvOptions, 8, 0, 1, 3);

        pushRefresh = new QPushButton(selectionPage);
        pushRefresh->setObjectName("pushRefresh");
        QIcon icon7(QIcon::fromTheme(QString::fromUtf8("view-refresh")));
        pushRefresh->setIcon(icon7);
        pushRefresh->setAutoDefault(false);

        gridLayout->addWidget(pushRefresh, 0, 2, 1, 1);

        pushOptions = new QPushButton(selectionPage);
        pushOptions->setObjectName("pushOptions");
        QIcon icon8(QIcon::fromTheme(QString::fromUtf8("go-down")));
        pushOptions->setIcon(icon8);
        pushOptions->setAutoDefault(false);

        gridLayout->addWidget(pushOptions, 1, 2, 1, 1);

        groupBox = new QGroupBox(selectionPage);
        groupBox->setObjectName("groupBox");
        sizePolicy2.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy2);
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName("verticalLayout");
        frame_3 = new QFrame(groupBox);
        frame_3->setObjectName("frame_3");
        frame_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_3->setFrameShadow(QFrame::Shadow::Plain);
        verticalLayout_2 = new QVBoxLayout(frame_3);
        verticalLayout_2->setObjectName("verticalLayout_2");
        radioNormal = new QRadioButton(frame_3);
        radioNormal->setObjectName("radioNormal");
        radioNormal->setChecked(true);

        verticalLayout_2->addWidget(radioNormal);

        radioDd = new QRadioButton(frame_3);
        radioDd->setObjectName("radioDd");

        verticalLayout_2->addWidget(radioDd);

        label_4 = new QLabel(frame_3);
        label_4->setObjectName("label_4");
        QFont font;
        font.setBold(false);
        font.setKerning(true);
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
        label_4->setWordWrap(true);

        verticalLayout_2->addWidget(label_4);


        verticalLayout->addWidget(frame_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        gridLayout->addWidget(groupBox, 3, 2, 1, 1);

        comboUsb = new QComboBox(selectionPage);
        comboUsb->setObjectName("comboUsb");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(comboUsb->sizePolicy().hasHeightForWidth());
        comboUsb->setSizePolicy(sizePolicy3);
        comboUsb->setMinimumSize(QSize(0, 32));

        gridLayout->addWidget(comboUsb, 0, 1, 1, 1);

        groupOptions = new QGroupBox(selectionPage);
        groupOptions->setObjectName("groupOptions");
        gridLayout_6 = new QGridLayout(groupOptions);
        gridLayout_6->setObjectName("gridLayout_6");
        horizontalSpacer_3 = new QSpacerItem(150, 20, QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_3, 1, 3, 1, 1);

        spinBoxSize = new QSpinBox(groupOptions);
        spinBoxSize->setObjectName("spinBoxSize");
        spinBoxSize->setSuffix(QString::fromUtf8("%"));
        spinBoxSize->setMaximum(100);
        spinBoxSize->setSingleStep(5);
        spinBoxSize->setValue(100);

        gridLayout_6->addWidget(spinBoxSize, 1, 1, 1, 1);

        label_percent = new QLabel(groupOptions);
        label_percent->setObjectName("label_percent");

        gridLayout_6->addWidget(label_percent, 1, 0, 1, 1);

        label_part_label = new QLabel(groupOptions);
        label_part_label->setObjectName("label_part_label");

        gridLayout_6->addWidget(label_part_label, 2, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout_6->addItem(horizontalSpacer, 1, 2, 1, 1);

        textLabel = new QLineEdit(groupOptions);
        textLabel->setObjectName("textLabel");
        textLabel->setInputMask(QString::fromUtf8(""));
        textLabel->setText(QString::fromUtf8(""));
        textLabel->setMaxLength(16);
        textLabel->setCursorMoveStyle(Qt::CursorMoveStyle::LogicalMoveStyle);
        textLabel->setClearButtonEnabled(true);

        gridLayout_6->addWidget(textLabel, 2, 1, 1, 2);

        frame_2 = new QFrame(groupOptions);
        frame_2->setObjectName("frame_2");
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Plain);
        gridLayout_7 = new QGridLayout(frame_2);
        gridLayout_7->setObjectName("gridLayout_7");
        checkPretend = new QCheckBox(frame_2);
        checkPretend->setObjectName("checkPretend");

        gridLayout_7->addWidget(checkPretend, 0, 0, 1, 1);

        checkCloneMode = new QCheckBox(frame_2);
        checkCloneMode->setObjectName("checkCloneMode");

        gridLayout_7->addWidget(checkCloneMode, 0, 1, 1, 1);

        checkEncrypt = new QCheckBox(frame_2);
        checkEncrypt->setObjectName("checkEncrypt");

        gridLayout_7->addWidget(checkEncrypt, 1, 0, 1, 1);

        checkCloneLive = new QCheckBox(frame_2);
        checkCloneLive->setObjectName("checkCloneLive");

        gridLayout_7->addWidget(checkCloneLive, 1, 1, 1, 1);


        gridLayout_6->addWidget(frame_2, 0, 0, 1, 4);


        gridLayout->addWidget(groupOptions, 3, 0, 1, 2);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer_2, 10, 0, 1, 3);

        stackedWidget->addWidget(selectionPage);
        outputPage = new QWidget();
        outputPage->setObjectName("outputPage");
        gridLayout_3 = new QGridLayout(outputPage);
        gridLayout_3->setObjectName("gridLayout_3");
        progBar = new QProgressBar(outputPage);
        progBar->setObjectName("progBar");
        progBar->setValue(0);
        progBar->setTextVisible(true);

        gridLayout_3->addWidget(progBar, 2, 0, 1, 2);

        outputBox = new QPlainTextEdit(outputPage);
        outputBox->setObjectName("outputBox");

        gridLayout_3->addWidget(outputBox, 0, 0, 1, 2);

        stackedWidget->addWidget(outputPage);

        gridLayout_2->addWidget(stackedWidget, 0, 0, 1, 1);

        QWidget::setTabOrder(comboUsb, pushSelectSource);
        QWidget::setTabOrder(pushSelectSource, checkPretend);
        QWidget::setTabOrder(checkPretend, checkEncrypt);
        QWidget::setTabOrder(checkEncrypt, checkCloneMode);
        QWidget::setTabOrder(checkCloneMode, checkCloneLive);
        QWidget::setTabOrder(checkCloneLive, spinBoxSize);
        QWidget::setTabOrder(spinBoxSize, textLabel);
        QWidget::setTabOrder(textLabel, checkGpt);
        QWidget::setTabOrder(checkGpt, checkUpdate);
        QWidget::setTabOrder(checkUpdate, checkSaveBoot);
        QWidget::setTabOrder(checkSaveBoot, checkKeep);
        QWidget::setTabOrder(checkKeep, checkSetPmbrBoot);
        QWidget::setTabOrder(checkSetPmbrBoot, checkForceMakefs);
        QWidget::setTabOrder(checkForceMakefs, checkForceUsb);
        QWidget::setTabOrder(checkForceUsb, checkForceAutomount);
        QWidget::setTabOrder(checkForceAutomount, pushAbout);
        QWidget::setTabOrder(pushAbout, pushHelp);
        QWidget::setTabOrder(pushHelp, pushBack);
        QWidget::setTabOrder(pushBack, pushNext);
        QWidget::setTabOrder(pushNext, pushCancel);
        QWidget::setTabOrder(pushCancel, outputBox);

        retranslateUi(MainWindow);

        pushNext->setDefault(true);
        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QDialog *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Program_Name", nullptr));
#if QT_CONFIG(tooltip)
        pushCancel->setToolTip(QCoreApplication::translate("MainWindow", "Quit application", nullptr));
#endif // QT_CONFIG(tooltip)
        pushCancel->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
#if QT_CONFIG(shortcut)
        pushCancel->setShortcut(QCoreApplication::translate("MainWindow", "Alt+N", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        pushHelp->setToolTip(QCoreApplication::translate("MainWindow", "Display help ", nullptr));
#endif // QT_CONFIG(tooltip)
        pushHelp->setText(QCoreApplication::translate("MainWindow", "Help", nullptr));
#if QT_CONFIG(shortcut)
        pushHelp->setShortcut(QCoreApplication::translate("MainWindow", "Alt+H", nullptr));
#endif // QT_CONFIG(shortcut)
        pushBack->setText(QCoreApplication::translate("MainWindow", "Back", nullptr));
        labelLogo->setText(QString());
        pushNext->setText(QCoreApplication::translate("MainWindow", "Next", nullptr));
#if QT_CONFIG(shortcut)
        pushNext->setShortcut(QString());
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        pushAbout->setToolTip(QCoreApplication::translate("MainWindow", "About this application", nullptr));
#endif // QT_CONFIG(tooltip)
        pushAbout->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
#if QT_CONFIG(shortcut)
        pushAbout->setShortcut(QCoreApplication::translate("MainWindow", "Alt+B", nullptr));
#endif // QT_CONFIG(shortcut)
        pushLumLogFile->setText(QCoreApplication::translate("MainWindow", "View Log", nullptr));
        pushSelectSource->setText(QCoreApplication::translate("MainWindow", "Select ISO", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Select Target USB Device</span></p></body></html>", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Select ISO file</span></p></body></html>", nullptr));
        groupAdvOptions->setTitle(QCoreApplication::translate("MainWindow", "Advanced Options", nullptr));
        checkForceMakefs->setText(QCoreApplication::translate("MainWindow", "Make the ext4 filesystem even if one exists", nullptr));
        checkSaveBoot->setText(QCoreApplication::translate("MainWindow", "Save the original boot directory when updating a live-usb", nullptr));
#if QT_CONFIG(tooltip)
        checkGpt->setToolTip(QCoreApplication::translate("MainWindow", "Use gpt partitioning instead of msdos", nullptr));
#endif // QT_CONFIG(tooltip)
        checkGpt->setText(QCoreApplication::translate("MainWindow", "GPT partitioning", nullptr));
        checkUpdate->setText(QCoreApplication::translate("MainWindow", "Update (only update an existing live-usb)", nullptr));
#if QT_CONFIG(tooltip)
        checkKeep->setToolTip(QCoreApplication::translate("MainWindow", "Don't replace syslinux files", nullptr));
#endif // QT_CONFIG(tooltip)
        checkKeep->setText(QCoreApplication::translate("MainWindow", "Keep syslinux files", nullptr));
        checkForceUsb->setText(QCoreApplication::translate("MainWindow", "Ignore USB/removable check", nullptr));
        checkForceAutomount->setText(QCoreApplication::translate("MainWindow", "Temporarily disable automounting", nullptr));
#if QT_CONFIG(tooltip)
        checkSetPmbrBoot->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        checkSetPmbrBoot->setText(QCoreApplication::translate("MainWindow", "Set pmbr_boot disk flag (won't boot via UEFI)", nullptr));
        checkForceNofuse->setText(QCoreApplication::translate("MainWindow", "Don't use fuseiso to mount iso files", nullptr));
        labelSizeEsp->setText(QCoreApplication::translate("MainWindow", "Size of ESP (uefi) partition:", nullptr));
        labelVerbosity->setText(QCoreApplication::translate("MainWindow", "Verbosity (less to more):", nullptr));
        comboBoxDataFormat->setItemText(0, QCoreApplication::translate("MainWindow", "exfat", nullptr));
        comboBoxDataFormat->setItemText(1, QCoreApplication::translate("MainWindow", "vfat", nullptr));
        comboBoxDataFormat->setItemText(2, QCoreApplication::translate("MainWindow", "ext4", nullptr));
        comboBoxDataFormat->setItemText(3, QCoreApplication::translate("MainWindow", "ntfs", nullptr));

        checkDataFirst->setText(QCoreApplication::translate("MainWindow", "Make separate data partition (percent)", nullptr));
        labelFormat->setText(QCoreApplication::translate("MainWindow", "Data partition format type", nullptr));
        pushRefresh->setText(QCoreApplication::translate("MainWindow", "Refresh drive list", nullptr));
        pushOptions->setText(QCoreApplication::translate("MainWindow", "Show advanced options", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Mode", nullptr));
        radioNormal->setText(QCoreApplication::translate("MainWindow", "Full-featured mode - writable Li&veUSB", nullptr));
#if QT_CONFIG(tooltip)
        radioDd->setToolTip(QCoreApplication::translate("MainWindow", "Read-only, cannot be used with persistency", nullptr));
#endif // QT_CONFIG(tooltip)
        radioDd->setText(QCoreApplication::translate("MainWindow", "Image &mode - read-only LiveUSB (dd)", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "For distros other than antiX/MX use image mode (dd).", nullptr));
        groupOptions->setTitle(QCoreApplication::translate("MainWindow", "Options", nullptr));
        label_percent->setText(QCoreApplication::translate("MainWindow", "Percent of USB-device to use:", nullptr));
        label_part_label->setText(QCoreApplication::translate("MainWindow", "Label ext partition:", nullptr));
#if QT_CONFIG(tooltip)
        checkPretend->setToolTip(QCoreApplication::translate("MainWindow", "Don't run commands that affect the usb device", nullptr));
#endif // QT_CONFIG(tooltip)
        checkPretend->setText(QCoreApplication::translate("MainWindow", "Dry run (no change to system)", nullptr));
#if QT_CONFIG(tooltip)
        checkCloneMode->setToolTip(QCoreApplication::translate("MainWindow", "clone from a mounted live-usb or iso-file.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkCloneMode->setText(QCoreApplication::translate("MainWindow", "Clone a mounted live system", nullptr));
#if QT_CONFIG(tooltip)
        checkEncrypt->setToolTip(QCoreApplication::translate("MainWindow", "Set up to boot from an encrypted partition, will prompt for pass phrase on first boot", nullptr));
#endif // QT_CONFIG(tooltip)
        checkEncrypt->setText(QCoreApplication::translate("MainWindow", "Encrypt", nullptr));
        checkCloneLive->setText(QCoreApplication::translate("MainWindow", "Clone running live system", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
