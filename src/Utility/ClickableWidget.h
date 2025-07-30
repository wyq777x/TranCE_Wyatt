#pragma once

#include <QEvent>
#include <QWidget>

class ClickableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickableWidget (QWidget *parent = nullptr)
    {
        this->installEventFilter (this);
    }

signals:
    void clicked ();

protected:
    bool eventFilter (QObject *watched, QEvent *event) override
    {
        if (watched == this && event->type () == QEvent::MouseButtonRelease)
        {
            emit clicked ();
            return true;
        }
        return QWidget::eventFilter (watched, event);
    }
};