#pragma once

#include <QString>

namespace Constants
{
// Database related constants
namespace Database
{
const QString USER_DB_NAME = "user.db";
const QString DICT_DB_NAME = "dict.db";
const QString ECDICT_CSV_PATH = "/../3rdpart/ECDICT/ecdict.csv";

namespace TableNames
{
const QString USERS = "users";
const QString WORDS = "words";
const QString CACHE = "cache";
const QString USER_LEARNING = "user_learning";
const QString USER_FAVORITES = "user_favorites";
} // namespace TableNames

namespace Columns
{
namespace Users
{
const QString ID = "id";
const QString USERNAME = "username";
const QString PASSWORD_HASH = "password_hash";
const QString EMAIL = "email";
const QString AVATAR_PATH = "avatar_path";
const QString CREATED_AT = "created_at";
} // namespace Users

namespace Words
{
const QString ID = "id";
const QString WORD = "word";
const QString TRANSLATION = "translation";
const QString LANGUAGE = "language";
const QString PART_OF_SPEECH = "part_of_speech";
const QString EXAMPLES = "examples";
const QString EXAMPLE_TRANSLATION = "example_translation";
const QString FREQUENCY = "frequency";
const QString PRONUNCIATION = "pronunciation";
const QString NOTES = "notes";
} // namespace Words
} // namespace Columns
} // namespace Database

// URLs and external resources
namespace Urls
{
const QString BING_SEARCH = "https://www.bing.com/search?q=%1";
const QString GITHUB_REPO = "https://github.com/wyq777x/TranCE_Wyatt";
} // namespace Urls

// File paths and resources
namespace Resources
{
const QString DEFAULT_USER_AVATAR = ":/image/DefaultUser.png";
const QString APP_ICON = ":/image/learnENG.ico";
const QString ENG_ICON = ":/image/engICO.png";
} // namespace Resources

// Application directories
namespace Paths
{
const QString DB_DIR = "/Utility/DBs";
const QString AVATAR_DIR = "/Utility/Avatars";
const QString USER_PROFILES_DIR = "/Utility/UserProfiles";
} // namespace Paths

// Application settings
namespace Settings
{
const QString DEFAULT_FONT_FAMILY = "Noto Sans";
const int DEFAULT_FONT_SIZE = 16;
const int TITLE_FONT_SIZE = 24;
const int SUBTITLE_FONT_SIZE = 20;
const int SMALL_FONT_SIZE = 14;
} // namespace Settings

// UI dimensions and styling
namespace UI
{
const int DEFAULT_BUTTON_HEIGHT = 50;
const int DEFAULT_INPUT_HEIGHT = 50;
const int DEFAULT_INPUT_WIDTH = 450;
const int DEFAULT_BORDER_RADIUS = 20;
const int SMALL_BORDER_RADIUS = 6;
const int ICON_SIZE = 32;
const int DIALOG_MIN_WIDTH = 400;
const int DIALOG_MIN_HEIGHT = 250;
const int DIALOG_MAX_WIDTH = 450;
const int DIALOG_MAX_HEIGHT = 350;

const QString HISTORY_SEARCH_TEXT = "Enable History Search List";
const QString LANGUAGE_SETTING_TEXT = "Software Language";
const QString STATUS_ON = "ON";
const QString STATUS_OFF = "OFF";

} // namespace UI

// Languages
namespace Languages
{
const QString ENGLISH = "English";
const QString CHINESE = "Chinese";
const QString ENGLISH_CODE = "en";
const QString CHINESE_CODE = "zh";
} // namespace Languages

// Error messages and status codes
namespace Messages
{
const QString UNKNOWN_ERROR = "Unknown error";
const QString FIELDS_CANNOT_BE_EMPTY = "Fields cannot be empty.";
const QString USERNAME_CANNOT_BE_EMPTY = "Username cannot be empty.";
const QString DATABASE_LOAD_MESSAGE = "loaded from the database. ";
const QString NOT_FOUND_IN_DATABASE = "the database. ";
} // namespace Messages
} // namespace Constants
