#pragma once
#include "View/TempPage.h"
#include <utility>

class RecitePage : public TempPage
{
    Q_OBJECT
public:
    explicit RecitePage (QWidget *parent = nullptr);

    std::pair<int, int> getProgress () const
    {
        return {currentProgress, totalProgress};
    }
    void setProgress (int current, int total)
    {
        currentProgress = current;
        totalProgress = total;
    }

private:
    int currentProgress = 0;
    int totalProgress = 50;
};