# File layout

| Path                 | Description                                                      |
| ---------------------|-------------------------------------------------------------------
| code/                | OpenMoHAA source code                                            |
| code/cgame           | Client game code                                                 |
| code/client          | Client (network stuff, snapshots, etc)                           |
| code/game            | Server game code (game logic stuff, etc)                         |
| code/gamespy         | Gamespy SDK                                                      |
| code/null            | Null redirector (used by the OpenMoHAA Dedicated Server)         |
| code/parser          | Flex/Bison parser                                                |
| code/qcommon         | Global shared stuff such as maths                                |
| code/renderer        | OpenMoHAA renderer                                               |
| code/sdl(12)         | SDL code                                                         |
| code/script          | Script engine                                                    |
| code/server          | Server code (network, snapshots, handling clients etc)           |
| code/skeletor        | Skeleton model engine                                            |
| code/sys             | Main application system                                          |
| code/tiki            | Complete TIKI engine                                             |
| code/uilib           | UI library (Ubertools stuff)                                     |
| code/{win32,unix}    | Platform-specific code                                           |
| docker               | Docker files, for compiling on various linux versions            |
| docs                 | Documentation stuff                                              |
| misc/                | Project stuff, definition files and various resources stuff.     |
