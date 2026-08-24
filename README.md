# INTRODUCTION

This is a project to help the Chinese users to learn English.In other words, it is mainly a Chinese-English dictionary with some additional features.


# FEATURES

- **Dictionary**: You can search for the meaning of a word in English or Chinese.
- **Add words to vocabulary/Favorites list**: You can add words to your vocabulary list.
- **Quiz**: You can take a quiz to test your vocabulary.
- **Flashcards**: You can use flashcards to learn new words.


# TECHNOLOGIES USED

- **Languages**: `C++`,`SQLite`,`Python`,`TypeScript`
- **Dependencies**: `Qt`,`ElaWidgetsTools`,`SQLiteCpp`,`ECDICT`,`FastAPI`,`React`
- **Build System**: `CMake`
- **Pack-up Tools**: `Inno Setup`
- **Operating System**: `Linux` & `Windows`
- **Version Control**: `Git`
- **Database**: `SQLite`


# BUILD

The AI sidecar environment is set up automatically during the build:

- The Python virtualenv (`aisidecar/server/.venv`) is created and
  `requirements.txt` installed when missing.
- The web frontend (`aisidecar/web/dist`) is built with `npm install &&
  npm run build` when missing.

Prerequisites: **Python 3.11+** and **npm** on PATH - the configure step
fails without them. Both steps are self-healing and skipped when their
outputs already exist; force a re-run by deleting `.venv` / `web/dist`.
Release/packaging builds that bundle a PyInstaller sidecar can skip this
with `-DTRANCE_AI_SETUP_SIDECAR=OFF`. See `aisidecar/README.md` for
details.



# REFERENCES/THANKS TO


- **[Inno Setup](https://www.jrsoftware.org/isinfo.php)**
- **[Liniyous/ElaWidgetsTools](https://github.com/Liniyous/ElaWidgetTools)**
- **[SQLiteCpp](https://github.com/SRombauts/SQLiteCpp)**
- **[ECDICT](https://github.com/skywind3000/ECDICT)**