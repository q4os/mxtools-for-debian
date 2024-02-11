#include "prompttab.h"

#include "searcher.h"
#include <QMap>
#include <QMessageBox>
#include <QProcess>

#include <QCheckBox>
#include <QColorDialog>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPalette>
#include <QPushButton>
#include <QVBoxLayout>

#include "datetimeformatting.h"
#include "fuzzybashstream.h"

PromptTab::PromptTab()
	: Tab("Prompt")
{
	DEBUG_ENTER(PromptTab::PromptTab);
	ui = new Ui::PromptTab;
	setWidget(new QWidget);
	ui->setupUi(widget());

	ui->listWidget_PromptCustom->setToolTip("Double click for more options");

	ui->lineEdit_DateFormatText->setCompleter(new QCompleter(DateTimeFormatWords));
	ui->lineEdit_TimeFormatText->setCompleter(new QCompleter(DateTimeFormatWords));

	connect(ui->comboBox_SelectPromptProvider, &QComboBox::currentTextChanged, [=](QString text) {
		Q_UNUSED(text)
		if (ui->comboBox_SelectPromptProvider->currentIndex() == 1)
		{
			ui->stackedWidget->setCurrentIndex(0);
		}
		else if (ui->comboBox_SelectPromptProvider->currentIndex() == 0)
		{
			ui->stackedWidget->setCurrentIndex(1);
		}
		else if (ui->comboBox_SelectPromptProvider->currentIndex() == 2)
		{
			ui->stackedWidget->setCurrentIndex(2);
		}
	});

	connect(ui->pushButton_PromptCustomAdd, &QPushButton::clicked, [=]() {
		QListWidgetItem* item = CustomItemSelectorDialog::getItem(widget());
		if (item != nullptr)
		{
			ui->listWidget_PromptCustom->addItem(item);
		}
	});

	connect(ui->pushButton_PromptCustomRemove, &QPushButton::clicked, [=]() {
		QList<QListWidgetItem*> items = ui->listWidget_PromptCustom->selectedItems();
		for (QListWidgetItem* item : items)
		{
			if (item != nullptr)
				delete ui->listWidget_PromptCustom->takeItem(ui->listWidget_PromptCustom->row(item));
		}
	});

	connect(ui->pushButton_PromptCustomEdit, &QPushButton::clicked, [=]() {
		if (ui->listWidget_PromptCustom->currentItem() == nullptr) {
			return;
		}
		CustomPromptItemEditor::edit(static_cast<CustomPromptItem*>(ui->listWidget_PromptCustom->currentItem()));
	});

	connect(ui->pushButton_PromptCustomUp, &QPushButton::clicked, [=]() {
		QList<QListWidgetItem*> items = ui->listWidget_PromptCustom->selectedItems();
		if (items.size() == 0)
			return;
		QListWidgetItem* item = items.at(0);
		if (item == nullptr)
			return;
		ui->listWidget_PromptCustom->setCurrentItem(item);
		int index = ui->listWidget_PromptCustom->currentRow();
		if (index == 0)
			return;
		item = ui->listWidget_PromptCustom->takeItem(index);
		ui->listWidget_PromptCustom->insertItem(index - 1, item);
		ui->listWidget_PromptCustom->setCurrentItem(item);
	});

	connect(ui->pushButton_PromptCustomDown, &QPushButton::clicked, [=]() {
		QList<QListWidgetItem*> items = ui->listWidget_PromptCustom->selectedItems();
		if (items.size() == 0)
			return;
		QListWidgetItem* item = items.at(0);
		if (item == nullptr)
			return;
		ui->listWidget_PromptCustom->setCurrentItem(item);
		int index = ui->listWidget_PromptCustom->currentRow();
		if (index == ui->listWidget_PromptCustom->count() - 1)
			return;
		item = ui->listWidget_PromptCustom->takeItem(index);
		ui->listWidget_PromptCustom->insertItem(index + 1, item);
		ui->listWidget_PromptCustom->setCurrentItem(item);
	});

	connect(ui->listWidget_PromptCustom, &QListWidget::itemDoubleClicked, [=](QListWidgetItem* item) {
		CustomPromptItem* citem = dynamic_cast<CustomPromptItem*>(item);
		CustomPromptItemEditor::edit(citem);
	});

	DEBUG_EXIT(PromptTab::PromptTab);
}

PromptTab::~PromptTab()
{
	DEBUG_ENTER(PromptTab::~PromptTab);
	delete widget();
	delete ui;
	DEBUG_EXIT(PromptTab::~PromptTab);
}

void PromptTab::setup(const BashrcSource data)
{
	DEBUG_ENTER(PromptTab::setup);
	QString program = data.program;
	/* Maybe check in bashrc in case they don't want to reconfigure their fancy prompt */
    FuzzyBashStream stream{program};
    auto tokens = stream.tokens();
	Searcher searcher(&program, Searcher::StateCheckDoubleQuotations | Searcher::StateCheckSingleQuotations | Searcher::StateCheckSpecialQuotations);
    if (std::any_of(tokens.begin(), tokens.end(), [this](const FuzzyBashStream::Token& token){
       return token.type() == FuzzyBashStream::TokenComment && token.content().contains(customPromptCommentString);
    }))
	{
        // TODO start storing xml in seperate file
		ui->comboBox_SelectPromptProvider->setCurrentText("Custom");
		QStringList lines = program.split("\n");
		QRegularExpression regex{"^\\s{0,}#\\s{0,}<.+>"};
		for (QString line : lines)
		{
			if (line.contains(regex))
			{
				QString trimmed = line.mid(line.indexOf("#") + 1);
				QListWidgetItem* item = xmlToItem(trimmed);
				if (!item)
				{
					DEBUG << "got null item on line: " << trimmed;
					continue;
				}
				ui->listWidget_PromptCustom->addItem(item);
			}
		}
	}
    else if (!(std::any_of(tokens.begin(), tokens.end(), [](const FuzzyBashStream::Token& token){
        return token.type() == FuzzyBashStream::TokenCommand && token.content().startsWith("prompt-");
    })))
	{
		ui->comboBox_SelectPromptProvider->setCurrentText("Default");
		ui->stackedWidget->setCurrentIndex(1);
		DEBUG << "Selected Default Prompt Provider";
		QRegularExpression regexFindPS1("PS1=('|\")(.{0,})(\\1)");
		// need to do the same for the program's bashrc
		QRegularExpressionMatchIterator iter = regexFindPS1.globalMatch(data.program);
		QRegularExpressionMatch lastMatch;
		bool foundPS1 = iter.hasNext();
		while (iter.hasNext())
		{
			lastMatch = iter.next();
		}
		QString programQuotedText = lastMatch.captured(2);
		DEBUG_VAR(programQuotedText);
		QString PS1 = bashInteractiveVariable("PS1");
		DEBUG_VAR(PS1);
		if (!PS1.isEmpty())
		{
			if (programQuotedText == PS1.remove("\\n") && foundPS1)
			{
				ui->checkBox_RemoveAllNewlines->setChecked(true);
			}
			else
			{
				ui->checkBox_RemoveAllNewlines->setChecked(false);
			}
		}
		else
			DEBUG << "PS1 variable not found in QProcess bash -i";
	}
	else
	{
		DEBUG << "Selected Fancy Prompt Prompt Provider";
		ui->comboBox_SelectPromptProvider->setCurrentText("Fancy Prompt");
		program.append("source /usr/local/bin/fancy-prompt.bash\n");
		ui->stackedWidget->setCurrentIndex(0);

        // Can assume that this will work because of previous check
        auto commandToken = *std::find_if(tokens.begin(), tokens.end(), [](const FuzzyBashStream::Token& token){
            return token.type() == FuzzyBashStream::TokenCommand && token.content().startsWith("prompt-");
        });

        FuzzyBashStream commandStream{commandToken.content(), FuzzyBashStream::ParseDisableCommandGrouping};
        auto tokens = commandStream.tokens();

        auto command = tokens.first();
        auto promptType = command.content().mid(QStringLiteral("prompt-").size()).at(0).toUpper();
        ui->comboBox_SelectFancyPrompt->setCurrentText(promptType);

        QMap<QString, QCheckBox*> flags;
        flags["--ascii"] = ui->checkBox_DisableUnicode;
        flags["--mute"] = ui->checkBox_MutedColors;
        flags["--bold"] = ui->checkBox_BoldLines;
        flags["--compact"] = ui->checkBox_CompactLargePrompts;
        flags["--compact2"] = ui->checkBox_CompactLargePrompts2;
        flags["--double"] = ui->checkBox_DoubleLines;
        flags["--nocolor"] = ui->checkBox_NoColor;
        flags["--parens"] = ui->checkBox_ParensInstead;

        for(auto flag : flags.keys())
        {
            auto exists = std::any_of(tokens.begin(), tokens.end(), [flag](const FuzzyBashStream::Token& token){
                return token.content() == flag;
            });
            flags[flag]->setChecked(exists);
        }

        QMap<QString, QLineEdit*> textOpts;
        textOpts["--date"] = ui->lineEdit_DateFormatText;
        textOpts["--time"] = ui->lineEdit_TimeFormatText;
        textOpts["--prompt"] = ui->lineEdit_PromptText;
        textOpts["--title"] = ui->lineEdit_TitleText;

        for(auto opt : textOpts.keys())
        {
            auto token = std::find_if(tokens.begin(), tokens.end(), [opt](const FuzzyBashStream::Token& token){
                return token.content() == opt;
            });
            token++; // skip --opt
            token++; // skip '='
            auto stringToken = *token;
            auto string = stringToken.content();
            if(stringToken.type() == FuzzyBashStream::TokenQuoted)
            {
                string = stringToken.quoted();
            }
            textOpts[opt]->setText(string);
        }

        QMap<QString, QSpinBox*> numOpts;
        numOpts["--right"] = ui->spinBox_RightMargin;
        numOpts["--lines"] = ui->spinBox_ExtraNewlinesBeforePrompt;


        for(auto opt : numOpts.keys())
        {
            auto token = std::find_if(tokens.begin(), tokens.end(), [opt](const FuzzyBashStream::Token& token){
                return token.content() == opt;
            });
            token++; // skip --opt
            token++; // skip '='
            auto stringToken = *token;
            auto string = stringToken.content();
            if(stringToken.type() == FuzzyBashStream::TokenQuoted)
            {
                string = stringToken.quoted();
            }
            numOpts[opt]->setValue(string.toInt());
        }
    }

	DEBUG_EXIT(PromptTab::setup);
}

int rgbToAnsi256(QColor color)
{
	int ansi256 = 1;
	// taken from: https://stackoverflow.com/questions/15682537/ansi-color-specific-rgb-sequence-bash
	bool finishedAnsi256 = false;
	if (color.red() == color.green() && color.green() == color.blue())
	{
		if (color.red() < 8)
		{
			if (!finishedAnsi256)
				ansi256 = 16;
			finishedAnsi256 = true;
		}
		if (color.red() > 248)
		{
			if (!finishedAnsi256)
				ansi256 = 231;
			finishedAnsi256 = true;
		}
		if (!finishedAnsi256)
			ansi256 = qRound(static_cast<float>((color.red() - 8) / 247) * 24) + 232;
	}
	if (!finishedAnsi256)
	{
		ansi256 = 16 +
			(36 * qRound(static_cast<float>(color.red() / 255 * 5))) +
			(6 * qRound(static_cast<float>(color.green() / 255 * 5))) +
			qRound(static_cast<float>(color.blue() / 255 * 5));
	}

	return ansi256;
}

BashrcSource PromptTab::exec(const BashrcSource data)
{
	DEBUG_ENTER(PromptTab::exec);
	BashrcSource rtn;
	// Copy contructor seems to work may be issue in future
	rtn = data;

	QString promptCommand;

	if (ui->comboBox_SelectPromptProvider->currentText() == "Fancy Prompt")
	{
		promptCommand.append("source /usr/local/bin/fancy-prompts.bash");
		promptCommand.append("\nprompt-");
		promptCommand.append(ui->comboBox_SelectFancyPrompt->currentText().toLower());
		promptCommand.append(' ');
		promptCommand.append((ui->checkBox_BoldLines->isChecked()) ? "--bold " : "");
		promptCommand.append((ui->checkBox_CompactLargePrompts->isChecked()) ? "--compact " : "");
		promptCommand.append((ui->checkBox_CompactLargePrompts2->isChecked()) ? "--compact2 " : "");
		promptCommand.append((ui->checkBox_DisableUnicode->isChecked()) ? "--ascii " : "");
		promptCommand.append((ui->checkBox_DoubleLines->isChecked()) ? "--double " : "");
		promptCommand.append((ui->checkBox_MutedColors->isChecked()) ? "--mute " : "");
		promptCommand.append((ui->checkBox_NoColor->isChecked()) ? "--nocolor " : "");
		promptCommand.append((ui->checkBox_ParensInstead->isChecked()) ? "--parens " : "");
		promptCommand.append(QString(" --lines=\"%1\"").arg(ui->spinBox_ExtraNewlinesBeforePrompt->value()));
		promptCommand.append(QString(" --right=\"%1\"").arg(ui->spinBox_RightMargin->value()));
		promptCommand.append(QString(" --date=\"%1\"").arg(ui->lineEdit_DateFormatText->text()));
		promptCommand.append(QString(" --time=\"%1\"").arg(ui->lineEdit_TimeFormatText->text()));
		promptCommand.append(QString(" --prompt=\"%1\"").arg(ui->lineEdit_PromptText->text()));
		promptCommand.append(QString(" --title=\"%1\"").arg(ui->lineEdit_TitleText->text()));
		rtn.program.append("\n");
		rtn.program.append(promptCommand);
	}
	else if (ui->comboBox_SelectPromptProvider->currentText() == "Custom")
	{
		QString lines;
		lines.append(customPromptCommentString + "\n");
		lines.append("export PS1=\"\"\n");
		QList<CustomPromptItem*> forLater;
		for (int i = 0; i < ui->listWidget_PromptCustom->count(); i++)
		{
			QString code{""};
			CustomPromptItem* item = static_cast<CustomPromptItem*>(ui->listWidget_PromptCustom->item(i));
			forLater.push_back(item);
			/// stringifying item
			if (item->type() == CustomPromptItemType::Text)
			{
				TextItem* obj = static_cast<TextItem*>(item);
				QColor foreground = obj->propertyForeground();
				QColor background = obj->propertyBackground();
				bool bold = obj->propertyBold();
				QString text = obj->propertyText();
				text = terminalEncode(text);
				int ansi256 = rgbToAnsi256(foreground);
				//    int ansi16 = (color.red()*6/256)*36 + (color.green()*6/256)*6 + (color.blue()*6/256);

				if (bold)
				{
					code.append("export PS1=\"$PS1\\[\\e[1m\\]\"\n");
				}

				if (obj->propertyForegroundEnabled())
				{
					code.append("if [ $COLORTERM == \"truecolor\" ]; then\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[38;2;%1;%2;%3m\\]\"\n").arg(foreground.red()).arg(foreground.green()).arg(foreground.blue()));
					code.append("else\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[38;5;%1m\\]\"\n").arg(ansi256));
					code.append("fi\n");
				}
				ansi256 = rgbToAnsi256(background);

				if (obj->propertyBackgroundEnabled())
				{
					code.append("if [ $COLORTERM == \"truecolor\" ]; then\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[48;2;%1;%2;%3m\\]\"\n").arg(background.red()).arg(background.green()).arg(background.blue()));
					code.append("else\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[48;5;%1m\\]\"\n").arg(ansi256));
					code.append("fi\n");
				}
				// add text

				code.append(QString("export PS1=\"$PS1%1\"\n").arg(text));

				lines.append(code);
			}
			else if (item->type() == CustomPromptItemType::Special)
			{
				SpecialItem* obj = static_cast<SpecialItem*>(item);
				QColor foreground = obj->propertyForeground();
				QColor background = obj->propertyBackground();
				bool bold = obj->propertyBold();
				auto type = obj->itemType();
				int ansi256 = rgbToAnsi256(foreground);
				//    int ansi16 = (color.red()*6/256)*36 + (color.green()*6/256)*6 + (color.blue()*6/256);

				if (bold)
				{
					code.append("export PS1=\"$PS1\\[\\e[1m\\]\"\n");
				}

				if (obj->propertyForegroundEnabled())
				{
					code.append("if [ $COLORTERM == \"truecolor\" ]; then\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[38;2;%1;%2;%3m\\]\"\n").arg(foreground.red()).arg(foreground.green()).arg(foreground.blue()));
					code.append("else\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[38;5;%1m\\]\"\n").arg(ansi256));
					code.append("fi\n");
				}
				ansi256 = rgbToAnsi256(background);

				if (obj->propertyBackgroundEnabled())
				{
					code.append("if [ $COLORTERM == \"truecolor\" ]; then\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[48;2;%1;%2;%3m\\]\"\n").arg(background.red()).arg(background.green()).arg(background.blue()));
					code.append("else\n");
					code.append(QString("\texport PS1=\"$PS1\\[\\033[48;5;%1m\\]\"\n").arg(ansi256));
					code.append("fi\n");
				}

				// add text

				code.append(QString("export PS1=\"$PS1%1\"\n").arg(SpecialItem::typeToBashString(type)));

				lines.append(code);
			}
			lines.append("export PS1=\"$PS1\\[$(tput sgr0)\\]\"\n");
		}
		// this stops the escape squences from applying anymore
		//        lines.append("export PS1=\"$PS1$(tput sgr0)\"\n");

		for (auto item : forLater)
		{
			lines.append("#" + itemToXml(item));
		}

		rtn.program.append(lines);
	}
	else
	{
		if (ui->checkBox_RemoveAllNewlines->isChecked())
		{
			//ExecuteResult result = runCmd("echo -n \"$PS1\"", true, true); // -n for removing the newline and \"'s for keeping leading whitespace
			QString PS1 = bashInteractiveVariable("PS1");
			DEBUG_VAR(PS1);
			if (!PS1.isEmpty())
			{
				rtn.program.append(QString("PS1=\"%1\"").arg(PS1.remove("\\n")));
			}
			else
			{
				int ui = QMessageBox::warning(widget(), NAME + QString(" - Warning"), QString("No Prompt Configurations Found! Can Not Remove Newlines"), QMessageBox::Ok, QMessageBox::NoButton);
				Q_UNUSED(ui)
				goto end;
			}
		}
	}
end:
	DEBUG_EXIT(PromptTab::exec);
	return rtn;
}

CustomPromptProperty::CustomPromptProperty(QString name, QObject* parent)
	: QObject(parent),
	  m_name(name)
{
}

CustomPromptProperty::~CustomPromptProperty()
{
	if (m_widget != nullptr)
		delete m_widget;
}

ColorProperty::ColorProperty(QString name, QObject* parent)
	: CustomPromptProperty(name, parent)
{
	setWidget(new QWidget);
	widget()->setLayout(new QHBoxLayout(widget()));
	m_enabledColor = false;
	m_enabled = new QCheckBox("Enabled", widget());
	connect(m_enabled, &QCheckBox::clicked, [=]() {
		m_enabledColor = m_enabled->isChecked();
		m_colorBtn->setDisabled(!m_enabledColor);
	});
	m_colorBtn = new QPushButton(widget());
	connect(m_colorBtn, &QPushButton::clicked, this, &ColorProperty::onBtnPress);
	m_colorBtn->setFlat(true);
	m_colorBtn->setText("");
	m_colorBtn->setAutoFillBackground(true);
	m_color = QColor(0, 0, 0);
	updateBtn();
	widget()->layout()->addWidget(m_enabled);
	widget()->layout()->addWidget(m_colorBtn);
}

bool ColorProperty::good()
{
	return true;
}

void ColorProperty::updateBtn()
{
	QPalette pal = m_colorBtn->palette();
	pal.setColor(QPalette::Button, m_color);
	m_colorBtn->setPalette(pal);
	m_colorBtn->update();
	m_enabled->setChecked(m_enabledColor);
}

void ColorProperty::onBtnPress()
{
	m_color = QColorDialog::getColor(m_color);
	updateBtn();
}

CheckboxProperty::CheckboxProperty(QString name, QObject* parent)
	: CustomPromptProperty(name, parent)
{
	m_checkbox = new QCheckBox(name);
	setWidget(m_checkbox);
}

bool CheckboxProperty::good()
{
	return true;
}

TextProperty::TextProperty(QString name, QObject* parent)
	: CustomPromptProperty(name, parent)
{
	m_editor = new QLineEdit();
	setWidget(m_editor);
}

bool TextProperty::good()
{
	return true;
}

SimpleTextItem::SimpleTextItem(QString name)
	: CustomPromptItem(name)
{
	//    m_defaultForegroundColor = backgroundColor();
	//    m_defaultBackgroundColor = textColor();
	m_properties.push_back(new ColorProperty(QObject::tr("Foreground Color")));
	m_properties.push_back(new ColorProperty(QObject::tr("Background Color")));
	m_properties.push_back(new CheckboxProperty(QObject::tr("Bold")));
}

void SimpleTextItem::updateMembers()
{
	CustomPromptItem::updateMembers();
#define ELSE_ERROR else DEBUG << "Missing property"
	if (propertyByName("Foreground Color"))
	{
		m_foreground = static_cast<ColorProperty*>(propertyByName("Foreground Color"))->color();
		m_foregroundEnabled = static_cast<ColorProperty*>(propertyByName("Foreground Color"))->enabled();
	}
	ELSE_ERROR;
	if (propertyByName("Background Color"))
	{
		m_background = static_cast<ColorProperty*>(propertyByName("Background Color"))->color();
		m_backgroundEnabled = static_cast<ColorProperty*>(propertyByName("Background Color"))->enabled();
	}
	ELSE_ERROR;
	if (propertyByName("Bold"))
	{
		m_bold = static_cast<CheckboxProperty*>(propertyByName("Bold"))->checked();
	}
	ELSE_ERROR;
	//    if(m_backgroundEnabled)
	//        setBackgroundColor(m_background);
	//    else
	//        setBackgroundColor(m_defaultBackgroundColor);
	//    if(m_foregroundEnabled)
	//        setTextColor(m_foreground);
	//    else
	//        setTextColor(m_defaultForegroundColor);
}

void SimpleTextItem::updateProperties()
{
	CustomPromptItem::updateProperties();
	if (propertyByName("Foreground Color"))
	{
		static_cast<ColorProperty*>(propertyByName("Foreground Color"))->setColor(m_foreground);
		static_cast<ColorProperty*>(propertyByName("Foreground Color"))->setEnabled(m_foregroundEnabled);
	}
	ELSE_ERROR;
	if (propertyByName("Background Color"))
	{
		static_cast<ColorProperty*>(propertyByName("Background Color"))->setColor(m_background);
		static_cast<ColorProperty*>(propertyByName("Background Color"))->setEnabled(m_backgroundEnabled);
	}
	ELSE_ERROR;
	if (propertyByName("Bold"))
	{
		static_cast<CheckboxProperty*>(propertyByName("Bold"))->setChecked(m_bold);
	}
	ELSE_ERROR;
#undef ELSE_ERROR
	//    if(m_backgroundEnabled)
	//        setBackgroundColor(m_background);
	//    else
	//        setBackgroundColor(m_defaultBackgroundColor);
	//    if(m_foregroundEnabled)
	//        setTextColor(m_foreground);
	//    else
	//        setTextColor(m_defaultForegroundColor);
}

QListWidgetItem* CustomItemSelectorDialog::getItem(QWidget* parent)
{
	Q_UNUSED(parent)
	QDialog dialog;
	dialog.setModal(true);
	//    dialog.setParent(parent);
	dialog.setWindowTitle(NAME);
	dialog.setWindowIcon(QIcon::fromTheme("preferences-system"));
	dialog.setLayout(new QVBoxLayout);
	QListWidget* list = new QListWidget;
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	list->addItem("Text");
	list->addItem("Hostname (Long)");
	list->addItem("Hostname (Short)");
	list->addItem("Username");
	list->addItem("Current Directory (Long)");
	list->addItem("Current Directory (Short)");
	dialog.layout()->addWidget(list);
	dialog.layout()->addWidget(buttonBox);
	int result = dialog.exec();
	if (result == QDialog::Rejected)
		return nullptr;
	if (list->currentItem()->text() == "Text")
	{
		return new TextItem("", "Text");
	}
	else if (list->currentItem()->text() == "Hostname (Long)")
	{
		return new SpecialItem("", SpecialItem::Type::HostLong);
	}
	else if (list->currentItem()->text() == "Hostname (Short)")
	{
		return new SpecialItem("", SpecialItem::Type::HostShort);
	}
	else if (list->currentItem()->text() == "Username")
	{
		return new SpecialItem("", SpecialItem::Type::Username);
	}
	else if (list->currentItem()->text() == "Current Directory (Long)")
	{
		return new SpecialItem("", SpecialItem::Type::WorkingLong);
	}
	else if (list->currentItem()->text() == "Current Directory (Short)")
	{
		return new SpecialItem("", SpecialItem::Type::WorkingShort);
	}
	return nullptr;
}

CustomPromptItem::CustomPromptItem(QString name)
{
	setText(name);
}

void CustomPromptItemEditor::edit(CustomPromptItem* item, QWidget* parent)
{
	QDialog* dialog = new QDialog{parent};
	dialog->setWindowTitle(NAME);
	dialog->setWindowIcon(QIcon::fromTheme("preferences-system"));
	//    dialog->setParent(parent);
	dialog->setModal(true);

	dialog->setLayout(new QVBoxLayout);
	for (CustomPromptProperty* prop : item->properties())
	{
		QGroupBox* box = new QGroupBox(prop->name());
		box->setLayout(new QVBoxLayout);
		box->layout()->addWidget(prop->widget());
		dialog->layout()->addWidget(box);
	}
	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	QObject::connect(btnBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
	QObject::connect(btnBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
	dialog->layout()->addWidget(btnBox);
	int result = dialog->exec();
	if (result == QDialog::Accepted)
		item->updateMembers();
}

SpecialItem::SpecialItem(QString name, SpecialItem::Type type)
	: SimpleTextItem(name)
{
	m_type = type;
	refreshText();
}

void SpecialItem::updateMembers()
{
	SimpleTextItem::updateMembers();
}

void SpecialItem::updateProperties()
{
	SimpleTextItem::updateProperties();
}

void SpecialItem::refreshText()
{
	switch (m_type)
	{
	case Type::HostLong:
		setText("Hostname (Long)");
		break;
	case Type::HostShort:
		setText("Hostname (Short)");
		break;
	case Type::Username:
		setText("Username");
		break;
	case Type::WorkingLong:
		setText("Current Directory(Long)");
		break;
	case Type::WorkingShort:
		setText("Current Directory(Short)");
		break;
	}
}

CustomPromptProperty* CustomPromptItem::propertyByName(QString name) const
{
	for (CustomPromptProperty* prop : properties())
	{
		if (prop->name() == name)
		{
			return prop;
		}
	}
	return nullptr;
}

QString ColorProperty::stringify(QColor color) const
{
	int ansi256 = 1;
	// taken from: https://stackoverflow.com/questions/15682537/ansi-color-specific-rgb-sequence-bash
	bool finishedAnsi256 = false;
	if (color.red() == color.green() && color.green() == color.blue())
	{
		if (color.red() < 8)
		{
			if (!finishedAnsi256)
				ansi256 = 16;
			finishedAnsi256 = true;
		}
		if (color.red() > 248)
		{
			if (!finishedAnsi256)
				ansi256 = 231;
			finishedAnsi256 = true;
		}
		if (!finishedAnsi256)
			ansi256 = qRound(static_cast<float>((color.red() - 8) / 247) * 24) + 232;
	}
	if (!finishedAnsi256)
	{
		ansi256 = 16 +
			(36 * qRound(static_cast<float>(color.red() / 255 * 5))) +
			(6 * qRound(static_cast<float>(color.green() / 255 * 5))) +
			qRound(static_cast<float>(color.blue() / 255 * 5));
	}

	//    int ansi16 = (color.red()*6/256)*36 + (color.green()*6/256)*6 + (color.blue()*6/256);

	QString code;

	code.append("if [ $COLORTERM == \"truecolor\" ]; then\n");
	code.append(QString("\texport PS1=\"$PS1\\x1b[38;2;%1;%2;%3m\"\n").arg(color.red()).arg(color.green()).arg(color.blue()));
	code.append("else\n");
	code.append(QString("\texport PS1=\"$PS1\\x1b[38;5;%1m").arg(ansi256));
	code.append("fi\n");

	return QString();
}

QString SpecialItem::typeToBashString(Type t)
{
#define CASE(a, b) \
	case Type::a:  \
		return b;
	switch (t)
	{
		CASE(HostLong, "\\H")
		CASE(HostShort, "\\h")
		CASE(Username, "\\u")
		CASE(WorkingLong, "\\w")
		CASE(WorkingShort, "\\W")
	}
#undef CASE
	return QString{};
}

QString itemToXml(CustomPromptItem* item)
{
	QString output;
	Q_UNUSED(item)
	QXmlStreamWriter xmlStream{&output};
	xmlStream.writeStartDocument();

	xmlStream.writeStartElement("item");

	xmlStream.writeAttribute("type", (item->type() == CustomPromptItemType::Special) ? "special" : "text");
	if (item->type() == CustomPromptItemType::Special)
	{
		QString itemTypeString;
		switch (static_cast<SpecialItem*>(item)->itemType())
		{
#define CASE(x)                                    \
	case SpecialItem::Type::x:                     \
		itemTypeString = #x;                       \
		itemTypeString = itemTypeString.toLower(); \
		break;
			CASE(HostLong)
			CASE(HostShort)
			CASE(WorkingLong)
			CASE(WorkingShort)
			CASE(Username)
#undef CASE
		}
		xmlStream.writeTextElement("itemType", itemTypeString);

		xmlStream.writeStartElement("foreground");
		QColor foregroundColor = static_cast<SpecialItem*>(item)->propertyForeground();
		xmlStream.writeTextElement("enabled", (static_cast<SpecialItem*>(item)->propertyForegroundEnabled()) ? "true" : "false");
		xmlStream.writeTextElement("red", QString::number(foregroundColor.red()));
		xmlStream.writeTextElement("green", QString::number(foregroundColor.green()));
		xmlStream.writeTextElement("blue", QString::number(foregroundColor.blue()));
		xmlStream.writeEndElement(); // foreground

		xmlStream.writeStartElement("background");
		QColor backgroundColor = static_cast<SpecialItem*>(item)->propertyBackground();
		xmlStream.writeTextElement("enabled", (static_cast<SpecialItem*>(item)->propertyBackgroundEnabled()) ? "true" : "false");
		xmlStream.writeTextElement("red", QString::number(backgroundColor.red()));
		xmlStream.writeTextElement("green", QString::number(backgroundColor.green()));
		xmlStream.writeTextElement("blue", QString::number(backgroundColor.blue()));
		xmlStream.writeEndElement(); // background

		xmlStream.writeTextElement("bold", (static_cast<SpecialItem*>(item)->propertyBold()) ? "true" : "false");
	}
	else if (item->type() == CustomPromptItemType::Text)
	{
		xmlStream.writeTextElement("text", static_cast<TextItem*>(item)->propertyText());

		xmlStream.writeStartElement("foreground");
		QColor foregroundColor = static_cast<TextItem*>(item)->propertyForeground();
		xmlStream.writeTextElement("enabled", (static_cast<TextItem*>(item)->propertyForegroundEnabled()) ? "true" : "false");
		xmlStream.writeTextElement("red", QString::number(foregroundColor.red()));
		xmlStream.writeTextElement("green", QString::number(foregroundColor.green()));
		xmlStream.writeTextElement("blue", QString::number(foregroundColor.blue()));
		xmlStream.writeEndElement(); // foreground

		xmlStream.writeStartElement("background");
		QColor backgroundColor = static_cast<TextItem*>(item)->propertyBackground();
		xmlStream.writeTextElement("enabled", (static_cast<TextItem*>(item)->propertyBackgroundEnabled()) ? "true" : "false");
		xmlStream.writeTextElement("red", QString::number(backgroundColor.red()));
		xmlStream.writeTextElement("green", QString::number(backgroundColor.green()));
		xmlStream.writeTextElement("blue", QString::number(backgroundColor.blue()));
		xmlStream.writeEndElement(); // background

		xmlStream.writeTextElement("bold", (static_cast<TextItem*>(item)->propertyBold()) ? "true" : "false");
	}

	xmlStream.writeEndElement(); // item

	xmlStream.writeEndDocument();

	return output;
}

CustomPromptItem* xmlToItem(QString xml)
{
	QXmlStreamReader xmlStream{xml};
	while (xmlStream.readNextStartElement())
	{
		if (xmlStream.name() == "item")
		{
			QString type = xmlStream.attributes().value("type").toString();
			CustomPromptItem* obj = new CustomPromptItem{""};
			if (type == "special")
			{
				DEBUG << "Found type special";
				if (obj != nullptr)
					delete obj;
				obj = new SpecialItem("", SpecialItem::Type::Username);
				QColor foreground, background;
				bool foregroundEnabled = false, backgroundEnabled = false;
				while (xmlStream.readNextStartElement())
				{
					DEBUG << xmlStream.name();
					if (xmlStream.name() == "itemType")
					{
						QString rawStringItemType = xmlStream.readElementText();
						if (rawStringItemType == "workinglong")
						{
							static_cast<SpecialItem*>(obj)->setItemType(SpecialItem::Type::WorkingLong);
						}
						else if (rawStringItemType == "workingshort")
						{
							static_cast<SpecialItem*>(obj)->setItemType(SpecialItem::Type::WorkingShort);
						}
						else if (rawStringItemType == "hostlong")
						{
							static_cast<SpecialItem*>(obj)->setItemType(SpecialItem::Type::HostLong);
						}
						else if (rawStringItemType == "hostshort")
						{
							static_cast<SpecialItem*>(obj)->setItemType(SpecialItem::Type::HostShort);
						}
						else if (rawStringItemType == "username")
						{
							static_cast<SpecialItem*>(obj)->setItemType(SpecialItem::Type::Username);
						}
						else
						{
							DEBUG << "rawStringItemType didn't match: " << rawStringItemType;
						}
					}
					else if (xmlStream.name() == "foreground")
					{
						bool hitRed = false, hitGreen = false, hitBlue = false, hitEnabled = false;
						while (xmlStream.readNextStartElement())
						{
							if (xmlStream.name() == "red")
							{
								foreground.setRed(xmlStream.readElementText().toInt());
								hitRed = true;
							}
							else if (xmlStream.name() == "green")
							{
								foreground.setGreen(xmlStream.readElementText().toInt());
								hitGreen = true;
							}
							else if (xmlStream.name() == "blue")
							{
								foreground.setBlue(xmlStream.readElementText().toInt());
								hitBlue = true;
							}
							else if (xmlStream.name() == "enabled")
							{
								foregroundEnabled = xmlStream.readElementText() == "true";
								hitEnabled = true;
							}
							else
							{
								break;
							}
						}
						if (hitRed && hitGreen && hitBlue && hitEnabled)
							break;
					}
					else if (xmlStream.name() == "background")
					{
						bool hitRed = false, hitGreen = false, hitBlue = false, hitEnabled = false;
						while (xmlStream.readNextStartElement())
						{
							if (xmlStream.name() == "red")
							{
								background.setRed(xmlStream.readElementText().toInt());
								hitRed = true;
							}
							else if (xmlStream.name() == "green")
							{
								background.setGreen(xmlStream.readElementText().toInt());
								hitGreen = true;
							}
							else if (xmlStream.name() == "blue")
							{
								background.setBlue(xmlStream.readElementText().toInt());
								hitBlue = true;
							}
							else if (xmlStream.name() == "enabled")
							{
								backgroundEnabled = xmlStream.readElementText() == "true";
								hitEnabled = true;
							}
							else
							{
								break;
							}
						}
						if (hitRed && hitGreen && hitBlue && hitEnabled)
							break;
					}
					else if (xmlStream.name() == "bold")
					{
						QString raw = xmlStream.readElementText();
						DEBUG << "raw: " << raw;
						static_cast<SpecialItem*>(obj)->setPropertyBold(raw == "true");
					}
					else
					{
						DEBUG << "Nothing to be done for iteration: " << xmlStream.name();
					}
				}
				static_cast<SpecialItem*>(obj)->setPropertyForeground(foreground);
				static_cast<SpecialItem*>(obj)->setPropertyBackground(background);
				static_cast<SpecialItem*>(obj)->setPropertyForegroundEnabled(foregroundEnabled);
				static_cast<SpecialItem*>(obj)->setPropertyBackgroundEnabled(backgroundEnabled);
			}
			else if (type == "text")
			{
				DEBUG << "Found type text";
				if (obj != nullptr)
					delete obj;
				obj = new TextItem("", "<placeholder>");
				QColor foreground, background;
				bool foregroundEnabled = false, backgroundEnabled = false;
				while (xmlStream.readNextStartElement())
				{
					QString name = xmlStream.name().toString();
					if (name == "text")
					{
						static_cast<TextItem*>(obj)->setPropertyText(xmlStream.readElementText());
					}
					else if (name == "foreground")
					{
						bool hitRed = false, hitGreen = false, hitBlue = false, hitEnabled = false;
						while (xmlStream.readNextStartElement())
						{
							name = xmlStream.name().toString();
							if (name == "red")
							{
								foreground.setRed(xmlStream.readElementText().toInt());
								hitRed = true;
							}
							else if (name == "green")
							{
								foreground.setGreen(xmlStream.readElementText().toInt());
								hitGreen = true;
							}
							else if (name == "blue")
							{
								foreground.setBlue(xmlStream.readElementText().toInt());
								hitBlue = true;
							}
							else if (name == "enabled")
							{
								foregroundEnabled = xmlStream.readElementText() == "true";
								hitEnabled = true;
							}
							else if (hitRed && hitGreen && hitBlue && hitEnabled)
								break;
							else
							{
								break;
							}
						}
					}
					else if (name == "background")
					{
						bool hitRed = false, hitGreen = false, hitBlue = false, hitEnabled = false;
						while (xmlStream.readNextStartElement())
						{
							name = xmlStream.name().toString();
							if (name == "red")
							{
								background.setRed(xmlStream.readElementText().toInt());
								hitRed = true;
							}
							else if (name == "green")
							{
								background.setGreen(xmlStream.readElementText().toInt());
								hitGreen = true;
							}
							else if (name == "blue")
							{
								background.setBlue(xmlStream.readElementText().toInt());
								hitBlue = true;
							}
							else if (name == "enabled")
							{
								backgroundEnabled = xmlStream.readElementText() == "true";
								hitEnabled = true;
							}
							else if (hitRed && hitGreen && hitBlue && hitEnabled)
								break;
							else
							{
								break;
							}
						}
					}
					else if (name == "bold")
					{
						QString raw = xmlStream.readElementText();
						DEBUG << "raw: " << raw;
						static_cast<TextItem*>(obj)->setPropertyBold(raw == "true");
					}
					else
					{
						DEBUG << "Nothing to be done for iteration: " << xmlStream.name();
					}
				}

				static_cast<TextItem*>(obj)->setPropertyForeground(foreground);
				static_cast<TextItem*>(obj)->setPropertyBackground(background);
				static_cast<TextItem*>(obj)->setPropertyForegroundEnabled(foregroundEnabled);
				static_cast<TextItem*>(obj)->setPropertyBackgroundEnabled(backgroundEnabled);
			}
			return obj;
		}
		else
			xmlStream.skipCurrentElement();
	}
	if (xmlStream.hasError())
	{
		QString errorString;
#define CASE(x)               \
	case QXmlStreamReader::x: \
		errorString = #x;     \
		break;
		switch (xmlStream.error())
		{
			CASE(NoError)
			CASE(CustomError)
			CASE(NotWellFormedError)
			CASE(PrematureEndOfDocumentError)
			CASE(UnexpectedElementError)
		}
#undef CASE
		DEBUG << "xmlStream.hasError() returned true" << errorString;
	}
	return nullptr;
}

TextItem::TextItem(QString name, QString text)
	: SimpleTextItem(name)
{
	m_text = text;
	updateProperties();
	m_properties.push_back(new TextProperty{"Text"});
}

void TextItem::updateMembers()
{
	SimpleTextItem::updateMembers();
	if (propertyByName("Text"))
		m_text = static_cast<TextProperty*>(propertyByName("Text"))->text();
	else
		DEBUG << "Text property doesn't exist";
	setText(m_text);
}

void TextItem::updateProperties()
{
	SimpleTextItem::updateProperties();
	if (propertyByName("Text"))
		static_cast<TextProperty*>(propertyByName("Text"))->setText(m_text);
	else
		DEBUG << "Text property doesn't exist";
	setText(m_text);
}

QString terminalEncode(QString input)
{
	input.replace(QRegularExpression("^[^\\]+$"), "\\$");
	input.replace("\\", "\\\\");
	return input;
}

QString terminalDecode(QString input)
{
	input.replace(QRegularExpression("\\$"), "$");
	input.replace("\\\\", "\\");
	return input;
}
