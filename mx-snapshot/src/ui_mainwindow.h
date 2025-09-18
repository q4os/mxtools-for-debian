/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
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
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *selectionPage;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_6;
    QLineEdit *textCodename;
    QSpacerItem *horizontalSpacer_4;
    QLabel *labelDistro;
    QLineEdit *textProjectName;
    QLabel *labelOptions;
    QLabel *labelKernel;
    QLabel *labelProjectName;
    QLabel *labelDate;
    QLabel *labelCodeName;
    QLineEdit *textDistroVersion;
    QComboBox *comboLiveKernel;
    QPushButton *pushReleaseDate;
    QLineEdit *textOptions;
    QSpacerItem *verticalSpacer;
    QLabel *labelIntro;
    QLabel *labelCurrentSpace;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_7;
    QLineEdit *lineEditName;
    QLabel *labelSnapshotDir;
    QLabel *labelLocation;
    QPushButton *btnSelectSnapshot;
    QLabel *labelIsoName;
    QLabel *labelDiskSpaceHelp;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_4;
    QLabel *labelUsedSpace;
    QLabel *labelFreeSpace;
    QWidget *settingsPage;
    QGridLayout *gridLayout_4;
    QLabel *labelTitleSummary;
    QFrame *frame_3;
    QGridLayout *gridLayout_3;
    QLabel *label;
    QRadioButton *radioPersonal;
    QRadioButton *radioRespin;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer_3;
    QLabel *labelSummary;
    QFrame *line_5;
    QFrame *line;
    QLabel *labelExclude;
    QFrame *line_2;
    QSpacerItem *verticalSpacer_2;
    QFrame *frame_4;
    QGridLayout *gridLayout_5;
    QComboBox *cbCompression;
    QCheckBox *checkSha512;
    QSpinBox *spinCPU;
    QLabel *labelThrottle;
    QLabel *labelChecksums;
    QLabel *labelCompression;
    QLabel *labelCPUs;
    QCheckBox *checkMd5;
    QSpinBox *spinThrottle;
    QLabel *labelOptions_2;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *btnEditExclude;
    QFrame *frame;
    QGridLayout *gridLayout;
    QCheckBox *excludeVirtualBox;
    QCheckBox *excludePictures;
    QCheckBox *excludeVideos;
    QCheckBox *excludeAll;
    QCheckBox *excludeNetworks;
    QCheckBox *excludeDownloads;
    QCheckBox *excludeDesktop;
    QCheckBox *excludeDocuments;
    QCheckBox *excludeFlatpaks;
    QCheckBox *excludeMusic;
    QCheckBox *excludeSteam;
    QWidget *outputPage;
    QVBoxLayout *verticalLayout_2;
    QTextEdit *outputBox;
    QLabel *outputLabel;
    QProgressBar *progressBar;
    QGridLayout *buttonBar;
    QPushButton *btnAbout;
    QPushButton *btnNext;
    QPushButton *btnHelp;
    QPushButton *btnCancel;
    QSpacerItem *horizontalSpacer2;
    QLabel *labelMX;
    QPushButton *btnBack;
    QSpacerItem *horizontalSpacer1;

    void setupUi(QDialog *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(984, 565);
        verticalLayout = new QVBoxLayout(MainWindow);
        verticalLayout->setObjectName("verticalLayout");
        stackedWidget = new QStackedWidget(MainWindow);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setFrameShadow(QFrame::Shadow::Plain);
        selectionPage = new QWidget();
        selectionPage->setObjectName("selectionPage");
        gridLayout_2 = new QGridLayout(selectionPage);
        gridLayout_2->setObjectName("gridLayout_2");
        groupBox = new QGroupBox(selectionPage);
        groupBox->setObjectName("groupBox");
        groupBox->setCursor(QCursor(Qt::ArrowCursor));
        groupBox->setCheckable(false);
        gridLayout_6 = new QGridLayout(groupBox);
        gridLayout_6->setObjectName("gridLayout_6");
        textCodename = new QLineEdit(groupBox);
        textCodename->setObjectName("textCodename");
        textCodename->setText(QString::fromUtf8(""));

        gridLayout_6->addWidget(textCodename, 4, 1, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(213, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_4, 7, 2, 1, 1);

        labelDistro = new QLabel(groupBox);
        labelDistro->setObjectName("labelDistro");

        gridLayout_6->addWidget(labelDistro, 2, 0, 1, 1);

        textProjectName = new QLineEdit(groupBox);
        textProjectName->setObjectName("textProjectName");
        textProjectName->setText(QString::fromUtf8(""));

        gridLayout_6->addWidget(textProjectName, 0, 1, 1, 1);

        labelOptions = new QLabel(groupBox);
        labelOptions->setObjectName("labelOptions");

        gridLayout_6->addWidget(labelOptions, 6, 0, 1, 1);

        labelKernel = new QLabel(groupBox);
        labelKernel->setObjectName("labelKernel");

        gridLayout_6->addWidget(labelKernel, 7, 0, 1, 1);

        labelProjectName = new QLabel(groupBox);
        labelProjectName->setObjectName("labelProjectName");

        gridLayout_6->addWidget(labelProjectName, 0, 0, 1, 1);

        labelDate = new QLabel(groupBox);
        labelDate->setObjectName("labelDate");

        gridLayout_6->addWidget(labelDate, 5, 0, 1, 1);

        labelCodeName = new QLabel(groupBox);
        labelCodeName->setObjectName("labelCodeName");

        gridLayout_6->addWidget(labelCodeName, 4, 0, 1, 1);

        textDistroVersion = new QLineEdit(groupBox);
        textDistroVersion->setObjectName("textDistroVersion");
        textDistroVersion->setText(QString::fromUtf8(""));

        gridLayout_6->addWidget(textDistroVersion, 2, 1, 1, 1);

        comboLiveKernel = new QComboBox(groupBox);
        comboLiveKernel->setObjectName("comboLiveKernel");

        gridLayout_6->addWidget(comboLiveKernel, 7, 1, 1, 1);

        pushReleaseDate = new QPushButton(groupBox);
        pushReleaseDate->setObjectName("pushReleaseDate");
        pushReleaseDate->setAcceptDrops(true);
#if QT_CONFIG(whatsthis)
        pushReleaseDate->setWhatsThis(QString::fromUtf8(""));
#endif // QT_CONFIG(whatsthis)
        pushReleaseDate->setStyleSheet(QString::fromUtf8("text-align: left; padding-left: 5px;"));
        pushReleaseDate->setAutoDefault(false);

        gridLayout_6->addWidget(pushReleaseDate, 5, 1, 1, 1);

        textOptions = new QLineEdit(groupBox);
        textOptions->setObjectName("textOptions");
        textOptions->setText(QString::fromUtf8(""));

        gridLayout_6->addWidget(textOptions, 6, 1, 1, 1);


        gridLayout_2->addWidget(groupBox, 14, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(verticalSpacer, 15, 0, 1, 2);

        labelIntro = new QLabel(selectionPage);
        labelIntro->setObjectName("labelIntro");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(labelIntro->sizePolicy().hasHeightForWidth());
        labelIntro->setSizePolicy(sizePolicy);
        labelIntro->setWordWrap(true);

        gridLayout_2->addWidget(labelIntro, 0, 0, 1, 2);

        labelCurrentSpace = new QLabel(selectionPage);
        labelCurrentSpace->setObjectName("labelCurrentSpace");
        labelCurrentSpace->setWordWrap(true);

        gridLayout_2->addWidget(labelCurrentSpace, 1, 0, 1, 2);

        groupBox_2 = new QGroupBox(selectionPage);
        groupBox_2->setObjectName("groupBox_2");
        gridLayout_7 = new QGridLayout(groupBox_2);
        gridLayout_7->setObjectName("gridLayout_7");
        lineEditName = new QLineEdit(groupBox_2);
        lineEditName->setObjectName("lineEditName");

        gridLayout_7->addWidget(lineEditName, 1, 1, 1, 1);

        labelSnapshotDir = new QLabel(groupBox_2);
        labelSnapshotDir->setObjectName("labelSnapshotDir");
        labelSnapshotDir->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 205);\n"
"border-color: rgb(0, 0, 0);\n"
"color: rgb(0, 0, 0);"));
        labelSnapshotDir->setFrameShape(QFrame::Shape::Box);
        labelSnapshotDir->setWordWrap(true);

        gridLayout_7->addWidget(labelSnapshotDir, 0, 1, 1, 1);

        labelLocation = new QLabel(groupBox_2);
        labelLocation->setObjectName("labelLocation");

        gridLayout_7->addWidget(labelLocation, 0, 0, 1, 1);

        btnSelectSnapshot = new QPushButton(groupBox_2);
        btnSelectSnapshot->setObjectName("btnSelectSnapshot");
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("cursor-arrow")));
        btnSelectSnapshot->setIcon(icon);

        gridLayout_7->addWidget(btnSelectSnapshot, 0, 2, 1, 1);

        labelIsoName = new QLabel(groupBox_2);
        labelIsoName->setObjectName("labelIsoName");

        gridLayout_7->addWidget(labelIsoName, 1, 0, 1, 1);


        gridLayout_2->addWidget(groupBox_2, 3, 0, 1, 2);

        labelDiskSpaceHelp = new QLabel(selectionPage);
        labelDiskSpaceHelp->setObjectName("labelDiskSpaceHelp");
        labelDiskSpaceHelp->setWordWrap(true);

        gridLayout_2->addWidget(labelDiskSpaceHelp, 5, 0, 1, 2);

        frame_2 = new QFrame(selectionPage);
        frame_2->setObjectName("frame_2");
        frame_2->setFrameShape(QFrame::Shape::Box);
        frame_2->setFrameShadow(QFrame::Shadow::Plain);
        verticalLayout_4 = new QVBoxLayout(frame_2);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        labelUsedSpace = new QLabel(frame_2);
        labelUsedSpace->setObjectName("labelUsedSpace");
        sizePolicy.setHeightForWidth(labelUsedSpace->sizePolicy().hasHeightForWidth());
        labelUsedSpace->setSizePolicy(sizePolicy);
        labelUsedSpace->setAutoFillBackground(false);
        labelUsedSpace->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 205);\n"
"border-color: rgb(0, 0, 0);\n"
"color: rgb(0, 0, 0);"));
        labelUsedSpace->setFrameShape(QFrame::Shape::NoFrame);
        labelUsedSpace->setTextFormat(Qt::TextFormat::PlainText);
        labelUsedSpace->setWordWrap(true);

        verticalLayout_4->addWidget(labelUsedSpace);

        labelFreeSpace = new QLabel(frame_2);
        labelFreeSpace->setObjectName("labelFreeSpace");
        labelFreeSpace->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 205);\n"
"border-color: rgb(0, 0, 0);\n"
"color: rgb(0, 0, 0);"));
        labelFreeSpace->setFrameShape(QFrame::Shape::NoFrame);
        labelFreeSpace->setTextFormat(Qt::TextFormat::PlainText);
        labelFreeSpace->setWordWrap(true);

        verticalLayout_4->addWidget(labelFreeSpace);


        gridLayout_2->addWidget(frame_2, 2, 0, 1, 2);

        stackedWidget->addWidget(selectionPage);
        settingsPage = new QWidget();
        settingsPage->setObjectName("settingsPage");
        gridLayout_4 = new QGridLayout(settingsPage);
        gridLayout_4->setObjectName("gridLayout_4");
        labelTitleSummary = new QLabel(settingsPage);
        labelTitleSummary->setObjectName("labelTitleSummary");
        labelTitleSummary->setText(QString::fromUtf8("TextLabel"));
        labelTitleSummary->setWordWrap(true);

        gridLayout_4->addWidget(labelTitleSummary, 0, 0, 1, 4);

        frame_3 = new QFrame(settingsPage);
        frame_3->setObjectName("frame_3");
        frame_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_3->setFrameShadow(QFrame::Shadow::Plain);
        frame_3->setLineWidth(0);
        gridLayout_3 = new QGridLayout(frame_3);
        gridLayout_3->setObjectName("gridLayout_3");
        label = new QLabel(frame_3);
        label->setObjectName("label");

        gridLayout_3->addWidget(label, 2, 1, 1, 1);

        radioPersonal = new QRadioButton(frame_3);
        radioPersonal->setObjectName("radioPersonal");
        radioPersonal->setChecked(true);

        gridLayout_3->addWidget(radioPersonal, 3, 1, 1, 1);

        radioRespin = new QRadioButton(frame_3);
        radioRespin->setObjectName("radioRespin");

        gridLayout_3->addWidget(radioRespin, 4, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer, 3, 2, 1, 1);


        gridLayout_4->addWidget(frame_3, 13, 0, 1, 4);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Policy::Fixed, QSizePolicy::Minimum);

        gridLayout_4->addItem(verticalSpacer_3, 2, 0, 1, 1);

        labelSummary = new QLabel(settingsPage);
        labelSummary->setObjectName("labelSummary");
        labelSummary->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 205);\n"
"border-color: rgb(0, 0, 0);\n"
"color: rgb(0, 0, 0);"));
        labelSummary->setFrameShape(QFrame::Shape::Box);
        labelSummary->setText(QString::fromUtf8("TextLabel"));
        labelSummary->setWordWrap(true);

        gridLayout_4->addWidget(labelSummary, 1, 0, 1, 4);

        line_5 = new QFrame(settingsPage);
        line_5->setObjectName("line_5");
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line_5, 7, 0, 1, 4);

        line = new QFrame(settingsPage);
        line->setObjectName("line");
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line, 11, 0, 1, 4);

        labelExclude = new QLabel(settingsPage);
        labelExclude->setObjectName("labelExclude");
        labelExclude->setWordWrap(true);

        gridLayout_4->addWidget(labelExclude, 4, 0, 1, 4);

        line_2 = new QFrame(settingsPage);
        line_2->setObjectName("line_2");
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout_4->addWidget(line_2, 3, 0, 1, 4);

        verticalSpacer_2 = new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(verticalSpacer_2, 14, 0, 1, 4);

        frame_4 = new QFrame(settingsPage);
        frame_4->setObjectName("frame_4");
        frame_4->setFrameShape(QFrame::Shape::StyledPanel);
        frame_4->setFrameShadow(QFrame::Shadow::Plain);
        frame_4->setLineWidth(0);
        gridLayout_5 = new QGridLayout(frame_4);
        gridLayout_5->setObjectName("gridLayout_5");
        cbCompression = new QComboBox(frame_4);
        cbCompression->setObjectName("cbCompression");

        gridLayout_5->addWidget(cbCompression, 1, 1, 1, 3);

        checkSha512 = new QCheckBox(frame_4);
        checkSha512->setObjectName("checkSha512");

        gridLayout_5->addWidget(checkSha512, 3, 3, 1, 1);

        spinCPU = new QSpinBox(frame_4);
        spinCPU->setObjectName("spinCPU");
        spinCPU->setMinimumSize(QSize(60, 0));
        spinCPU->setMinimum(1);
        spinCPU->setMaximum(999);

        gridLayout_5->addWidget(spinCPU, 1, 6, 1, 1);

        labelThrottle = new QLabel(frame_4);
        labelThrottle->setObjectName("labelThrottle");
        labelThrottle->setLineWidth(1);

        gridLayout_5->addWidget(labelThrottle, 3, 5, 1, 1);

        labelChecksums = new QLabel(frame_4);
        labelChecksums->setObjectName("labelChecksums");

        gridLayout_5->addWidget(labelChecksums, 3, 0, 1, 1);

        labelCompression = new QLabel(frame_4);
        labelCompression->setObjectName("labelCompression");

        gridLayout_5->addWidget(labelCompression, 1, 0, 1, 1);

        labelCPUs = new QLabel(frame_4);
        labelCPUs->setObjectName("labelCPUs");

        gridLayout_5->addWidget(labelCPUs, 1, 5, 1, 1);

        checkMd5 = new QCheckBox(frame_4);
        checkMd5->setObjectName("checkMd5");

        gridLayout_5->addWidget(checkMd5, 3, 1, 1, 1);

        spinThrottle = new QSpinBox(frame_4);
        spinThrottle->setObjectName("spinThrottle");

        gridLayout_5->addWidget(spinThrottle, 3, 6, 1, 1);

        labelOptions_2 = new QLabel(frame_4);
        labelOptions_2->setObjectName("labelOptions_2");

        gridLayout_5->addWidget(labelOptions_2, 0, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(30, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_2, 1, 4, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_3, 1, 7, 1, 1);


        gridLayout_4->addWidget(frame_4, 8, 0, 1, 4);

        btnEditExclude = new QPushButton(settingsPage);
        btnEditExclude->setObjectName("btnEditExclude");
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnEditExclude->sizePolicy().hasHeightForWidth());
        btnEditExclude->setSizePolicy(sizePolicy1);
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("edit")));
        btnEditExclude->setIcon(icon1);

        gridLayout_4->addWidget(btnEditExclude, 6, 0, 1, 1);

        frame = new QFrame(settingsPage);
        frame->setObjectName("frame");
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Sunken);
        frame->setLineWidth(0);
        frame->setMidLineWidth(1);
        gridLayout = new QGridLayout(frame);
        gridLayout->setObjectName("gridLayout");
        excludeVirtualBox = new QCheckBox(frame);
        excludeVirtualBox->setObjectName("excludeVirtualBox");
        excludeVirtualBox->setText(QString::fromUtf8("VirtualBox"));
        excludeVirtualBox->setChecked(false);

        gridLayout->addWidget(excludeVirtualBox, 1, 4, 1, 1);

        excludePictures = new QCheckBox(frame);
        excludePictures->setObjectName("excludePictures");

        gridLayout->addWidget(excludePictures, 1, 1, 1, 1);

        excludeVideos = new QCheckBox(frame);
        excludeVideos->setObjectName("excludeVideos");

        gridLayout->addWidget(excludeVideos, 1, 3, 1, 1);

        excludeAll = new QCheckBox(frame);
        excludeAll->setObjectName("excludeAll");

        gridLayout->addWidget(excludeAll, 2, 0, 1, 1);

        excludeNetworks = new QCheckBox(frame);
        excludeNetworks->setObjectName("excludeNetworks");

        gridLayout->addWidget(excludeNetworks, 1, 0, 1, 1);

        excludeDownloads = new QCheckBox(frame);
        excludeDownloads->setObjectName("excludeDownloads");

        gridLayout->addWidget(excludeDownloads, 0, 2, 1, 1);

        excludeDesktop = new QCheckBox(frame);
        excludeDesktop->setObjectName("excludeDesktop");

        gridLayout->addWidget(excludeDesktop, 0, 0, 1, 1);

        excludeDocuments = new QCheckBox(frame);
        excludeDocuments->setObjectName("excludeDocuments");

        gridLayout->addWidget(excludeDocuments, 0, 1, 1, 1);

        excludeFlatpaks = new QCheckBox(frame);
        excludeFlatpaks->setObjectName("excludeFlatpaks");

        gridLayout->addWidget(excludeFlatpaks, 0, 3, 1, 1);

        excludeMusic = new QCheckBox(frame);
        excludeMusic->setObjectName("excludeMusic");

        gridLayout->addWidget(excludeMusic, 0, 4, 1, 1);

        excludeSteam = new QCheckBox(frame);
        excludeSteam->setObjectName("excludeSteam");
        excludeSteam->setText(QString::fromUtf8("Steam"));
        excludeSteam->setChecked(false);

        gridLayout->addWidget(excludeSteam, 1, 2, 1, 1);


        gridLayout_4->addWidget(frame, 5, 0, 1, 4);

        stackedWidget->addWidget(settingsPage);
        outputPage = new QWidget();
        outputPage->setObjectName("outputPage");
        verticalLayout_2 = new QVBoxLayout(outputPage);
        verticalLayout_2->setObjectName("verticalLayout_2");
        outputBox = new QTextEdit(outputPage);
        outputBox->setObjectName("outputBox");
        outputBox->setUndoRedoEnabled(false);
        outputBox->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
        outputBox->setReadOnly(true);
        outputBox->setHtml(QString::fromUtf8("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Noto Sans'; font-size:10.5pt;\"><br /></p></body></html>"));
        outputBox->setOverwriteMode(true);
        outputBox->setAcceptRichText(false);
        outputBox->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
        outputBox->setPlaceholderText(QString::fromUtf8(""));

        verticalLayout_2->addWidget(outputBox);

        outputLabel = new QLabel(outputPage);
        outputLabel->setObjectName("outputLabel");
        outputLabel->setWordWrap(true);

        verticalLayout_2->addWidget(outputLabel);

        progressBar = new QProgressBar(outputPage);
        progressBar->setObjectName("progressBar");
        progressBar->setMaximum(100);
        progressBar->setValue(0);
        progressBar->setTextVisible(false);

        verticalLayout_2->addWidget(progressBar);

        stackedWidget->addWidget(outputPage);

        verticalLayout->addWidget(stackedWidget);

        buttonBar = new QGridLayout();
        buttonBar->setSpacing(5);
        buttonBar->setObjectName("buttonBar");
        buttonBar->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        buttonBar->setContentsMargins(0, 0, 0, 0);
        btnAbout = new QPushButton(MainWindow);
        btnAbout->setObjectName("btnAbout");
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(btnAbout->sizePolicy().hasHeightForWidth());
        btnAbout->setSizePolicy(sizePolicy2);
        QIcon icon2(QIcon::fromTheme(QString::fromUtf8("help-about")));
        btnAbout->setIcon(icon2);
        btnAbout->setAutoDefault(true);

        buttonBar->addWidget(btnAbout, 0, 0, 1, 1);

        btnNext = new QPushButton(MainWindow);
        btnNext->setObjectName("btnNext");
        sizePolicy2.setHeightForWidth(btnNext->sizePolicy().hasHeightForWidth());
        btnNext->setSizePolicy(sizePolicy2);
        QIcon icon3(QIcon::fromTheme(QString::fromUtf8("go-next")));
        btnNext->setIcon(icon3);
        btnNext->setAutoDefault(true);

        buttonBar->addWidget(btnNext, 0, 8, 1, 1);

        btnHelp = new QPushButton(MainWindow);
        btnHelp->setObjectName("btnHelp");
        sizePolicy2.setHeightForWidth(btnHelp->sizePolicy().hasHeightForWidth());
        btnHelp->setSizePolicy(sizePolicy2);
        QIcon icon4(QIcon::fromTheme(QString::fromUtf8("help-contents")));
        btnHelp->setIcon(icon4);

        buttonBar->addWidget(btnHelp, 0, 1, 1, 1);

        btnCancel = new QPushButton(MainWindow);
        btnCancel->setObjectName("btnCancel");
        sizePolicy2.setHeightForWidth(btnCancel->sizePolicy().hasHeightForWidth());
        btnCancel->setSizePolicy(sizePolicy2);
        QIcon icon5(QIcon::fromTheme(QString::fromUtf8("window-close")));
        btnCancel->setIcon(icon5);
        btnCancel->setAutoDefault(true);

        buttonBar->addWidget(btnCancel, 0, 10, 1, 1);

        horizontalSpacer2 = new QSpacerItem(100, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Minimum);

        buttonBar->addItem(horizontalSpacer2, 0, 5, 1, 1);

        labelMX = new QLabel(MainWindow);
        labelMX->setObjectName("labelMX");
        labelMX->setMaximumSize(QSize(32, 32));
        labelMX->setMidLineWidth(0);
        labelMX->setPixmap(QPixmap(QString::fromUtf8(":/icons/logo.svg")));
        labelMX->setScaledContents(true);

        buttonBar->addWidget(labelMX, 0, 3, 1, 1);

        btnBack = new QPushButton(MainWindow);
        btnBack->setObjectName("btnBack");
        sizePolicy2.setHeightForWidth(btnBack->sizePolicy().hasHeightForWidth());
        btnBack->setSizePolicy(sizePolicy2);
        QIcon icon6(QIcon::fromTheme(QString::fromUtf8("go-previous")));
        btnBack->setIcon(icon6);

        buttonBar->addWidget(btnBack, 0, 7, 1, 1);

        horizontalSpacer1 = new QSpacerItem(100, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Minimum);

        buttonBar->addItem(horizontalSpacer1, 0, 2, 1, 1);


        verticalLayout->addLayout(buttonBar);

        QWidget::setTabOrder(btnSelectSnapshot, lineEditName);
        QWidget::setTabOrder(lineEditName, textProjectName);
        QWidget::setTabOrder(textProjectName, textDistroVersion);
        QWidget::setTabOrder(textDistroVersion, textCodename);
        QWidget::setTabOrder(textCodename, textOptions);
        QWidget::setTabOrder(textOptions, btnAbout);
        QWidget::setTabOrder(btnAbout, btnHelp);
        QWidget::setTabOrder(btnHelp, btnBack);
        QWidget::setTabOrder(btnBack, btnNext);
        QWidget::setTabOrder(btnNext, btnCancel);
        QWidget::setTabOrder(btnCancel, excludeDocuments);
        QWidget::setTabOrder(excludeDocuments, excludeDownloads);
        QWidget::setTabOrder(excludeDownloads, btnEditExclude);
        QWidget::setTabOrder(btnEditExclude, cbCompression);
        QWidget::setTabOrder(cbCompression, checkMd5);
        QWidget::setTabOrder(checkMd5, radioPersonal);
        QWidget::setTabOrder(radioPersonal, radioRespin);
        QWidget::setTabOrder(radioRespin, outputBox);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(1);
        btnNext->setDefault(true);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QDialog *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MX Snapshot", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Optional customization", nullptr));
        labelDistro->setText(QCoreApplication::translate("MainWindow", "Release version:", nullptr));
        labelOptions->setText(QCoreApplication::translate("MainWindow", "Boot options:", nullptr));
        labelKernel->setText(QCoreApplication::translate("MainWindow", "Live kernel:", nullptr));
        labelProjectName->setText(QCoreApplication::translate("MainWindow", "Project name:", nullptr));
        labelDate->setText(QCoreApplication::translate("MainWindow", "Release date:", nullptr));
        labelCodeName->setText(QCoreApplication::translate("MainWindow", "Release codename:", nullptr));
        pushReleaseDate->setText(QCoreApplication::translate("MainWindow", "Current date", nullptr));
        labelIntro->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p>Snapshot is a utility that creates a bootable image (ISO) of your working system that you can use for storage or distribution. You can continue working with undemanding applications while it is running.<br/></p></body></html>", nullptr));
        labelCurrentSpace->setText(QCoreApplication::translate("MainWindow", "Used space on / (root) and /home partitions:", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "Location and ISO name", nullptr));
        labelSnapshotDir->setText(QString());
        labelLocation->setText(QCoreApplication::translate("MainWindow", "Snapshot location:", nullptr));
        btnSelectSnapshot->setText(QCoreApplication::translate("MainWindow", "Select a different snapshot directory", nullptr));
        labelIsoName->setText(QCoreApplication::translate("MainWindow", "Snapshot name:", nullptr));
        labelDiskSpaceHelp->setText(QString());
        labelUsedSpace->setText(QString());
        labelFreeSpace->setText(QString());
        label->setText(QCoreApplication::translate("MainWindow", "Type of snapshot:", nullptr));
        radioPersonal->setText(QCoreApplication::translate("MainWindow", "Preserving accounts (for personal backup)", nullptr));
#if QT_CONFIG(tooltip)
        radioRespin->setToolTip(QCoreApplication::translate("MainWindow", "<html><head/><body><p>This option will reset &quot;demo&quot; and &quot;root&quot; passwords to the MX Linux defaults and will not copy any personal accounts created.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        radioRespin->setText(QCoreApplication::translate("MainWindow", "Resetting accounts (for distribution to others)", nullptr));
        labelExclude->setText(QCoreApplication::translate("MainWindow", "You can also exclude certain directories by ticking the common choices below, or by clicking on the button to directly edit /etc/mx-snapshot-exclude.list.", nullptr));
        checkSha512->setText(QCoreApplication::translate("MainWindow", "sha512", nullptr));
#if QT_CONFIG(tooltip)
        labelThrottle->setToolTip(QCoreApplication::translate("MainWindow", "Throttle the I/O input rate by the given percentage.", nullptr));
#endif // QT_CONFIG(tooltip)
        labelThrottle->setText(QCoreApplication::translate("MainWindow", "I/O throttle:", nullptr));
        labelChecksums->setText(QCoreApplication::translate("MainWindow", "Calculate checksums:", nullptr));
        labelCompression->setText(QCoreApplication::translate("MainWindow", "ISO compression scheme:", nullptr));
        labelCPUs->setText(QCoreApplication::translate("MainWindow", "Number of CPU cores to use:", nullptr));
        checkMd5->setText(QCoreApplication::translate("MainWindow", "md5", nullptr));
        labelOptions_2->setText(QCoreApplication::translate("MainWindow", "Options:", nullptr));
        btnEditExclude->setText(QCoreApplication::translate("MainWindow", "Edit Exclusion File", nullptr));
        excludePictures->setText(QCoreApplication::translate("MainWindow", "Pictures", nullptr));
        excludeVideos->setText(QCoreApplication::translate("MainWindow", "Videos", nullptr));
        excludeAll->setText(QCoreApplication::translate("MainWindow", "All of the above", nullptr));
#if QT_CONFIG(statustip)
        excludeNetworks->setStatusTip(QCoreApplication::translate("MainWindow", "exclude network configurations", nullptr));
#endif // QT_CONFIG(statustip)
        excludeNetworks->setText(QCoreApplication::translate("MainWindow", "Networks", nullptr));
        excludeDownloads->setText(QCoreApplication::translate("MainWindow", "Downloads", nullptr));
        excludeDesktop->setText(QCoreApplication::translate("MainWindow", "Desktop", nullptr));
        excludeDocuments->setText(QCoreApplication::translate("MainWindow", "Documents", nullptr));
        excludeFlatpaks->setText(QCoreApplication::translate("MainWindow", "Flatpaks", nullptr));
        excludeMusic->setText(QCoreApplication::translate("MainWindow", "Music", nullptr));
        outputLabel->setText(QString());
#if QT_CONFIG(tooltip)
        btnAbout->setToolTip(QCoreApplication::translate("MainWindow", "About this application", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAbout->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
#if QT_CONFIG(shortcut)
        btnAbout->setShortcut(QCoreApplication::translate("MainWindow", "Alt+B", nullptr));
#endif // QT_CONFIG(shortcut)
        btnNext->setText(QCoreApplication::translate("MainWindow", "Next", nullptr));
#if QT_CONFIG(shortcut)
        btnNext->setShortcut(QString());
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        btnHelp->setToolTip(QCoreApplication::translate("MainWindow", "Display help ", nullptr));
#endif // QT_CONFIG(tooltip)
        btnHelp->setText(QCoreApplication::translate("MainWindow", "Help", nullptr));
#if QT_CONFIG(shortcut)
        btnHelp->setShortcut(QCoreApplication::translate("MainWindow", "Alt+H", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        btnCancel->setToolTip(QCoreApplication::translate("MainWindow", "Quit application", nullptr));
#endif // QT_CONFIG(tooltip)
        btnCancel->setText(QCoreApplication::translate("MainWindow", "Cancel", nullptr));
#if QT_CONFIG(shortcut)
        btnCancel->setShortcut(QCoreApplication::translate("MainWindow", "Alt+N", nullptr));
#endif // QT_CONFIG(shortcut)
        labelMX->setText(QString());
        btnBack->setText(QCoreApplication::translate("MainWindow", "Back", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
