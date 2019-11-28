#ifndef PREVIEW_H
#define PREVIEW_H

#include <QDebug>
#include <QMdiSubWindow>
#include <QProcess>
#include <QSettings>

class Preview : public QObject
{
    Q_OBJECT
public:
    explicit Preview(QObject *parent = nullptr);

    void start(QString);
    void stop();
    void reload();
    void reload(QString);

    bool started() { return m_status; }

private:
    QString m_xephyr_command = "/usr/bin/Xephyr";
    QStringList xephyr_args();
    QString m_awesome_command = "/usr/bin/awesome";
    QStringList awesome_args(QString);

    QProcess *m_xephyr_process;
    QProcess *m_awesome_process;
    QProcess *m_id_process;

    bool m_status;

    QSettings m_prefs;

signals:
    void ended();
    void ready();
};

#endif // PREVIEW_H
