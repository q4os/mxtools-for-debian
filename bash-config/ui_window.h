/********************************************************************************
** Form generated from reading UI file 'window.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Window
{
  public:
	QVBoxLayout* verticalLayout;
	QTabWidget* tabWidget_Tabs;
	QHBoxLayout* horizontalLayout;
	QPushButton* pushButton_3;
	QPushButton* pushButton_2;
	QLabel* label;
	QPushButton* pushButton;

	void setupUi(QWidget* Window)
	{
		if (Window->objectName().isEmpty())
			Window->setObjectName(QStringLiteral("Window"));
		Window->resize(400, 300);
		verticalLayout = new QVBoxLayout(Window);
		verticalLayout->setSpacing(6);
		verticalLayout->setContentsMargins(11, 11, 11, 11);
		verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
		tabWidget_Tabs = new QTabWidget(Window);
		tabWidget_Tabs->setObjectName(QStringLiteral("tabWidget_Tabs"));

		verticalLayout->addWidget(tabWidget_Tabs);

		horizontalLayout = new QHBoxLayout();
		horizontalLayout->setSpacing(6);
		horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
		pushButton_3 = new QPushButton(Window);
		pushButton_3->setObjectName(QStringLiteral("pushButton_3"));

		horizontalLayout->addWidget(pushButton_3);

		pushButton_2 = new QPushButton(Window);
		pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

		horizontalLayout->addWidget(pushButton_2);

		label = new QLabel(Window);
		label->setObjectName(QStringLiteral("label"));
		label->setAlignment(Qt::AlignCenter);

		horizontalLayout->addWidget(label);

		pushButton = new QPushButton(Window);
		pushButton->setObjectName(QStringLiteral("pushButton"));

		horizontalLayout->addWidget(pushButton);

		verticalLayout->addLayout(horizontalLayout);

		retranslateUi(Window);

		QMetaObject::connectSlotsByName(Window);
	} // setupUi

	void retranslateUi(QWidget* Window)
	{
		Window->setWindowTitle(QApplication::translate("Window", "Window", Q_NULLPTR));
		pushButton_3->setText(QApplication::translate("Window", "PushButton", Q_NULLPTR));
		pushButton_2->setText(QApplication::translate("Window", "PushButton", Q_NULLPTR));
		label->setText(QApplication::translate("Window", "Logo", Q_NULLPTR));
		pushButton->setText(QApplication::translate("Window", "PushButton", Q_NULLPTR));
	} // retranslateUi
};

namespace Ui
{
	class Window : public Ui_Window
	{
	};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WINDOW_H
