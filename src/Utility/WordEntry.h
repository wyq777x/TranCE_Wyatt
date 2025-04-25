#pragma once

#include <QString>

struct WordEntry
{
    QString word;
    QString translation;
    QString language;
    QString partOfSpeech;  // Part of speech (e.g., noun, verb)
    QString examples;      // Example sentences
    int frequency;         // Word frequency in the language
    QString pronunciation; // Pronunciation guide
    QString notes;         // Additional notes or comments
};