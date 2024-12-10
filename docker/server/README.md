# Server image

## Introduction

Contains Dockerfile for building openmohaa-based server on Debian Bookworm. MOHAA data must be stored in `/usr/local/share/mohaa`, the directory structure should look like this:
```
[/usr/local/share/mohaa]
├── [main]
│   └── Pak*.pk3
├── [mainta]
│   └── pak*.pk3
├── [maintt]
│   └── pak*.pk3
└── [home]
    ├── [main]
    │   └── Custom PK3s go here
    ├── [mainta]
    │   └── Custom PK3s go here
    └── [maintt]
        └── Custom PK3s go here
```

`sound` and `video` folders are not needed.

There are two ways to handle game data:
1) Create a new image based on this image (with a new Dockerfile) and copy the game directory in `/usr/local/share/mohaa`. This is the recommended solution because game data should be immutable. For mods, a base image can be created, or a volume can be mounted to `/usr/local/share/mohaa/home` which contains files (in main, mainta or maintt subdir) that will modify the game.

or

2) Mount a volume to the game directory directly in `/usr/local/share/mohaa`.

## Building

1) Put the game data (`main`, `mainta` and `maintt`) inside `full/game`, it should match the directory structure above
2) Execute one of the build.* scripts (depending on if you are running Linux or Windows).

## Starting

The server can now be executed using `docker run -p 12203:12203 -p 12300:12300 openmohaa/full`, or by using a docker composition file.
