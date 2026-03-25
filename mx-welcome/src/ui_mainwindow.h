/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>
#include "flatbutton.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QGridLayout *gridLayout_4;
    QGridLayout *buttonBar;
    QLabel *labelMX;
    QPushButton *buttonCancel;
    QPushButton *buttonAbout;
    QSpacerItem *horizontalSpacer1;
    QSpacerItem *horizontalSpacer2;
    QGridLayout *gridLayout_2;
    QSpacerItem *horizontalSpacer_18;
    QTabWidget *tabWidget;
    QWidget *Welcome;
    QGridLayout *gridLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_16;
    QSpacerItem *horizontalSpacer_17;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_20;
    QSpacerItem *horizontalSpacer_21;
    QGridLayout *gridLayout;
    QSpacerItem *horizontalSpacer_11;
    QSpacerItem *horizontalSpacer_15;
    QSpacerItem *horizontalSpacer_8;
    QSpacerItem *horizontalSpacer_14;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *horizontalSpacer_7;
    QSpacerItem *horizontalSpacer_10;
    QSpacerItem *horizontalSpacer_13;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_12;
    QSpacerItem *horizontalSpacer_5;
    QSpacerItem *horizontalSpacer_9;
    QSpacerItem *horizontalSpacer_2;
    FlatButton *buttonSetup;
    FlatButton *buttonPanelOrient;
    FlatButton *buttonTools;
    FlatButton *buttonWiki;
    FlatButton *buttonManual;
    FlatButton *buttonFAQ;
    FlatButton *buttonTour;
    FlatButton *buttonPackageInstall;
    FlatButton *buttonContribute;
    FlatButton *buttonVideo;
    FlatButton *buttonForum;
    QWidget *About;
    QGridLayout *gridLayout_6;
    QGridLayout *gridLayout_5;
    QLabel *label_3;
    QLabel *labelSupportUntil;
    QPushButton *ButtonQSI;
    QPushButton *buttonTOS;
    QLabel *labelMXversion;
    QLabel *labelTOS;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *LabelDebianVersion;
    QTextBrowser *textBrowser;
    QFrame *line;
    QLabel *labelDesktopVersion;
    QLabel *label_5;
    QCheckBox *checkBox;
    QSpacerItem *horizontalSpacer_19;
    QLabel *labelLoginInfo;
    QFrame *line_2;
    QFrame *line_3;
    QLabel *labelTitle;
    QLabel *labelgraphic;

    void setupUi(QDialog *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(674, 653);
        MainWindow->setSizeGripEnabled(false);
        MainWindow->setModal(false);
        gridLayout_4 = new QGridLayout(MainWindow);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        buttonBar = new QGridLayout();
        buttonBar->setObjectName(QString::fromUtf8("buttonBar"));
        buttonBar->setContentsMargins(9, -1, 9, 9);
        labelMX = new QLabel(MainWindow);
        labelMX->setObjectName(QString::fromUtf8("labelMX"));
        labelMX->setMaximumSize(QSize(32, 32));
        labelMX->setMidLineWidth(0);
        labelMX->setPixmap(QPixmap(QString::fromUtf8(":/icons/logo.png")));
        labelMX->setScaledContents(true);

        buttonBar->addWidget(labelMX, 0, 3, 1, 1);

        buttonCancel = new QPushButton(MainWindow);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(buttonCancel->sizePolicy().hasHeightForWidth());
        buttonCancel->setSizePolicy(sizePolicy);
        buttonCancel->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        QString iconThemeName = QString::fromUtf8("window-close");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon = QIcon::fromTheme(iconThemeName);
        } else {
            icon.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        buttonCancel->setIcon(icon);
        buttonCancel->setAutoDefault(true);

        buttonBar->addWidget(buttonCancel, 0, 8, 1, 1);

        buttonAbout = new QPushButton(MainWindow);
        buttonAbout->setObjectName(QString::fromUtf8("buttonAbout"));
        sizePolicy.setHeightForWidth(buttonAbout->sizePolicy().hasHeightForWidth());
        buttonAbout->setSizePolicy(sizePolicy);
        buttonAbout->setFocusPolicy(Qt::NoFocus);
        QIcon icon1;
        iconThemeName = QString::fromUtf8("help-about");
        if (QIcon::hasThemeIcon(iconThemeName)) {
            icon1 = QIcon::fromTheme(iconThemeName);
        } else {
            icon1.addFile(QString::fromUtf8("."), QSize(), QIcon::Normal, QIcon::Off);
        }
        buttonAbout->setIcon(icon1);
        buttonAbout->setAutoDefault(true);

        buttonBar->addWidget(buttonAbout, 0, 0, 1, 1);

        horizontalSpacer1 = new QSpacerItem(183, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonBar->addItem(horizontalSpacer1, 0, 1, 1, 1);

        horizontalSpacer2 = new QSpacerItem(100, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonBar->addItem(horizontalSpacer2, 0, 5, 1, 1);


        gridLayout_4->addLayout(buttonBar, 5, 0, 1, 2);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(9, -1, 9, -1);
        horizontalSpacer_18 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_18, 4, 0, 1, 1);

        tabWidget = new QTabWidget(MainWindow);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setMinimumSize(QSize(654, 0));
        tabWidget->setStyleSheet(QString::fromUtf8("QTabBar::tab { height: 30px; width: 326px; } "));
        Welcome = new QWidget();
        Welcome->setObjectName(QString::fromUtf8("Welcome"));
        gridLayout_3 = new QGridLayout(Welcome);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_16 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_16);

        horizontalSpacer_17 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_17);


        gridLayout_3->addLayout(horizontalLayout_4, 0, 0, 1, 2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalSpacer_20 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_20);

        horizontalSpacer_21 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_21);


        gridLayout_3->addLayout(horizontalLayout_5, 2, 0, 1, 2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setVerticalSpacing(0);
        horizontalSpacer_11 = new QSpacerItem(62, 44, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_11, 5, 0, 1, 1);

        horizontalSpacer_15 = new QSpacerItem(61, 44, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_15, 5, 6, 1, 2);

        horizontalSpacer_8 = new QSpacerItem(129, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_8, 1, 2, 1, 3);

        horizontalSpacer_14 = new QSpacerItem(135, 44, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_14, 5, 2, 1, 3);

        horizontalSpacer_4 = new QSpacerItem(62, 44, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 4, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(61, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 3, 6, 1, 2);

        horizontalSpacer_6 = new QSpacerItem(61, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_6, 2, 6, 1, 2);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_7, 1, 0, 1, 1);

        horizontalSpacer_10 = new QSpacerItem(129, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_10, 3, 2, 1, 3);

        horizontalSpacer_13 = new QSpacerItem(61, 44, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_13, 4, 6, 1, 2);

        horizontalSpacer_3 = new QSpacerItem(62, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 2, 0, 1, 1);

        horizontalSpacer_12 = new QSpacerItem(135, 44, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_12, 4, 2, 1, 3);

        horizontalSpacer_5 = new QSpacerItem(129, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_5, 2, 2, 1, 3);

        horizontalSpacer_9 = new QSpacerItem(61, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_9, 1, 7, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(62, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 3, 0, 1, 1);

        buttonSetup = new FlatButton(Welcome);
        buttonSetup->setObjectName(QString::fromUtf8("buttonSetup"));
        buttonSetup->setFocusPolicy(Qt::NoFocus);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/setup.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonSetup->setIcon(icon2);
        buttonSetup->setIconSize(QSize(42, 42));
        buttonSetup->setAutoDefault(true);
        buttonSetup->setFlat(true);

        gridLayout->addWidget(buttonSetup, 0, 3, 1, 1);

        buttonPanelOrient = new FlatButton(Welcome);
        buttonPanelOrient->setObjectName(QString::fromUtf8("buttonPanelOrient"));
        buttonPanelOrient->setFocusPolicy(Qt::NoFocus);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/mxwelcome-icon-panel-orient.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonPanelOrient->setIcon(icon3);
        buttonPanelOrient->setIconSize(QSize(42, 42));
        buttonPanelOrient->setAutoDefault(true);
        buttonPanelOrient->setFlat(true);

        gridLayout->addWidget(buttonPanelOrient, 5, 1, 1, 1);

        buttonTools = new FlatButton(Welcome);
        buttonTools->setObjectName(QString::fromUtf8("buttonTools"));
        buttonTools->setFocusPolicy(Qt::NoFocus);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/tools.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonTools->setIcon(icon4);
        buttonTools->setIconSize(QSize(42, 42));
        buttonTools->setAutoDefault(true);
        buttonTools->setFlat(true);

        gridLayout->addWidget(buttonTools, 4, 1, 1, 1);

        buttonWiki = new FlatButton(Welcome);
        buttonWiki->setObjectName(QString::fromUtf8("buttonWiki"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(buttonWiki->sizePolicy().hasHeightForWidth());
        buttonWiki->setSizePolicy(sizePolicy1);
        buttonWiki->setFocusPolicy(Qt::NoFocus);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/wiki.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonWiki->setIcon(icon5);
        buttonWiki->setIconSize(QSize(42, 42));
        buttonWiki->setAutoDefault(true);
        buttonWiki->setFlat(true);

        gridLayout->addWidget(buttonWiki, 3, 1, 1, 1);

        buttonManual = new FlatButton(Welcome);
        buttonManual->setObjectName(QString::fromUtf8("buttonManual"));
        sizePolicy1.setHeightForWidth(buttonManual->sizePolicy().hasHeightForWidth());
        buttonManual->setSizePolicy(sizePolicy1);
        buttonManual->setFocusPolicy(Qt::NoFocus);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/users-manual.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonManual->setIcon(icon6);
        buttonManual->setIconSize(QSize(42, 42));
        buttonManual->setAutoDefault(true);
        buttonManual->setFlat(true);

        gridLayout->addWidget(buttonManual, 2, 1, 1, 1);

        buttonFAQ = new FlatButton(Welcome);
        buttonFAQ->setObjectName(QString::fromUtf8("buttonFAQ"));
        buttonFAQ->setFocusPolicy(Qt::NoFocus);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/faq.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonFAQ->setIcon(icon7);
        buttonFAQ->setIconSize(QSize(42, 42));
        buttonFAQ->setAutoDefault(true);
        buttonFAQ->setFlat(true);

        gridLayout->addWidget(buttonFAQ, 1, 1, 1, 1);

        buttonTour = new FlatButton(Welcome);
        buttonTour->setObjectName(QString::fromUtf8("buttonTour"));
        buttonTour->setFocusPolicy(Qt::NoFocus);
        buttonTour->setIconSize(QSize(42, 42));
        buttonTour->setAutoDefault(true);
        buttonTour->setFlat(true);

        gridLayout->addWidget(buttonTour, 5, 5, 1, 1);

        buttonPackageInstall = new FlatButton(Welcome);
        buttonPackageInstall->setObjectName(QString::fromUtf8("buttonPackageInstall"));
        buttonPackageInstall->setFocusPolicy(Qt::NoFocus);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/mxwelcome-icon-packageinstaller.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonPackageInstall->setIcon(icon8);
        buttonPackageInstall->setIconSize(QSize(42, 42));
        buttonPackageInstall->setAutoDefault(true);
        buttonPackageInstall->setFlat(true);

        gridLayout->addWidget(buttonPackageInstall, 4, 5, 1, 1);

        buttonContribute = new FlatButton(Welcome);
        buttonContribute->setObjectName(QString::fromUtf8("buttonContribute"));
        buttonContribute->setFocusPolicy(Qt::NoFocus);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/contribute.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonContribute->setIcon(icon9);
        buttonContribute->setIconSize(QSize(42, 42));
        buttonContribute->setAutoDefault(true);
        buttonContribute->setFlat(true);

        gridLayout->addWidget(buttonContribute, 3, 5, 1, 1);

        buttonVideo = new FlatButton(Welcome);
        buttonVideo->setObjectName(QString::fromUtf8("buttonVideo"));
        sizePolicy1.setHeightForWidth(buttonVideo->sizePolicy().hasHeightForWidth());
        buttonVideo->setSizePolicy(sizePolicy1);
        buttonVideo->setFocusPolicy(Qt::NoFocus);
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/icons/videos.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonVideo->setIcon(icon10);
        buttonVideo->setIconSize(QSize(42, 42));
        buttonVideo->setAutoDefault(true);
        buttonVideo->setFlat(true);

        gridLayout->addWidget(buttonVideo, 2, 5, 1, 1);

        buttonForum = new FlatButton(Welcome);
        buttonForum->setObjectName(QString::fromUtf8("buttonForum"));
        sizePolicy1.setHeightForWidth(buttonForum->sizePolicy().hasHeightForWidth());
        buttonForum->setSizePolicy(sizePolicy1);
        buttonForum->setFocusPolicy(Qt::NoFocus);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/icons/forums.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonForum->setIcon(icon11);
        buttonForum->setIconSize(QSize(42, 42));
        buttonForum->setAutoDefault(true);
        buttonForum->setFlat(true);

        gridLayout->addWidget(buttonForum, 1, 5, 1, 1);


        gridLayout_3->addLayout(gridLayout, 1, 0, 1, 2);

        tabWidget->addTab(Welcome, QString());
        About = new QWidget();
        About->setObjectName(QString::fromUtf8("About"));
        gridLayout_6 = new QGridLayout(About);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        label_3 = new QLabel(About);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_5->addWidget(label_3, 0, 2, 1, 1);

        labelSupportUntil = new QLabel(About);
        labelSupportUntil->setObjectName(QString::fromUtf8("labelSupportUntil"));
        labelSupportUntil->setWordWrap(true);

        gridLayout_5->addWidget(labelSupportUntil, 1, 3, 1, 1);

        ButtonQSI = new QPushButton(About);
        ButtonQSI->setObjectName(QString::fromUtf8("ButtonQSI"));

        gridLayout_5->addWidget(ButtonQSI, 7, 0, 1, 4);

        buttonTOS = new QPushButton(About);
        buttonTOS->setObjectName(QString::fromUtf8("buttonTOS"));
        buttonTOS->setStyleSheet(QString::fromUtf8(""));
        buttonTOS->setFlat(false);

        gridLayout_5->addWidget(buttonTOS, 4, 0, 1, 4);

        labelMXversion = new QLabel(About);
        labelMXversion->setObjectName(QString::fromUtf8("labelMXversion"));
        labelMXversion->setWordWrap(true);

        gridLayout_5->addWidget(labelMXversion, 0, 1, 1, 1);

        labelTOS = new QLabel(About);
        labelTOS->setObjectName(QString::fromUtf8("labelTOS"));
        labelTOS->setWordWrap(true);

        gridLayout_5->addWidget(labelTOS, 3, 0, 1, 4);

        label = new QLabel(About);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_5->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(About);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_5->addWidget(label_2, 1, 0, 1, 1);

        label_4 = new QLabel(About);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_5->addWidget(label_4, 1, 2, 1, 1);

        LabelDebianVersion = new QLabel(About);
        LabelDebianVersion->setObjectName(QString::fromUtf8("LabelDebianVersion"));
        LabelDebianVersion->setWordWrap(true);

        gridLayout_5->addWidget(LabelDebianVersion, 1, 1, 1, 1);

        textBrowser = new QTextBrowser(About);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setMaximumSize(QSize(16777215, 81));

        gridLayout_5->addWidget(textBrowser, 6, 0, 1, 4);

        line = new QFrame(About);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout_5->addWidget(line, 2, 0, 1, 4);

        labelDesktopVersion = new QLabel(About);
        labelDesktopVersion->setObjectName(QString::fromUtf8("labelDesktopVersion"));
        labelDesktopVersion->setWordWrap(true);

        gridLayout_5->addWidget(labelDesktopVersion, 0, 3, 1, 1);

        label_5 = new QLabel(About);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_5->addWidget(label_5, 5, 0, 1, 4);


        gridLayout_6->addLayout(gridLayout_5, 0, 0, 1, 1);

        tabWidget->addTab(About, QString());

        gridLayout_2->addWidget(tabWidget, 3, 0, 1, 3);

        checkBox = new QCheckBox(MainWindow);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setFocusPolicy(Qt::NoFocus);

        gridLayout_2->addWidget(checkBox, 4, 1, 1, 1);

        horizontalSpacer_19 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_19, 4, 2, 1, 1);

        labelLoginInfo = new QLabel(MainWindow);
        labelLoginInfo->setObjectName(QString::fromUtf8("labelLoginInfo"));

        gridLayout_2->addWidget(labelLoginInfo, 5, 0, 1, 3);

        line_2 = new QFrame(MainWindow);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout_2->addWidget(line_2, 6, 0, 1, 3);

        line_3 = new QFrame(MainWindow);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        gridLayout_2->addWidget(line_3, 2, 0, 1, 3);

        labelTitle = new QLabel(MainWindow);
        labelTitle->setObjectName(QString::fromUtf8("labelTitle"));
        labelTitle->setText(QString::fromUtf8("<html><head/><body><p align=\"center\"><span style=\" font-size:14pt; font-weight:600;\">MX-18 &quot;Continuum&quot;</span></p></body></html>"));
        labelTitle->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(labelTitle, 1, 0, 1, 3);


        gridLayout_4->addLayout(gridLayout_2, 1, 0, 1, 2);

        labelgraphic = new QLabel(MainWindow);
        labelgraphic->setObjectName(QString::fromUtf8("labelgraphic"));
        labelgraphic->setPixmap(QPixmap(QString::fromUtf8("../../../../usr/share/mx-welcome/header.jpg")));
        labelgraphic->setScaledContents(true);

        gridLayout_4->addWidget(labelgraphic, 0, 0, 1, 2);


        retranslateUi(MainWindow);

        buttonCancel->setDefault(true);
        buttonAbout->setDefault(false);
        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QDialog *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MX Welcome", nullptr));
        labelMX->setText(QString());
#if QT_CONFIG(tooltip)
        buttonCancel->setToolTip(QCoreApplication::translate("MainWindow", "Quit application", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonCancel->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
#if QT_CONFIG(shortcut)
        buttonCancel->setShortcut(QCoreApplication::translate("MainWindow", "Alt+N", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        buttonAbout->setToolTip(QCoreApplication::translate("MainWindow", "About this application", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonAbout->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
#if QT_CONFIG(shortcut)
        buttonAbout->setShortcut(QCoreApplication::translate("MainWindow", "Alt+B", nullptr));
#endif // QT_CONFIG(shortcut)
        buttonSetup->setText(QCoreApplication::translate("MainWindow", "Install MX Linux", nullptr));
        buttonPanelOrient->setText(QCoreApplication::translate("MainWindow", "Tweak (Panel, etc...)", nullptr));
        buttonTools->setText(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        buttonWiki->setText(QCoreApplication::translate("MainWindow", "Wiki", nullptr));
        buttonManual->setText(QCoreApplication::translate("MainWindow", "Users Manual", nullptr));
        buttonFAQ->setText(QCoreApplication::translate("MainWindow", "FAQ", nullptr));
        buttonTour->setText(QCoreApplication::translate("MainWindow", "Tour", nullptr));
        buttonPackageInstall->setText(QCoreApplication::translate("MainWindow", "Popular Apps", nullptr));
        buttonContribute->setText(QCoreApplication::translate("MainWindow", "Contribute", nullptr));
        buttonVideo->setText(QCoreApplication::translate("MainWindow", "Videos", nullptr));
        buttonForum->setText(QCoreApplication::translate("MainWindow", "Forums", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(Welcome), QCoreApplication::translate("MainWindow", "Welcome", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "DESKTOP", nullptr));
        labelSupportUntil->setText(QString());
        ButtonQSI->setText(QCoreApplication::translate("MainWindow", "Quick-System-Info Full Report", nullptr));
        buttonTOS->setText(QCoreApplication::translate("MainWindow", "Terms of Use", nullptr));
        labelMXversion->setText(QString());
        labelTOS->setText(QCoreApplication::translate("MainWindow", "The name \342\200\234MX Linux\342\200\235 is covered by Linux Foundation Sublicense No. 20140605-0483. We develop software that is covered by a free license that can be examined in the Wiki list. We also include software developed by others that is under a free license.", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "MX VERSION", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "DEBIAN VERSION:", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "SUPPORTED UNTIL:", nullptr));
        LabelDebianVersion->setText(QString());
        textBrowser->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Noto Sans'; font-size:12pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10.5pt;\"><br /></p></body></html>", nullptr));
        labelDesktopVersion->setText(QString());
        label_5->setText(QCoreApplication::translate("MainWindow", "SHORT SYSTEM REPORT:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(About), QCoreApplication::translate("MainWindow", "About", nullptr));
        checkBox->setText(QCoreApplication::translate("MainWindow", "Show this dialog at start up", nullptr));
        labelLoginInfo->setText(QString());
        labelgraphic->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
