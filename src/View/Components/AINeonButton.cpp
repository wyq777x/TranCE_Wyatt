#include "AINeonButton.h"

#include <QEasingCurve>
#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>

namespace
{
constexpr qreal STROKE_WIDTH = 2.0;
constexpr qreal CORNER_RADIUS = 12.0;
constexpr int PULSE_DURATION_MS = 1600;

const QColor CYAN ("#00E5FF");  // start of the gradient
const QColor BLUE ("#2979FF");  // end of the gradient
const QColor GLOW_BASE ("#00C8FF"); // outer glow tint
} // namespace

AINeonButton::AINeonButton (QWidget *parent)
    : QWidget (parent), m_text (tr ("AI Mode"))
{
    setCursor (Qt::PointingHandCursor);
    setFocusPolicy (Qt::NoFocus);
    setAttribute (Qt::WA_Hover);

    // Outer glow.
    m_glowEffect = new QGraphicsDropShadowEffect (this);
    m_glowEffect->setOffset (0, 0);
    m_glowEffect->setBlurRadius (18);
    m_glowEffect->setColor (GLOW_BASE);
    setGraphicsEffect (m_glowEffect);

    // Breathing pulse animation (0 -> 1 -> 0, looping).
    m_pulseAnimation = new QPropertyAnimation (this, "pulse", this);
    m_pulseAnimation->setDuration (PULSE_DURATION_MS);
    m_pulseAnimation->setStartValue (0.0);
    m_pulseAnimation->setKeyValueAt (0.5, 1.0);
    m_pulseAnimation->setEndValue (0.0);
    m_pulseAnimation->setEasingCurve (QEasingCurve::InOutSine);
    m_pulseAnimation->setLoopCount (-1);

    startPulse ();
}

void AINeonButton::setText (const QString &text)
{
    m_text = text;
    update ();
}

QString AINeonButton::text () const
{
    return m_text;
}

QSize AINeonButton::sizeHint () const
{
    return QSize (200, 48);
}

void AINeonButton::activate ()
{
    stopPulse ();
}

void AINeonButton::paintEvent (QPaintEvent *event)
{
    Q_UNUSED (event);

    QPainter painter (this);
    painter.setRenderHint (QPainter::Antialiasing);

    const QRectF body = QRectF (rect ()).adjusted (
        STROKE_WIDTH, STROKE_WIDTH, -STROKE_WIDTH, -STROKE_WIDTH);

    // Soft outer glow stroke, its intensity breathing with m_pulse.
    qreal glowAlpha = 60.0 + m_pulse * 120.0;
    if (m_hovered)
        glowAlpha = qMin (glowAlpha + 60.0, 255.0);

    QColor glowColor = GLOW_BASE;
    glowColor.setAlphaF (glowAlpha / 255.0);

    QPen glowPen (glowColor, STROKE_WIDTH * 3.0);
    glowPen.setJoinStyle (Qt::RoundJoin);
    painter.setPen (glowPen);
    painter.setBrush (Qt::NoBrush);
    painter.drawRoundedRect (body, CORNER_RADIUS, CORNER_RADIUS);

    // Main cyan-blue gradient stroke.
    QLinearGradient gradient (body.topLeft (), body.bottomRight ());
    gradient.setColorAt (0.0, CYAN);
    gradient.setColorAt (1.0, BLUE);

    QPen strokePen (QBrush (gradient), STROKE_WIDTH);
    strokePen.setJoinStyle (Qt::RoundJoin);
    painter.setPen (strokePen);
    painter.setBrush (Qt::NoBrush);
    painter.drawRoundedRect (body, CORNER_RADIUS, CORNER_RADIUS);

    // Faint inner fill for depth.
    QColor fill (CYAN);
    fill.setAlphaF (m_hovered ? 0.08 : 0.04);
    painter.setPen (Qt::NoPen);
    painter.setBrush (fill);
    painter.drawRoundedRect (body, CORNER_RADIUS, CORNER_RADIUS);

    // Gradient label text.
    QLinearGradient textGradient (rect ().topLeft (), rect ().bottomRight ());
    textGradient.setColorAt (0.0, CYAN);
    textGradient.setColorAt (1.0, BLUE);

    QFont font = painter.font ();
    font.setPointSize (12);
    font.setBold (true);
    painter.setFont (font);
    painter.setPen (QPen (QBrush (textGradient), 1));
    painter.drawText (rect (), Qt::AlignCenter, m_text);
}

void AINeonButton::mousePressEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton)
    {
        m_pressed = true;
        update ();
    }
    QWidget::mousePressEvent (event);
}

void AINeonButton::mouseReleaseEvent (QMouseEvent *event)
{
    if (event->button () == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        update ();

        // First click activates the button and stops the pulse.
        activate ();
        emit clicked ();
    }
    QWidget::mouseReleaseEvent (event);
}

void AINeonButton::enterEvent (QEnterEvent *event)
{
    m_hovered = true;
    update ();
    QWidget::enterEvent (event);
}

void AINeonButton::leaveEvent (QEvent *event)
{
    m_hovered = false;
    update ();
    QWidget::leaveEvent (event);
}

qreal AINeonButton::pulse () const
{
    return m_pulse;
}

void AINeonButton::setPulse (qreal value)
{
    m_pulse = value;

    if (m_glowEffect)
    {
        m_glowEffect->setBlurRadius (14.0 + value * 26.0);

        QColor glowColor = GLOW_BASE;
        glowColor.setAlphaF ((90.0 + value * 90.0) / 255.0);
        m_glowEffect->setColor (glowColor);
    }

    update ();
}

void AINeonButton::startPulse ()
{
    if (m_pulseAnimation)
        m_pulseAnimation->start ();
}

void AINeonButton::stopPulse ()
{
    if (m_activated)
        return;

    m_activated = true;

    if (m_pulseAnimation)
        m_pulseAnimation->stop ();

    setPulse (0.45); // settle into a steady glow
}
