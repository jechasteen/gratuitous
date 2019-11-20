#include "gratuitous.h"
#include "advancedsearch.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QMdiArea>
#include <QMessageBox>
#include <QStringList>
#include <QStandardPaths>
#include <QtWidgets>


Gratuitous::Gratuitous(QWidget *parent)
    : QMainWindow(parent)
{
    m_mdi_area = new QMdiArea;
    setCentralWidget(m_mdi_area);

    m_prefs = new Prefs;
    m_search_dock = new AdvancedSearch(this);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_search_dock);
    get_default_path();
    setup_menu_actions();
    add_menus();
    setup_toolbar();

    setWindowTitle("gratuitous");
}

// Utilities
void Gratuitous::get_default_path()
{
    // TODO: Preferences
    QString default_path;
    QStringList config_paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    for (const QString &path : config_paths)
    {
        QFileInfo dir(path);
        if (!dir.exists() || !dir.isDir() || !dir.isWritable())
            continue;

        dir.setFile(path + "/awesome");
        if (!dir.exists() || !dir.isDir() || !dir.isWritable())
            default_path = path;
        else
            default_path = path + "/awesome";
        break;
    }
    if (default_path.isEmpty())
        default_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];

    m_default_path = default_path;
}

void Gratuitous::do_save(LuaEdit *editor)
{
    LuaEdit *selected_editor = get_active_editor();
    if (editor == nullptr && selected_editor == nullptr)
        return;
    if (editor != nullptr)
        selected_editor = editor;

    if (selected_editor->filename().isEmpty())
    {
        QString filename = QFileDialog::getSaveFileName(this, tr("save as..."), m_default_path); // TODO: prefs
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
}

/*
 * Slots
 */

// File
void Gratuitous::open()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose file", m_default_path, "*.lua"); // TODO: config
    if (!filename.isEmpty())
        create_new_editor(filename);
}

void Gratuitous::save_as()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save as", m_default_path); // TODO: config
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
    if (m_search_dock->isVisible())
        m_search_dock->hide();
    else
        m_search_dock->show();
}

void Gratuitous::show_prefs_dialog()
{
    m_prefs->show();
}

// Window
void Gratuitous::close_window()
{
    LuaEdit *active_editor = get_active_editor();
    if (active_editor->document()->isModified())
    {
        QMessageBox msg;
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
}

// Help
void Gratuitous::show_about()
{
    QString text = "gratuitous " + VERSION + "\nbrought to you by jaylectric\nhttps://github.com/jaylectric/gratuitous";
    QMessageBox::about(this, "about gratuitohttps://www.qtcentre.org/threads/23840-how-align-some-buttons-in-QToolbar-from-right-to-leftus", text);
}

// Other
void Gratuitous::create_new_editor(QString filename)
{
    auto *new_window = new QMdiSubWindow;
    auto *new_editor = new LuaEdit(new_window, std::move(filename));

    new_window->setAttribute(Qt::WA_DeleteOnClose);
    new_window->setWidget(new_editor);
    new_window->setWindowTitle((new_editor->filename().isEmpty() ? "untitled" + QString::number(editor_count) : new_editor->filename()) + "[*]");
    m_mdi_area->addSubWindow(new_window)->showMaximized(); // TODO: Preferences

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

    connect(action_focus_window, &QAction::triggered, this, focus_window);
    new_editor->set_action(action_focus_window);
    m_mdi_area->setActiveSubWindow(new_window);
}

void Gratuitous::editor_was_modified(bool changed)
{
    auto *active_editor = m_mdi_area->activeSubWindow()->findChild<LuaEdit*>();
    active_editor->document()->setModified(changed);
    active_editor->parentWidget()->setWindowModified(changed);
}

void Gratuitous::handle_quick_search()
{
    auto active_editor = get_active_editor();
    const QString query = m_toolbar->findChild<QLineEdit*>()->text();
    active_editor->find(query);
}

void Gratuitous::quick_search_next()
{
    handle_quick_search();
}

void Gratuitous::quick_search_previous()
{
    auto active_editor = get_active_editor();
    const QString query = m_toolbar->findChild<QLineEdit*>()->text();
    active_editor->find(query, QTextDocument::FindBackward);
}

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

    action_exit = new QAction(tr("exit"), this);
    action_exit->setIcon(QIcon::fromTheme("application-exit"));
    action_exit->setStatusTip(tr("close all documents and exit gratuitous"));
    connect(action_exit, &QAction::triggered, this, &Gratuitous::exit);

    // Edit
    action_focus_quick_find = new QAction(tr("quick find..."));
    action_focus_quick_find->setShortcut(QKeySequence::Find);
    action_focus_quick_find->setIcon(QIcon::fromTheme("edit-find"));
    action_focus_quick_find->setStatusTip(tr("search in the current document"));
    connect(action_focus_quick_find, &QAction::triggered, this, &Gratuitous::focus_quick_find);

    action_show_find_dock = new QAction(tr("advanced search..."), this);
    action_show_find_dock->setIcon(QIcon::fromTheme("system-search"));
    action_show_find_dock->setStatusTip(tr("open the advanced find window"));
    connect(action_show_find_dock, &QAction::triggered, this, &Gratuitous::toggle_search_dock_visible);

    action_show_prefs_dialog = new QAction(tr("preferences..."), this);
    action_show_prefs_dialog->setIcon(QIcon::fromTheme("preferences-desktop"));
    action_show_prefs_dialog->setShortcut(QKeySequence::Preferences);
    action_show_prefs_dialog->setStatusTip(tr("open the preferences dialog"));
    connect(action_show_prefs_dialog, &QAction::triggered, this, &Gratuitous::show_prefs_dialog);

    // View
    action_show_preview = new QAction(tr("preview..."), this);
    action_show_preview->setIcon(QIcon::fromTheme("media-playback-start"));
    action_show_preview->setShortcut(QKeySequence::FullScreen);
    action_show_preview->setStatusTip(tr("open the preview window"));
    connect(action_show_preview, &QAction::triggered, this, &Gratuitous::show_preview);

    action_reload_preview = new QAction(tr("reload preview"), this);
    action_reload_preview->setIcon(QIcon::fromTheme("view-refresh"));
    action_show_preview->setShortcut(QKeySequence::Refresh);
    action_reload_preview->setStatusTip(tr("send a reload signal to awesome"));
    connect(action_reload_preview, &QAction::triggered, this, &Gratuitous::reload_preview);

    // Window
    action_tile_windows = new QAction(tr("tile"), this);
    action_tile_windows->setStatusTip(tr("tile all currently open windows"));
    connect(action_tile_windows, &QAction::triggered, this, &Gratuitous::tile_windows);

    action_cascade_windows = new QAction(tr("cascade"), this);
    action_cascade_windows->setStatusTip(tr("cascade all currently open windows"));
    connect(action_cascade_windows, &QAction::triggered, this, &Gratuitous::cascade_windows);

    action_close_window = new QAction(tr("close"), this);
    action_close_window->setIcon(QIcon::fromTheme("window-close"));
    action_close_window->setShortcut(QKeySequence::Close);
    action_close_window->setStatusTip(tr("close window"));
    connect(action_close_window, &QAction::triggered, this, &Gratuitous::close_window);

    action_close_all_windows = new QAction(tr("close all"), this);
    action_close_all_windows->setStatusTip(tr("close all open windows"));
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
    menu_file->addSeparator();
    menu_file->addAction(action_exit);

    menu_edit = menuBar()->addMenu(tr("&edit"));
    menu_edit->addAction(action_show_find_dock);
    menu_edit->addAction(action_focus_quick_find);
    menu_edit->addAction(action_show_prefs_dialog);

    menu_view = menuBar()->addMenu(tr("&view"));
    menu_view->addAction(action_show_preview);
    menu_view->addAction(action_reload_preview);

    menu_window = menuBar()->addMenu(tr("&window"));
    menu_window->addAction(action_tile_windows);
    menu_window->addAction(action_cascade_windows);
    menu_window->addSeparator();
    menu_window->addAction(action_close_window);
    menu_window->addAction(action_close_all_windows);

    menu_help = menuBar()->addMenu(tr("&help"));
    menu_help->addAction(action_show_awesome_docs);
    menu_help->addAction(action_show_lua_reference);
    menu_help->addSeparator();
    menu_help->addAction(action_show_about);
}

void Gratuitous::setup_toolbar()
{
    m_toolbar = new QToolBar;
    m_toolbar->addAction(action_new);
    m_toolbar->addAction(action_open);
    m_toolbar->addAction(action_save);
    m_toolbar->addSeparator();
    m_toolbar->addAction(action_show_find_dock);
    m_toolbar->addSeparator();
    m_toolbar->addAction(action_show_preview);
    m_toolbar->addAction(action_reload_preview);

    QWidget *spacer = new QWidget(m_toolbar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacer->setVisible(true);
    m_toolbar->addWidget(spacer);

    QLineEdit *quick_search = new QLineEdit(this);
    quick_search->setPlaceholderText("quick search...");
    connect(quick_search, &QLineEdit::returnPressed, this, &Gratuitous::handle_quick_search);
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
