#pragma once
#include "Model/DictDbModel.h"
#include <QDir>
#include <memory>
#include <qtmetamacros.h>

class DbModelFactory
{

public:
    static std::unique_ptr<DictDbModel>
    createDictDbModel (const QString &dbPath)
    {
        QDir dir (dbPath);
        if (!dir.exists ())
        {
            dir.mkpath (dbPath);
        }
        QString dbFilePath = dir.absoluteFilePath ("dict.db");
        return std::make_unique<DictDbModel> (dbFilePath);
    }
};