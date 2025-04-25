#pragma once

#include <QString>

struct WordEntry
{
    QString word;
    QString translation;
    QString language;
    QString partOfSpeech;  // 词性
    QString examples;      // 示例句子，可以是JSON格式
    int frequency;         // 词频统计
    QString pronunciation; // 发音
    QString notes;         // 附加笔记
};