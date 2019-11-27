#ifndef SEARCH_H
#define SEARCH_H

#include "luaedit.h"

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QTextDocument>
#include <QVBoxLayout>

class Search : public QDockWidget
{
    Q_OBJECT
public:
    Search(QWidget *parent = nullptr, QString title = "search and replace");
    void quick(QString query, QTextDocument::FindFlags flags = nullptr) { Search::do_search(query, flags); }

private:
    //Utilities
    LuaEdit *get_active_editor() { return parent()->findChild<QMdiArea*>()->activeSubWindow()->findChild<LuaEdit*>(); }
    bool do_search(QString query, QTextDocument::FindFlags flags = nullptr);
    QTextDocument::FindFlags get_find_flags(bool backward = false);


private:
    // UI
    void setup_ui();

    QWidget *m_widget;

    QVBoxLayout *m_vertical_layout;
    QHBoxLayout *m_search_layout;
    QHBoxLayout *m_replace_layout;
    QHBoxLayout *m_options_layout;

    QLineEdit *m_search_line;
    QLineEdit *m_replace_line;

    QPushButton *m_search_button;
    QPushButton *m_search_previous_button;
    QPushButton *m_replace_button;
    QPushButton *m_replace_all_button;

    QCheckBox *m_checkbox_case_sensitive;
    QCheckBox *m_checkbox_find_whole_words;

    // State
    bool m_case_sensitive = false;
    bool m_find_whole_words = false;

private:
    // actions
    void setup_actions();

    QAction *action_search;
    QAction *action_search_previous;
    QAction *action_replace;
    QAction *action_replace_all;
    QAction *action_toggle_case_sensitive;
    QAction *action_toggle_find_whole_words;
    QAction *action_toggle_global_search;
protected slots:
    void search();
    void search_previous();
    void replace();
    void replace_all();
    void toggle_case_sensitive();
    void toggle_find_whole_words();
};

#endif // SEARCH_H
