#pragma once

#include "Def.h"
#include <QString>

namespace Constants
{
// Database related constants
namespace Database
{
const QString USER_DB_NAME = QStringLiteral ("user.db");
const QString DICT_DB_NAME = QStringLiteral ("dict.db");
const QString ECDICT_CSV_PATH =
    QStringLiteral ("/../3rdpart/ECDICT/ecdict.csv");

namespace TableNames
{
const QString USERS = QStringLiteral ("users");
const QString WORDS = QStringLiteral ("words");
const QString CACHE = QStringLiteral ("cache");
const QString USER_LEARNING = QStringLiteral ("user_learning");
const QString USER_FAVORITES = QStringLiteral ("user_favorites");
} // namespace TableNames

namespace Columns
{
namespace Users
{
const QString ID = QStringLiteral ("id");
const QString USERNAME = QStringLiteral ("username");
const QString PASSWORD_HASH = QStringLiteral ("password_hash");
const QString EMAIL = QStringLiteral ("email");
const QString AVATAR_PATH = QStringLiteral ("avatar_path");
const QString CREATED_AT = QStringLiteral ("created_at");
} // namespace Users

namespace Words
{
const QString ID = QStringLiteral ("id");
const QString WORD = QStringLiteral ("word");
const QString TRANSLATION = QStringLiteral ("translation");
const QString LANGUAGE = QStringLiteral ("language");
const QString PART_OF_SPEECH = QStringLiteral ("part_of_speech");
const QString EXAMPLES = QStringLiteral ("examples");
const QString EXAMPLE_TRANSLATION = QStringLiteral ("example_translation");
const QString FREQUENCY = QStringLiteral ("frequency");
const QString PRONUNCIATION = QStringLiteral ("pronunciation");
const QString NOTES = QStringLiteral ("notes");
} // namespace Words
} // namespace Columns
} // namespace Database

// URLs and external resources
namespace Urls
{
const QString BING_SEARCH = QStringLiteral ("https://www.bing.com/search?q=%1");
const QString GITHUB_REPO =
    QStringLiteral ("https://github.com/wyq777x/TranCE_Wyatt");
} // namespace Urls

// File paths and resources
namespace Resources
{
const QString DEFAULT_USER_AVATAR = QStringLiteral (":/image/DefaultUser.png");
const QString APP_ICON = QStringLiteral (":/image/learnENG.ico");
const QString ENG_ICON = QStringLiteral (":/image/engICO.png");
} // namespace Resources

// Application directories
namespace Paths
{
const QString DB_DIR = QStringLiteral ("/Utility/DBs");
const QString AVATAR_DIR = QStringLiteral ("/Utility/Avatars");
const QString USER_PROFILES_DIR = QStringLiteral ("/Utility/UserProfiles");
} // namespace Paths

// Application settings
namespace Settings
{
const QString DEFAULT_FONT_FAMILY = QStringLiteral ("Noto Sans");
constexpr int DEFAULT_FONT_SIZE = 16;
constexpr int TITLE_FONT_SIZE = 24;
constexpr int SUBTITLE_FONT_SIZE = 20;
constexpr int SMALL_FONT_SIZE = 14;
} // namespace Settings

namespace JsonFields
{

namespace userAccount
{
const QString FIELDNAME = QStringLiteral ("userAccount");
const QString USERNAME = QStringLiteral ("username");
const QString USER_UUID = QStringLiteral ("user_uuid");
} // namespace userAccount

namespace appSettings
{
const QString FIELDNAME = QStringLiteral ("appSettings");
const QString LANGUAGE = QStringLiteral ("language");
const QString HISTORY_LIST_ENABLED = QStringLiteral ("HistoryListEnabled");
} // namespace appSettings

} // namespace JsonFields

// UI dimensions and styling
namespace UI
{
constexpr int DEFAULT_BUTTON_HEIGHT = 50;
constexpr int DEFAULT_INPUT_HEIGHT = 50;
constexpr int DEFAULT_INPUT_WIDTH = 450;
constexpr int DEFAULT_BORDER_RADIUS = 20;
constexpr int SMALL_BORDER_RADIUS = 6;
constexpr int ICON_SIZE = 32;
constexpr int DIALOG_MIN_WIDTH = 400;
constexpr int DIALOG_MIN_HEIGHT = 250;
constexpr int DIALOG_MAX_WIDTH = 450;
constexpr int DIALOG_MAX_HEIGHT = 350;

const QString HISTORY_SEARCH_TEXT =
    QStringLiteral ("Enable History Search List");
const QString LANGUAGE_SETTING_TEXT = QStringLiteral ("Software Language");
const QString STATUS_ON = QStringLiteral ("ON");
const QString STATUS_OFF = QStringLiteral ("OFF");

const ElaIconType::IconName UNLIKE_ICON = ElaIconType::Heart;
const ElaIconType::IconName LIKE_ICON = ElaIconType::HeartCircleCheck;

} // namespace UI

// Languages
namespace Languages
{
const QString ENGLISH = QStringLiteral ("English");
const QString CHINESE = QStringLiteral ("Chinese");
const QString ENGLISH_CODE = QStringLiteral ("en");
const QString CHINESE_CODE = QStringLiteral ("zh");
} // namespace Languages

// Error messages and status codes
namespace Messages
{
const QString UNKNOWN_ERROR = QStringLiteral ("Unknown error");
const QString FIELDS_CANNOT_BE_EMPTY =
    QStringLiteral ("Fields cannot be empty.");
const QString USERNAME_CANNOT_BE_EMPTY =
    QStringLiteral ("Username cannot be empty.");
const QString DATABASE_LOAD_MESSAGE =
    QStringLiteral ("loaded from the database. ");
const QString NOT_FOUND_IN_DATABASE = QStringLiteral ("the database. ");
} // namespace Messages

} // namespace Constants
