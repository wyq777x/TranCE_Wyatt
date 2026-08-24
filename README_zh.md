# 简介

这是一个帮助中文用户学习英文的项目. 换句话说, 就主要是一个类似于中英词典一样,又带有一些额外功能的软件.


# 特性

- **词典**: 你可以以本地或在线搜索(Bing)的方式搜索一个字/词的中英文释义.
- **添加单词到收藏页面**: 你可以将单词加入收藏的单词列表.
- **问答**: 你可以通过问答游戏来进行词汇水平测试.
- **词卡**: 你可以通过词卡来学习新单词.


# 使用技术

- **语言**: `C++`,`SQLite`,`Python`,`TypeScript`
- **依赖库**: `Qt`,`ElaWidgetsTools`,`SQLiteCpp`,`ECDICT`,`FastAPI`,`React`
- **构建系统**: `CMake`
- **打包工具**: `Inno Setup`
- **支持的操作系统**: `Linux` & `Windows`
- **版本控制**: `Git`
- **数据库**: `SQLite`


# 构建

AI sidecar 环境在构建时自动准备:

- Python 虚拟环境 (`aisidecar/server/.venv`) 缺失时自动创建并安装
  `requirements.txt`;
- Web 前端 (`aisidecar/web/dist`) 缺失时自动执行 `npm install &&
  npm run build`。

前置要求:PATH 上需有 **Python 3.11+** 与 **npm**,缺失时 configure 阶段
直接报错。两步均为自愈式:产物已存在则跳过,增量构建零开销;删除
`.venv` / `web/dist` 即可强制重跑。发布打包若捆绑 PyInstaller 产物,
可用 `-DTRANCE_AI_SETUP_SIDECAR=OFF` 跳过。详见 `aisidecar/README.md`。


# 引用第三方项目/鸣谢


- **[Inno Setup](https://www.jrsoftware.org/isinfo.php)**
- **[Liniyous/ElaWidgetsTools](https://github.com/Liniyous/ElaWidgetTools)**
- **[SQLiteCpp](https://github.com/SRombauts/SQLiteCpp)**
- **[ECDICT](https://github.com/skywind3000/ECDICT)**