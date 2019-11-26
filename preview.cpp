#include "preview.h"

#include <csignal>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <QMdiArea>
#include <QMainWindow>
#include <QObject>
#include <QProcessEnvironment>
#include <QStringList>
#include <QWindow>

Preview::Preview(QObject *parent)
    : QObject(parent)
{
    m_xephyr_process = new QProcess;
    m_awesome_process = new QProcess;

    QStringList awesome_args;
    m_status = false;

    connect(m_xephyr_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exit_code, QProcess::ExitStatus exit_status) {
                qDebug() << "Xephyr quit.\nExit Code" << exit_code << "\nExit status:" << exit_status;
                m_awesome_process->kill();
                emit preview_ended();
            }
    );

    connect(m_awesome_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exit_code, QProcess::ExitStatus exitStatus){
                qDebug() << "Awesome quit.\nExit code:" << exit_code << "\nExit status:" << exitStatus;
                m_xephyr_process->kill();
                emit preview_ended();
            }
    );

    connect(m_xephyr_process,
            &QProcess::readyReadStandardOutput,
            [=](){
                qDebug() << m_xephyr_process->readAllStandardOutput();
            }
    );

    connect(m_awesome_process,
            &QProcess::readyReadStandardOutput,
            [=](){
                qDebug() << m_awesome_process->readAllStandardOutput();
            }
    );

    connect(m_xephyr_process,
            &QProcess::readyReadStandardError,
            [=](){
                qDebug() << m_xephyr_process->readAllStandardError();
            }
    );

    connect(m_awesome_process,
            &QProcess::readyReadStandardError,
            [=](){
                qDebug() << m_awesome_process->readAllStandardError();
            }
    );
}

void Preview::start(QString filename)
{
    if (filename.isEmpty()) return;

    m_xephyr_process = new QProcess;
    m_awesome_process = new QProcess;

    m_xephyr_process->start(m_xephyr_command, xephyr_args());
    if (m_xephyr_process->waitForStarted())
    {
        qDebug() << "Xephyr started";

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("DISPLAY", ":1.0");
        m_awesome_process->setProcessEnvironment(env);
        m_awesome_process->start(m_awesome_command, awesome_args(filename));
        if (m_awesome_process->waitForStarted())
        {
            qDebug() << "awesome started";
            m_status = true;
            emit ready();
        }
        else
        {
            qDebug() << "something went wrong starting awesome" << m_awesome_process->errorString();
        }
    }
    else
    {
        qDebug() << "something went wrong starting Xephyr" << m_xephyr_process->errorString();
    }
}

void Preview::stop()
{
    if (m_xephyr_process->state() == QProcess::ProcessState::Running)
    {
        if (m_awesome_process->state() == QProcess::ProcessState::Running)
            m_awesome_process->close();
        m_xephyr_process->close();
    }

    m_status = false;
    delete m_xephyr_process;
    delete m_awesome_process;
    emit preview_ended();
}

void Preview::reload()
{
    if (m_awesome_process->state() == QProcess::ProcessState::Running)
    {
        qint64 pid = m_awesome_process->processId();
        kill(static_cast<int>(pid), SIGHUP);
    }
}

// overloaded function in case we want to open a different config than previously
void Preview::reload(QString filename)
{
    if (m_awesome_process->state() == QProcess::ProcessState::Running)
    {
        m_awesome_process->close();
        if (m_awesome_process->waitForFinished())
        {
            m_awesome_process->start(m_awesome_command, awesome_args(filename));
        }
        else
            qDebug() << "tried to restart with new rc.lua, failed to stop awesome process";
    }
}

QStringList Preview::awesome_args(QString filename)
{
    QStringList args;
    args << "-c" << filename;
    return args;
}

QString exec(const char* command)
{
    std::array<char, 128> buffer;
    QString result;
    int count = 0;
    while (result.isEmpty() && count < 10) {
        std::unique_ptr<FILE, decltype (&pclose)> pipe(popen(command, "r"), pclose);
        if (!pipe)
        {
            qDebug() << "failed to open command" << command;
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        count++;
    }
    return result;
}

void Preview::set_win_id()
{
    const char command[] = "/etc/gratuitous/get_preview_id.sh";
    QString id = exec(command);
    m_win_id = id.trimmed();
    qDebug() << "Preview window id:" << m_win_id;
}

QStringList Preview::xephyr_args()
{
    QStringList args;
    QString width = QString::number(m_prefs.value("preview/width").value<int>());
    QString height = QString::number(m_prefs.value("preview/height").value<int>());
    args << ":1" << "-ac" << "-br" << "-noreset" << "-screen" << width + "x" + height;
    qDebug() << "Arguments passed to Xephyr:\n" << args;

    return args;
}
