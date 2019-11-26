#ifndef PREVIEW_H
#define PREVIEW_H

#include <QDebug>
#include <QProcess>
#include <QMdiSubWindow>

class Preview : public QObject
{
    Q_OBJECT
public:
    explicit Preview(QObject *parent = nullptr);

    void start(QString);
    void stop();
    void reload();
    void reload(QString);
    QMdiSubWindow *window() { return m_window; }

    bool started() { return m_status; }
    QString get_win_id() const { return m_win_id; }
    void set_win_id();

private:
    QString m_xephyr_command = "/usr/bin/Xephyr";
    QStringList xephyr_args();
    QString m_awesome_command = "/usr/bin/awesome";
    QStringList awesome_args(QString);

    QProcess *m_xephyr_process;
    QProcess *m_awesome_process;
    QProcess *m_id_process;

    QMdiSubWindow *m_window;
    QString m_win_id;

    bool m_status;


signals:
    void preview_ended();
    void ready();
};

#endif // PREVIEW_H
