#include "gratuitous.h"
#include "search.h"

#include <QFileDialog>
#include <QList>
#include <QMdiArea>
#include <QMessageBox>
#include <QStringList>
#include <QtWidgets>
#include <QWidget>
#include <QWindow>

Gratuitous::Gratuitous(QWidget *parent)
    : QMainWindow(parent)
{
    m_mdi_area = new QMdiArea;
    m_search = new Search(this);
    m_prefs_dialog = new Prefs(this);
    m_preview = new Preview(this);

    // connect to preview ready with a lambda in case we need it
    // this had been used for attempting to re-parent Xephyr
    connect(m_preview, &Preview::ready,
            [this]() {
                qDebug() << "Preview ready";
            }
    );

    setCentralWidget(m_mdi_area);
    m_statusbar = new QStatusBar(this);
    setStatusBar(m_statusbar);

    setup_menu_actions();
    add_menus();
    setup_toolbar();

    connect(m_prefs_dialog, &Prefs::settings_applied, this, &Gratuitous::handle_settings_change);

    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_search);
    setWindowTitle("gratuitous");
}

// Utilities

void Gratuitous::do_save(LuaEdit *editor)
{
    LuaEdit *selected_editor = get_active_editor();
    if (editor == nullptr && selected_editor == nullptr)
        return;
    if (editor != nullptr)
        selected_editor = editor;

    if (selected_editor->filename().isEmpty())
    {
        QString filename = QFileDialog::getSaveFileName(this, tr("save as..."), m_prefs.value("path").value<QString>());
        if (filename.isEmpty()) return;
        selected_editor->set_filename(filename);
    }

    QFile file(selected_editor->filename());
    if (!file.open(QIODevice::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Warning", "Error saving file " + selected_editor->filename() + ": " + file.errorString());
        return;
    }

    QTextStream out(&file);
    QString text = selected_editor->toPlainText();
    out << text;

    selected_editor->parentWidget()->setWindowTitle(selected_editor->filename() + "[*]");
    selected_editor->document()->setModified(false);
    file.close();

    if (m_prefs.value("preview/autoreload").value<bool>() && m_preview->started())
        m_preview->reload();
}

/*
 * Slots
 */

// File
void Gratuitous::open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose file", m_prefs.value("path").value<QString>(), "*.lua");
    if (!filename.isEmpty())
        create_new_editor(filename);
}

void Gratuitous::save_as()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save as", m_prefs.value("path").value<QString>());
    if (filename.isEmpty()) return;
    LuaEdit *active_editor = get_active_editor();
    active_editor->set_filename(filename);
    do_save(active_editor);
    active_editor->document()->setModified(false);
}

void Gratuitous::save_all()
{
    QList<QMdiSubWindow*> sub_windows = m_mdi_area->subWindowList();
    qDebug() << sub_windows;
    for (auto &window : sub_windows)
    {
        auto *editor = window->findChild<LuaEdit*>();
        qDebug() << editor;
        do_save(editor);
        editor->document()->setModified(false);
    }
}

void Gratuitous::revert()
{
    LuaEdit *active_editor = get_active_editor();
    QString filename = active_editor->filename();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "warning", "revert: could not open file: " + file.errorString());
        return;
    }
    QTextStream in(&file);
    QString text = in.readAll();
    active_editor->setPlainText(text);
}

void Gratuitous::exit()
{
    QList<QMdiSubWindow*> editor_windows = m_mdi_area->subWindowList();
    for (QMdiSubWindow* win : editor_windows)
    {
        if (win->findChild<LuaEdit*>())
        {
            win->setFocus();
            close_window();
        }
        else
        {
            // This supports future window types
            win->setFocus();
            m_mdi_area->closeActiveSubWindow();
        }
    }
    editor_windows = m_mdi_area->subWindowList();
    if (editor_windows.isEmpty())
    {
        QApplication::exit();
        return;
    }
    // just in case something odd happened, check again that there are no windows left
    exit();
}

// Edit
void Gratuitous::toggle_search_dock_visible()
{
    if (m_search->isVisible())
        m_search->hide();
    else
        m_search->show();
}

void Gratuitous::show_prefs_dialog()
{
    m_prefs_dialog->show();
}

// View
void Gratuitous::set_preview_menu_entries()
{
    action_start_preview->setVisible(m_preview->started() ? false : true);
    action_stop_preview->setVisible(m_preview->started() ? true : false);
}

void Gratuitous::toggle_preview()
{
    if (!m_preview->started())
    {
        LuaEdit *active_editor = get_active_editor();
        if (active_editor == nullptr) return;

        m_preview->start(active_editor->filename());
        set_preview_menu_entries();
    }
    else
    {
        m_preview->stop();
        set_preview_menu_entries();
    }
    action_reload_preview->setEnabled(!m_preview->started());
}

void Gratuitous::toggle_auto_reload()
{
    m_prefs.setValue("preview/autoreload", action_toggle_auto_reload->isChecked());
}

void Gratuitous::reload_preview()
{
    m_preview->reload();
}

// Window
void Gratuitous::set_window_actions_enabled(bool state)
{
    action_close_window->setEnabled(state);
    action_close_all_windows->setEnabled(state);
    action_next_window->setEnabled(state);
    action_prev_window->setEnabled(state);
}

void Gratuitous::close_window()
{
    LuaEdit *active_editor = get_active_editor();
    if (active_editor == nullptr) return;

    if (active_editor->document()->isModified())
    {
        QMessageBox msg(this);
        msg.setText(active_editor->filename() + " has been modified.");
        msg.setInformativeText("Do you want to save the changes?");
        msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Save);
        int response = msg.exec();
        switch (response)
        {
        case QMessageBox::Save:
            save();
            break;
        case QMessageBox::Cancel:
            return;
        }
    }
    active_editor->action()->setVisible(false);
    //menu_window->removeAction(active_editor->action());
    m_mdi_area->closeActiveSubWindow();
    delete active_editor;

    if (m_mdi_area->subWindowList().length() == 0)
        set_window_actions_enabled(false);
}

void Gratuitous::close_all_windows()
{
    m_mdi_area->closeAllSubWindows();
    set_window_actions_enabled(false);
}
// Help
void Gratuitous::show_about()
{
    QString text = "gratuitous " + VERSION + "\nbrought to you by jaylectric\nhttps://github.com/jaylectric/gratuitous";
    QMessageBox::about(this, "about gratuitous", text);
}

// Other
void Gratuitous::create_new_editor(QString filename)
{
    auto *new_window = new QMdiSubWindow;
    auto *new_editor = new LuaEdit(new_window, std::move(filename));

    new_window->setAttribute(Qt::WA_DeleteOnClose);
    new_window->setWidget(new_editor);
    new_window->setWindowTitle((new_editor->filename().isEmpty() ? "untitled" + QString::number(editor_count) : new_editor->filename()) + "[*]");
    m_mdi_area->addSubWindow(new_window)->showMaximized();

    // connect the new subwindow to modified slot
    connect(new_editor->document(), &QTextDocument::modificationChanged, this, &Gratuitous::editor_was_modified);

    QString menu_entry_text = new_editor->filename().isEmpty() ? "untitled" + QString::number(editor_count) : new_editor->filename();
    auto *action_focus_window = new QAction(menu_entry_text, this);
    action_focus_window->setData(menu_entry_text);
    menu_window->addAction(action_focus_window);

    auto focus_window = [&, action_focus_window] {
        QString filename = action_focus_window->text();
        QList<QMdiSubWindow*> windows = m_mdi_area->subWindowList();
        for (auto& window : windows)
            if (match_editor(window, filename))
                window->setFocus();
    };

    set_window_actions_enabled(true);
    connect(action_focus_window, &QAction::triggered, this, focus_window);
    new_editor->set_action(action_focus_window);
    m_mdi_area->setActiveSubWindow(new_window);

    // get editor settings from m_prefs
    auto *font = new QFont;
    font->setFamily(m_prefs.value("font/family").value<QString>());
    font->setPointSize(m_prefs.value("font/size").value<int>());
    font->setFixedPitch(m_prefs.value("font/fixed").value<bool>());
    new_editor->setFont(*font);

    bool wordwrap = m_prefs.value("editor/wordwrap").value<bool>();
    new_editor->setLineWrapMode(wordwrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
}

void Gratuitous::editor_was_modified(bool changed)
{
    auto *active_editor = m_mdi_area->activeSubWindow()->findChild<LuaEdit*>();
    active_editor->document()->setModified(changed);
    active_editor->parentWidget()->setWindowModified(changed);
}

void Gratuitous::quick_search_next()
{
    const QString query = m_toolbar->findChild<QLineEdit*>()->text();
    m_search->quick(query);
}

void Gratuitous::quick_search_previous()
{
    const QString query = m_toolbar->findChild<QLineEdit*>()->text();
    m_search->quick(query, QTextDocument::FindBackward);
}

void Gratuitous::handle_settings_change()
{
    QStringList keys = m_prefs.allKeys();
    QFont font;
    bool wordwrap = false;
    for (auto& key : keys)
    {
        if (key.contains("font"))
        {
            if (key.contains("family"))
                font.setFamily(m_prefs.value(key).value<QString>());
            if (key.contains("size"))
                font.setPointSize(m_prefs.value(key).value<int>());
            if (key.contains("fixed"))
                font.setFixedPitch(m_prefs.value(key).value<bool>());
        }
        if (key.contains("editor"))
        {
            if (key.contains("wordwrap"))
                wordwrap = m_prefs.value(key).value<bool>();
        }
    }

    change_all_editors(font, wordwrap);
}

void Gratuitous::change_all_editors(QFont font, bool wordwrap)
{
    QList<QMdiSubWindow*> windows = m_mdi_area->subWindowList();
    for (auto& window : windows)
    {
        auto *current_editor = get_editor(window);
        if (current_editor != nullptr)
        {
            current_editor->setFont(font);
            current_editor->setLineWrapMode(wordwrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
        }
    }
}

//
// UI Setup
void Gratuitous::setup_menu_actions()
{
    // File
    action_new = new QAction(tr("new"), this);
    action_new->setIcon(QIcon::fromTheme("document-new"));
    action_new->setShortcut(QKeySequence::New);
    action_new->setStatusTip(tr("create a new file"));
    connect(action_new, &QAction::triggered, this, &Gratuitous::new_file);

    action_open = new QAction(tr("open"), this);
    action_open->setIcon(QIcon::fromTheme("document-open"));
    action_open->setShortcut(QKeySequence::Open);
    action_open->setStatusTip(tr("open a file"));
    connect(action_open, &QAction::triggered, this, &Gratuitous::open);

    action_save = new QAction(tr("save"), this);
    action_save->setIcon(QIcon::fromTheme("document-save"));
    action_save->setShortcut(QKeySequence::Save);
    action_save->setStatusTip(tr("save current file"));
    connect(action_save, &QAction::triggered, this, &Gratuitous::save);

    action_save_as = new QAction(tr("save as"), this);
    action_save_as->setIcon(QIcon::fromTheme("document-save-as"));
    action_save_as->setShortcut(QKeySequence::SaveAs);
    action_save_as->setStatusTip(tr("save current file as..."));
    connect(action_save_as, &QAction::triggered, this, &Gratuitous::save_as);

    action_save_all = new QAction(tr("save all..."), this);
    action_save_all->setIcon(QIcon::fromTheme("document-save-all"));
    action_save_all->setStatusTip(tr("save all open files"));
    connect(action_save_all, &QAction::triggered, this, &Gratuitous::save_all);

    action_revert = new QAction(tr("revert file"), this);
    action_revert->setIcon(QIcon::fromTheme("document-revert"));
    action_revert->setStatusTip(tr("revert selected file to original state."));
    connect(action_revert, &QAction::triggered, this, &Gratuitous::revert);

    action_exit = new QAction(tr("exit"), this);
    action_exit->setIcon(QIcon::fromTheme("application-exit"));
    action_exit->setStatusTip(tr("close all documents and exit gratuitous"));
    connect(action_exit, &QAction::triggered, this, &Gratuitous::exit);

    // Edit
    action_undo = new QAction(tr("undo"));
    action_undo->setShortcut(QKeySequence::Undo);
    action_undo->setIcon(QIcon::fromTheme("edit-undo"));
    action_undo->setStatusTip(tr("undo last change in active editor"));
    connect(action_undo, &QAction::triggered, this, &Gratuitous::undo);

    action_redo = new QAction(tr("redo"));
    action_redo->setShortcut(QKeySequence::Redo);
    action_redo->setIcon(QIcon::fromTheme("edit-redo"));
    action_redo->setStatusTip(tr("redo last change in active editor"));
    connect(action_redo, &QAction::triggered, this, &Gratuitous::redo);

    action_cut = new QAction(tr("cut"));
    action_cut->setShortcut(QKeySequence::Cut);
    action_cut->setIcon(QIcon::fromTheme("edit-cut"));
    action_cut->setStatusTip(tr("cut selected text to clipboard"));
    connect(action_cut, &QAction::triggered, this, &Gratuitous::cut);

    action_copy = new QAction(tr("copy"));
    action_copy->setShortcut(QKeySequence::Copy);
    action_copy->setIcon(QIcon::fromTheme("edit-copy"));
    action_copy->setStatusTip(tr("copy selected text to clipboard"));
    connect(action_cut, &QAction::triggered, this, &Gratuitous::copy);

    action_paste = new QAction(tr("paste"));
    action_paste->setShortcut(QKeySequence::Paste);
    action_paste->setIcon(QIcon::fromTheme("edit-paste"));
    action_paste->setStatusTip(tr("paste text from clipboard at cursor position"));
    connect(action_paste, &QAction::triggered, this, &Gratuitous::paste);

    action_focus_quick_find = new QAction(tr("quick find..."));
    action_focus_quick_find->setShortcut(QKeySequence::Find);
    action_focus_quick_find->setIcon(QIcon::fromTheme("edit-find"));
    action_focus_quick_find->setStatusTip(tr("search in the current document"));
    connect(action_focus_quick_find, &QAction::triggered, this, &Gratuitous::focus_quick_find);

    action_show_find_dock = new QAction(tr("search and replace..."), this);
    action_show_find_dock->setIcon(QIcon::fromTheme("system-search"));
    action_show_find_dock->setStatusTip(tr("open the search and replace dock"));
    connect(action_show_find_dock, &QAction::triggered, this, &Gratuitous::toggle_search_dock_visible);

    action_show_prefs_dialog = new QAction(tr("preferences..."), this);
    action_show_prefs_dialog->setIcon(QIcon::fromTheme("preferences-desktop"));
    action_show_prefs_dialog->setShortcut(QKeySequence::Preferences);
    action_show_prefs_dialog->setStatusTip(tr("open the preferences dialog"));
    connect(action_show_prefs_dialog, &QAction::triggered, this, &Gratuitous::show_prefs_dialog);

    // View
    action_start_preview = new QAction(tr("start preview..."), this);
    action_start_preview->setIcon(QIcon::fromTheme("media-playback-start"));
    action_start_preview->setShortcut(QKeySequence::FullScreen);
    action_start_preview->setStatusTip(tr("open the preview window"));
    connect(action_start_preview, &QAction::triggered, this, &Gratuitous::toggle_preview);

    action_stop_preview = new QAction(tr("stop preview..."), this);
    action_stop_preview->setIcon(QIcon::fromTheme("media-playback-stop"));
    action_stop_preview->setShortcut(QKeySequence::FullScreen);
    action_stop_preview->setStatusTip(tr("close the preview window"));
    action_stop_preview->setVisible(false);
    connect(action_stop_preview, &QAction::triggered, this, &Gratuitous::toggle_preview);

    action_toggle_auto_reload = new QAction(tr("auto-reload"), this);
    action_toggle_auto_reload->setIcon(QIcon::fromTheme("object-select"));
    action_toggle_auto_reload->setStatusTip(tr("toggle auto-reload on save"));
    action_toggle_auto_reload->setCheckable(true);
    connect(action_toggle_auto_reload, &QAction::toggled, this, &Gratuitous::toggle_auto_reload);

    action_reload_preview = new QAction(tr("reload preview"), this);
    action_reload_preview->setIcon(QIcon::fromTheme("view-refresh"));
    action_start_preview->setShortcut(QKeySequence::Refresh);
    action_reload_preview->setStatusTip(tr("send a reload signal to awesome"));
    action_reload_preview->setEnabled(false);
    connect(action_reload_preview, &QAction::triggered, this, &Gratuitous::reload_preview);

    // Window
    action_tile_windows = new QAction(tr("tile"), this);
    action_tile_windows->setStatusTip(tr("tile all currently open windows"));
    connect(action_tile_windows, &QAction::triggered, this, &Gratuitous::tile_windows);

    action_cascade_windows = new QAction(tr("cascade"), this);
    action_cascade_windows->setStatusTip(tr("cascade all currently open windows"));
    connect(action_cascade_windows, &QAction::triggered, this, &Gratuitous::cascade_windows);

    action_next_window = new QAction(tr("next window"), this);
    action_next_window->setIcon(QIcon::fromTheme("go-next"));
    action_next_window->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab));
    action_next_window->setStatusTip(tr("focus next window"));
    action_next_window->setEnabled(false);
    connect(action_next_window, &QAction::triggered, this, &Gratuitous::next_window);

    action_prev_window = new QAction(tr("previous window"), this);
    action_prev_window->setIcon(QIcon::fromTheme("go-previous"));
    action_prev_window->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));
    action_prev_window->setStatusTip(tr("focus previous window"));
    action_prev_window->setEnabled(false);
    connect(action_prev_window, &QAction::triggered, this, &Gratuitous::prev_window);

    action_close_window = new QAction(tr("close"), this);
    action_close_window->setIcon(QIcon::fromTheme("window-close"));
    action_close_window->setStatusTip(tr("close window"));
    action_close_window->setShortcut(QKeySequence::Close);
    action_close_window->setEnabled(false);
    connect(action_close_window, &QAction::triggered, this, &Gratuitous::close_window);

    action_close_all_windows = new QAction(tr("close all"), this);
    action_close_all_windows->setStatusTip(tr("close all open windows"));
    action_close_all_windows->setEnabled(false);
    connect(action_close_all_windows, &QAction::triggered, this, &Gratuitous::close_all_windows);

    // Help
    action_show_awesome_docs = new QAction(tr("awesome docs..."), this);
    action_show_awesome_docs->setStatusTip(tr("go to the official awesome docs in a new browser window"));
    connect(action_show_awesome_docs, &QAction::triggered, this, &Gratuitous::show_awesome_docs);

    action_show_lua_reference = new QAction(tr("lua reference..."));
    action_show_lua_reference->setStatusTip(tr("open a brief lua reference"));
    connect(action_show_lua_reference, &QAction::triggered, this, &Gratuitous::show_lua_reference);

    action_show_about = new QAction(tr("about gratuitous..."));
    connect(action_show_about, &QAction::triggered, this, &Gratuitous::show_about);
}

void Gratuitous::add_menus()
{
    menu_file = menuBar()->addMenu(tr("&file"));
    menu_file->addAction(action_new);
    menu_file->addAction(action_open);
    menu_file->addAction(action_save);
    menu_file->addAction(action_save_as);
    menu_file->addAction(action_save_all);
    menu_file->addAction(action_revert);
    menu_file->addSeparator();
    menu_file->addAction(action_exit);

    menu_edit = menuBar()->addMenu(tr("&edit"));
    menu_edit->addAction(action_undo);
    menu_edit->addAction(action_redo);
    menu_edit->addSeparator();
    menu_edit->addAction(action_cut);
    menu_edit->addAction(action_copy);
    menu_edit->addAction(action_paste);
    menu_edit->addSeparator();
    menu_edit->addAction(action_show_find_dock);
    menu_edit->addAction(action_focus_quick_find);
    menu_edit->addAction(action_show_prefs_dialog);

    menu_view = menuBar()->addMenu(tr("&view"));
    menu_view->addAction(action_start_preview);
    menu_view->addAction(action_stop_preview);
    menu_view->addAction(action_toggle_auto_reload);
    menu_view->addAction(action_reload_preview);

    menu_window = menuBar()->addMenu(tr("&window"));
    menu_window->addAction(action_tile_windows);
    menu_window->addAction(action_cascade_windows);
    menu_window->addSeparator();
    menu_window->addAction(action_next_window);
    menu_window->addAction(action_prev_window);
    menu_window->addSeparator();
    menu_window->addAction(action_close_window);
    menu_window->addAction(action_close_all_windows);
    menu_window->addSeparator();

    menu_help = menuBar()->addMenu(tr("&help"));
    menu_help->addAction(action_show_awesome_docs);
    menu_help->addAction(action_show_lua_reference);
    menu_help->addSeparator();
    menu_help->addAction(action_show_about);
}

void Gratuitous::setup_toolbar()
{
    m_toolbar = new QToolBar;
    // file
    m_toolbar->addAction(action_new);
    m_toolbar->addAction(action_open);
    m_toolbar->addAction(action_save);
    m_toolbar->addSeparator();
    // edit
    m_toolbar->addAction(action_show_find_dock);
    m_toolbar->addSeparator();
    // view
    m_toolbar->addAction(action_start_preview);
    m_toolbar->addAction(action_stop_preview);
    m_toolbar->addAction(action_toggle_auto_reload);
    m_toolbar->addAction(action_reload_preview);
    m_toolbar->addSeparator();
    // window
    m_toolbar->addAction(action_prev_window);
    m_toolbar->addAction(action_next_window);

    QWidget *spacer = new QWidget(m_toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setVisible(true);
    m_toolbar->addWidget(spacer);

    QLineEdit *quick_search = new QLineEdit(this);
    quick_search->setMaxLength(50);
    quick_search->setPlaceholderText("quick search...");
    connect(quick_search, &QLineEdit::returnPressed, this, &Gratuitous::quick_search_next);
    m_toolbar->addWidget(quick_search);

    QAction *action_find_next = new QAction;
    action_find_next->setShortcut(QKeySequence::FindNext);
    action_find_next->setIcon(QIcon::fromTheme("arrow-down"));
    action_find_next->setToolTip("find next (f3)");
    connect(action_find_next, &QAction::triggered, this, &Gratuitous::quick_search_next);
    m_toolbar->addAction(action_find_next);

    QAction *action_find_prev = new QAction;
    action_find_prev->setShortcut(QKeySequence::FindPrevious);
    action_find_prev->setIcon(QIcon::fromTheme("arrow-up"));
    action_find_prev->setToolTip("find previous (shift+f3");
    connect(action_find_prev, &QAction::triggered, this, &Gratuitous::quick_search_previous);
    m_toolbar->addAction(action_find_prev);

    addToolBar(m_toolbar);
    // This is a hack to get focus off of the quick_search bar at init
    m_mdi_area->setFocus();
}
