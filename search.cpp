#include "search.h"

#include <QMessageBox>
#include <QTextCursor>

Search::Search(QWidget *parent, QString title)
    : QDockWidget(title, parent)
{
    setup_actions();
    setup_ui();
    setWidget(m_widget);
    setAllowedAreas(Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea);
    hide();
}

// Utilities
bool Search::do_search(QString query, QTextDocument::FindFlags flags)
{
    auto *active_editor = get_active_editor();
    QTextCursor cursor = active_editor->textCursor();
    auto cursor_original = cursor;
    if (!active_editor->find(query, flags))
    {
        cursor.movePosition((flags & QTextDocument::FindBackward) == QTextDocument::FindBackward
                            ? QTextCursor::End
                            : QTextCursor::Start);
        active_editor->setTextCursor(cursor);

        if (!active_editor->find(query, flags))
        {
            // TODO: show a message in the status bar
            qDebug() << "query not found";
            active_editor->setTextCursor(cursor_original);
        }
        return false;
    }
    return true;
}

QTextDocument::FindFlags Search::get_find_flags(bool backward)
{
    QTextDocument::FindFlags flags;
    if (m_case_sensitive) flags |= QTextDocument::FindCaseSensitively;
    if (m_find_whole_words) flags |= QTextDocument::FindWholeWords;
    if (backward) flags |= QTextDocument::FindBackward;
    return flags;
}

// Slots

void Search::search()
{
    QTextDocument::FindFlags flags = get_find_flags();
    do_search(m_search_line->text(), flags);
}

void Search::search_previous()
{
    QTextDocument::FindFlags flags = get_find_flags(true);
    do_search(m_search_line->text(), flags);
}

void Search::replace()
{
    QTextCursor cursor = get_active_editor()->textCursor();
    if (cursor.hasSelection())
        cursor.insertText(m_replace_line->text());
}

void Search::replace_all()
{
    int res = QMessageBox::warning(this, "warning", "are you sure you want to replace all instances?", QMessageBox::Ok | QMessageBox::Cancel);
    if (res == QMessageBox::Ok)
    {
        QTextDocument::FindFlags flags = get_find_flags();
        do_search(m_search_line->text(), flags);
        do {
            replace();
        } while (do_search(m_search_line->text(), flags));
    }
}

void Search::toggle_case_sensitive()
{
    m_case_sensitive = m_checkbox_case_sensitive->isChecked();
    qDebug() << m_case_sensitive << m_checkbox_case_sensitive->isChecked();
}

void Search::toggle_find_whole_words()
{
    m_find_whole_words = m_checkbox_find_whole_words->isChecked();
}

// Setup
void Search::setup_actions()
{
    action_search = new QAction(tr("search"), this);
    action_search->setIcon(QIcon::fromTheme("system-search"));
    action_search->setStatusTip(tr("find next instance"));
    connect(action_search, &QAction::triggered, this, &Search::search);

    action_search_previous = new QAction(tr("search previous"), this);
    action_search_previous->setStatusTip("find previous instance");
    connect(action_search_previous, &QAction::triggered, this, &Search::search_previous);

    action_replace = new QAction(tr("replace"), this);
    action_replace->setIcon(QIcon::fromTheme("edit-find"));
    action_replace->setStatusTip(tr("replace current instance"));
    connect(action_replace, &QAction::triggered, this, &Search::replace);

    action_replace_all = new QAction(tr("replace all"), this);
    action_replace_all->setIcon(QIcon::fromTheme("edit-find-replace"));
    action_replace_all->setStatusTip(tr("replace all instances"));
    connect(action_replace_all, &QAction::triggered, this, &Search::replace_all);

    action_toggle_case_sensitive = new QAction(tr("case-sensitive"));
    action_toggle_case_sensitive->setStatusTip(tr("search using case-sensitive query"));
    //connect(action_toggle_case_sensitive, &QAction::triggered, this, &AdvancedSearch::toggle_case_sensitive);

    action_toggle_find_whole_words = new QAction(tr("find whole words"));
    action_toggle_find_whole_words->setStatusTip(tr("search whole word matches only"));
    //connect(action_toggle_find_whole_words, &QAction::triggered, this, &AdvancedSearch::toggle_find_whole_words);
}

void Search::setup_ui()
{
    m_widget = new QWidget;

    m_vertical_layout = new QVBoxLayout;
    m_search_layout = new QHBoxLayout;
    m_replace_layout = new QHBoxLayout;
    m_options_layout = new QHBoxLayout;

    // Search Line Widgets
    m_search_line = new QLineEdit;
    m_search_line->setPlaceholderText(tr("search..."));
    connect(m_search_line, &QLineEdit::returnPressed, this, &Search::search);

    m_search_button = new QPushButton;
    m_search_button->setText(tr("search"));
    m_search_button->addAction(action_search);
    connect(m_search_button, &QPushButton::pressed, this, &Search::search);

    m_search_previous_button = new QPushButton;
    m_search_previous_button->setText(tr("previous"));
    m_search_previous_button->addAction(action_search_previous);
    connect(m_search_previous_button, &QPushButton::pressed, this, &Search::search_previous);

    m_search_layout->addWidget(m_search_line);
    m_search_layout->addWidget(m_search_button);
    m_search_layout->addWidget(m_search_previous_button);
    m_vertical_layout->addStretch(1);

    // Replace Line Widgets
    m_replace_line = new QLineEdit;
    m_replace_line->setPlaceholderText(tr("replace with..."));

    m_replace_button = new QPushButton;
    m_replace_button->setText(tr("replace"));
    m_replace_button->addAction(action_replace);
    connect(m_replace_button, &QPushButton::pressed, this, &Search::replace);

    m_replace_all_button = new QPushButton;
    m_replace_all_button->setText(tr("replace all"));
    m_replace_all_button->addAction(action_replace_all);
    connect(m_replace_all_button, &QPushButton::pressed, this, &Search::replace_all);

    m_replace_layout->addWidget(m_replace_line);
    m_replace_layout->addWidget(m_replace_button);
    m_replace_layout->addWidget(m_replace_all_button);

    // Options Line Widgets
    m_checkbox_case_sensitive = new QCheckBox;
    m_checkbox_case_sensitive->setText(tr("case sensitive"));
    m_checkbox_case_sensitive->addAction(action_toggle_case_sensitive);
    connect(m_checkbox_case_sensitive, &QCheckBox::toggled, this, &Search::toggle_case_sensitive);

    m_checkbox_find_whole_words = new QCheckBox;
    m_checkbox_find_whole_words->setText(tr("find whole words"));
    m_checkbox_case_sensitive->addAction(action_toggle_find_whole_words);
    connect(m_checkbox_find_whole_words, &QCheckBox::toggled, this, &Search::toggle_find_whole_words);

    m_options_layout->addWidget(m_checkbox_case_sensitive);
    m_options_layout->addWidget(m_checkbox_find_whole_words);

    // Add them all to the vert layout
    m_vertical_layout->addLayout(m_search_layout);
    m_vertical_layout->addLayout(m_replace_layout);
    m_vertical_layout->addLayout(m_options_layout);
    //m_vertical_layout->setSizeConstraint(QLayout::SetFixedSize);
    m_vertical_layout->addStretch(1);

    m_widget->setLayout(m_vertical_layout);
}
