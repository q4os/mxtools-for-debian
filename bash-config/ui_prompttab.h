/********************************************************************************
** Form generated from reading UI file 'prompttab.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROMPTTAB_H
#define UI_PROMPTTAB_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PromptTab
{
  public:
	QVBoxLayout* verticalLayout;
	QScrollArea* scrollArea;
	QWidget* scrollAreaWidgetContents;
	QVBoxLayout* verticalLayout_2;
	QComboBox* comboBox_SelectPromptProvider;
	QStackedWidget* stackedWidget;
	QWidget* page_FancyPrompts;
	QVBoxLayout* verticalLayout_3;
	QComboBox* comboBox_SelectFancyPrompt;
	QCheckBox* checkBox_CompactLargePrompts;
	QCheckBox* checkBox_ParensInstead;
	QCheckBox* checkBox_NoColor;
	QCheckBox* checkBox_BoldLines;
	QCheckBox* checkBox_DoubleLines;
	QCheckBox* checkBox_DisableUnicode;
	QCheckBox* checkBox_MutedColors;
	QHBoxLayout* horizontalLayout_5;
	QLabel* label_5;
	QLineEdit* lineEdit_TimeFormatText;
	QHBoxLayout* horizontalLayout;
	QLabel* label;
	QLineEdit* lineEdit_DateFormatText;
	QHBoxLayout* horizontalLayout_2;
	QLabel* label_2;
	QSpinBox* spinBox_RightMargin;
	QHBoxLayout* horizontalLayout_3;
	QLabel* label_3;
	QSpinBox* spinBox_ExtraNewlinesBeforePrompt;
	QHBoxLayout* horizontalLayout_4;
	QLabel* label_4;
	QLineEdit* lineEdit_PromptText;
	QHBoxLayout* horizontalLayout_7;
	QLabel* label_7;
	QLineEdit* lineEdit_TitleText;
	QWidget* page_2;

	void setupUi(QWidget* PromptTab)
	{
		if (PromptTab->objectName().isEmpty())
			PromptTab->setObjectName(QStringLiteral("PromptTab"));
		PromptTab->resize(460, 496);
		verticalLayout = new QVBoxLayout(PromptTab);
		verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
		scrollArea = new QScrollArea(PromptTab);
		scrollArea->setObjectName(QStringLiteral("scrollArea"));
		scrollArea->setWidgetResizable(true);
		scrollAreaWidgetContents = new QWidget();
		scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
		scrollAreaWidgetContents->setGeometry(QRect(0, -4, 426, 588));
		verticalLayout_2 = new QVBoxLayout(scrollAreaWidgetContents);
		verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
		comboBox_SelectPromptProvider = new QComboBox(scrollAreaWidgetContents);
		comboBox_SelectPromptProvider->setObjectName(QStringLiteral("comboBox_SelectPromptProvider"));

		verticalLayout_2->addWidget(comboBox_SelectPromptProvider);

		stackedWidget = new QStackedWidget(scrollAreaWidgetContents);
		stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
		page_FancyPrompts = new QWidget();
		page_FancyPrompts->setObjectName(QStringLiteral("page_FancyPrompts"));
		verticalLayout_3 = new QVBoxLayout(page_FancyPrompts);
		verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
		comboBox_SelectFancyPrompt = new QComboBox(page_FancyPrompts);
		comboBox_SelectFancyPrompt->setObjectName(QStringLiteral("comboBox_SelectFancyPrompt"));

		verticalLayout_3->addWidget(comboBox_SelectFancyPrompt);

		checkBox_CompactLargePrompts = new QCheckBox(page_FancyPrompts);
		checkBox_CompactLargePrompts->setObjectName(QStringLiteral("checkBox_CompactLargePrompts"));

		verticalLayout_3->addWidget(checkBox_CompactLargePrompts);

		checkBox_ParensInstead = new QCheckBox(page_FancyPrompts);
		checkBox_ParensInstead->setObjectName(QStringLiteral("checkBox_ParensInstead"));

		verticalLayout_3->addWidget(checkBox_ParensInstead);

		checkBox_NoColor = new QCheckBox(page_FancyPrompts);
		checkBox_NoColor->setObjectName(QStringLiteral("checkBox_NoColor"));

		verticalLayout_3->addWidget(checkBox_NoColor);

		checkBox_BoldLines = new QCheckBox(page_FancyPrompts);
		checkBox_BoldLines->setObjectName(QStringLiteral("checkBox_BoldLines"));

		verticalLayout_3->addWidget(checkBox_BoldLines);

		checkBox_DoubleLines = new QCheckBox(page_FancyPrompts);
		checkBox_DoubleLines->setObjectName(QStringLiteral("checkBox_DoubleLines"));

		verticalLayout_3->addWidget(checkBox_DoubleLines);

		checkBox_DisableUnicode = new QCheckBox(page_FancyPrompts);
		checkBox_DisableUnicode->setObjectName(QStringLiteral("checkBox_DisableUnicode"));

		verticalLayout_3->addWidget(checkBox_DisableUnicode);

		checkBox_MutedColors = new QCheckBox(page_FancyPrompts);
		checkBox_MutedColors->setObjectName(QStringLiteral("checkBox_MutedColors"));

		verticalLayout_3->addWidget(checkBox_MutedColors);

		horizontalLayout_5 = new QHBoxLayout();
		horizontalLayout_5->setSpacing(10);
		horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
		horizontalLayout_5->setContentsMargins(5, 5, 5, 5);
		label_5 = new QLabel(page_FancyPrompts);
		label_5->setObjectName(QStringLiteral("label_5"));

		horizontalLayout_5->addWidget(label_5);

		lineEdit_TimeFormatText = new QLineEdit(page_FancyPrompts);
		lineEdit_TimeFormatText->setObjectName(QStringLiteral("lineEdit_TimeFormatText"));

		horizontalLayout_5->addWidget(lineEdit_TimeFormatText);

		verticalLayout_3->addLayout(horizontalLayout_5);

		horizontalLayout = new QHBoxLayout();
		horizontalLayout->setSpacing(10);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
		horizontalLayout->setContentsMargins(5, 5, 5, 5);
		label = new QLabel(page_FancyPrompts);
		label->setObjectName(QStringLiteral("label"));

		horizontalLayout->addWidget(label);

		lineEdit_DateFormatText = new QLineEdit(page_FancyPrompts);
		lineEdit_DateFormatText->setObjectName(QStringLiteral("lineEdit_DateFormatText"));

		horizontalLayout->addWidget(lineEdit_DateFormatText);

		verticalLayout_3->addLayout(horizontalLayout);

		horizontalLayout_2 = new QHBoxLayout();
		horizontalLayout_2->setSpacing(10);
		horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
		horizontalLayout_2->setContentsMargins(5, 5, 5, 5);
		label_2 = new QLabel(page_FancyPrompts);
		label_2->setObjectName(QStringLiteral("label_2"));
		QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
		label_2->setSizePolicy(sizePolicy);

		horizontalLayout_2->addWidget(label_2);

		spinBox_RightMargin = new QSpinBox(page_FancyPrompts);
		spinBox_RightMargin->setObjectName(QStringLiteral("spinBox_RightMargin"));
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(spinBox_RightMargin->sizePolicy().hasHeightForWidth());
		spinBox_RightMargin->setSizePolicy(sizePolicy1);

		horizontalLayout_2->addWidget(spinBox_RightMargin);

		verticalLayout_3->addLayout(horizontalLayout_2);

		horizontalLayout_3 = new QHBoxLayout();
		horizontalLayout_3->setSpacing(10);
		horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
		horizontalLayout_3->setContentsMargins(5, 5, 5, 5);
		label_3 = new QLabel(page_FancyPrompts);
		label_3->setObjectName(QStringLiteral("label_3"));

		horizontalLayout_3->addWidget(label_3);

		spinBox_ExtraNewlinesBeforePrompt = new QSpinBox(page_FancyPrompts);
		spinBox_ExtraNewlinesBeforePrompt->setObjectName(QStringLiteral("spinBox_ExtraNewlinesBeforePrompt"));

		horizontalLayout_3->addWidget(spinBox_ExtraNewlinesBeforePrompt);

		verticalLayout_3->addLayout(horizontalLayout_3);

		horizontalLayout_4 = new QHBoxLayout();
		horizontalLayout_4->setSpacing(10);
		horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
		horizontalLayout_4->setContentsMargins(5, 5, 5, 5);
		label_4 = new QLabel(page_FancyPrompts);
		label_4->setObjectName(QStringLiteral("label_4"));

		horizontalLayout_4->addWidget(label_4);

		lineEdit_PromptText = new QLineEdit(page_FancyPrompts);
		lineEdit_PromptText->setObjectName(QStringLiteral("lineEdit_PromptText"));

		horizontalLayout_4->addWidget(lineEdit_PromptText);

		verticalLayout_3->addLayout(horizontalLayout_4);

		horizontalLayout_7 = new QHBoxLayout();
		horizontalLayout_7->setSpacing(10);
		horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
		horizontalLayout_7->setContentsMargins(5, 5, 5, 5);
		label_7 = new QLabel(page_FancyPrompts);
		label_7->setObjectName(QStringLiteral("label_7"));

		horizontalLayout_7->addWidget(label_7);

		lineEdit_TitleText = new QLineEdit(page_FancyPrompts);
		lineEdit_TitleText->setObjectName(QStringLiteral("lineEdit_TitleText"));

		horizontalLayout_7->addWidget(lineEdit_TitleText);

		verticalLayout_3->addLayout(horizontalLayout_7);

		stackedWidget->addWidget(page_FancyPrompts);
		page_2 = new QWidget();
		page_2->setObjectName(QStringLiteral("page_2"));
		stackedWidget->addWidget(page_2);

		verticalLayout_2->addWidget(stackedWidget);

		scrollArea->setWidget(scrollAreaWidgetContents);

		verticalLayout->addWidget(scrollArea);

		retranslateUi(PromptTab);

		stackedWidget->setCurrentIndex(0);

		QMetaObject::connectSlotsByName(PromptTab);
	} // setupUi

	void retranslateUi(QWidget* PromptTab)
	{
		PromptTab->setWindowTitle(QApplication::translate("PromptTab", "Form", Q_NULLPTR));
		comboBox_SelectPromptProvider->clear();
		comboBox_SelectPromptProvider->insertItems(0, QStringList() << QApplication::translate("PromptTab", "Fancy Prompt", Q_NULLPTR));
		comboBox_SelectFancyPrompt->clear();
		comboBox_SelectFancyPrompt->insertItems(0, QStringList() << QApplication::translate("PromptTab", "Tiny", Q_NULLPTR) << QApplication::translate("PromptTab", "Std", Q_NULLPTR) << QApplication::translate("PromptTab", "Color", Q_NULLPTR) << QApplication::translate("PromptTab", "Gentoo", Q_NULLPTR) << QApplication::translate("PromptTab", "Dir", Q_NULLPTR) << QApplication::translate("PromptTab", "Med", Q_NULLPTR) << QApplication::translate("PromptTab", "Narrow", Q_NULLPTR) << QApplication::translate("PromptTab", "Wide", Q_NULLPTR) << QApplication::translate("PromptTab", "Fancy", Q_NULLPTR) << QApplication::translate("PromptTab", "Zee", Q_NULLPTR) << QApplication::translate("PromptTab", "Date", Q_NULLPTR) << QApplication::translate("PromptTab", "Curl", Q_NULLPTR));
		checkBox_CompactLargePrompts->setText(QApplication::translate("PromptTab", "Make The Larger Prompts Smaller", Q_NULLPTR));
		checkBox_ParensInstead->setText(QApplication::translate("PromptTab", "Use Parentheses Instead Of Square Brackets", Q_NULLPTR));
		checkBox_NoColor->setText(QApplication::translate("PromptTab", "No Colors(Overrides All Other Color Options)", Q_NULLPTR));
		checkBox_BoldLines->setText(QApplication::translate("PromptTab", "Use Bold Lines", Q_NULLPTR));
		checkBox_DoubleLines->setText(QApplication::translate("PromptTab", "Use Double Lines", Q_NULLPTR));
		checkBox_DisableUnicode->setText(QApplication::translate("PromptTab", "Disable Unicode", Q_NULLPTR));
		checkBox_MutedColors->setText(QApplication::translate("PromptTab", "Muted Colors", Q_NULLPTR));
		label_5->setText(QApplication::translate("PromptTab", "Time Format", Q_NULLPTR));
		label->setText(QApplication::translate("PromptTab", "Date Format", Q_NULLPTR));
		label_2->setText(QApplication::translate("PromptTab", "Right Margin", Q_NULLPTR));
		label_3->setText(QApplication::translate("PromptTab", "Extra Newlines Before Prompt", Q_NULLPTR));
		label_4->setText(QApplication::translate("PromptTab", "Prompt Text", Q_NULLPTR));
		label_7->setText(QApplication::translate("PromptTab", "Title Text", Q_NULLPTR));
	} // retranslateUi
};

namespace Ui
{
	class PromptTab : public Ui_PromptTab
	{
	};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROMPTTAB_H
