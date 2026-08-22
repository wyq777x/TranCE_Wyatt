#pragma once

#include <QObject>
#include <QString>

/**
 * @brief Read-only tap for learning-behaviour signals consumed by the AI
 *        subsystem (learner mastery model).
 *
 * The tap is deliberately dependency-free: existing code calls the static
 * notify* helpers after a successful persistence operation; when AI mode
 * is off nobody listens to the signals and the cost is a no-op signal
 * emission. This keeps the "offline features must not depend on AI"
 * guarantee while giving the sidecar's learner model its event stream.
 */
class AiEventTap : public QObject
{
    Q_OBJECT

public:
    static AiEventTap &getInstance ()
    {
        static AiEventTap instance;
        return instance;
    }

    AiEventTap (const AiEventTap &) = delete;
    AiEventTap &operator= (const AiEventTap &) = delete;

    // A quiz answer was judged (Recite quiz flow).
    static void notifyQuizAnswered (const QString &word, bool correct)
    {
        emit getInstance ().quizAnswered (word, correct);
    }

    // A word was looked up in the dictionary.
    static void notifyWordLookedUp (const QString &word)
    {
        emit getInstance ().wordLookedUp (word);
    }

    // A word went through the recite flow.
    static void notifyWordRecited (const QString &word)
    {
        emit getInstance ().wordRecited (word);
    }

    // A word was added to / removed from favorites.
    static void notifyFavoriteChanged (const QString &word, bool favorite)
    {
        emit getInstance ().favoriteChanged (word, favorite);
    }

    // vocabulary status changed (0 = learning, 1 = mastered).
    static void notifyWordStatusChanged (const QString &word, int status)
    {
        emit getInstance ().wordStatusChanged (word, status);
    }

signals:
    void quizAnswered (const QString &word, bool correct);
    void wordLookedUp (const QString &word);
    void wordRecited (const QString &word);
    void favoriteChanged (const QString &word, bool favorite);
    void wordStatusChanged (const QString &word, int status);

private:
    explicit AiEventTap () = default;
};
