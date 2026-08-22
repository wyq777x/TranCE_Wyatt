#pragma once

#include <QString>
#include <QWidget>

class QEnterEvent;
class QEvent;
class QGraphicsDropShadowEffect;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;

/**
 * @brief A neon-styled button with a cyan-blue gradient stroke and a
 *        breathing outer glow.
 *
 * The button pulses (breathing animation) until it is activated for the
 * first time, after which the pulse stops and a steady glow remains.
 */
class AINeonButton : public QWidget
{
    Q_OBJECT
    // Breathing phase in [0, 1], driven by QPropertyAnimation.
    Q_PROPERTY (qreal pulse READ pulse WRITE setPulse)

public:
    explicit AINeonButton (QWidget *parent = nullptr);

    void setText (const QString &text);
    QString text () const;

    QSize sizeHint () const override;

    /**
     * @brief Stop the breathing pulse and settle into a steady glow.
     * Called automatically on the first click.
     */
    void activate ();

signals:
    void clicked ();

protected:
    void paintEvent (QPaintEvent *event) override;
    void mousePressEvent (QMouseEvent *event) override;
    void mouseReleaseEvent (QMouseEvent *event) override;
    void enterEvent (QEnterEvent *event) override;
    void leaveEvent (QEvent *event) override;

private:
    qreal pulse () const;
    void setPulse (qreal value);

    void startPulse ();
    void stopPulse ();

    QString m_text;
    qreal m_pulse = 0.0;
    bool m_activated = false;
    bool m_hovered = false;
    bool m_pressed = false;

    QPropertyAnimation *m_pulseAnimation = nullptr;
    QGraphicsDropShadowEffect *m_glowEffect = nullptr;
};
