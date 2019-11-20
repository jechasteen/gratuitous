#ifndef PREFS_H
#define PREFS_H

#include <QWidget>
#include <QtWidgets>

class Prefs : public QWidget
{
    Q_OBJECT
public:
    explicit Prefs(QWidget *parent = nullptr);

signals:

public slots:

private:
    void set_defaults();
    void setup_options();

    QVBoxLayout *m_vlayout;

    struct prefs {
        QFont *font;
    };

    struct prefs m_default;
    struct prefs m_current;
};

#endif // PREFS_H
