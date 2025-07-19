# Contributing guidelines

Whether you're reporting issues or coding, follow the guidelines below to help keep the project consistent and maintainable.

## Issues

When opening a new issue, include the following information to help address the problem effectively:
- Accurate description of the bug.
- Steps to reproduce.
- Log file (`qconsole.log`) and crash output if applicable:
  - Attach the relevant log file
  - For crashes, paste the console output that includes the backtrace.
- Operating System and version. Example: `Debian 12`.
- Full game version. Example: `0.70.0-alpha+0.0b1a20dcf win_msvc64-x86_64-debug`. The version is in the log file or can be obtained when typing `version`.

## Coding

### Compatibility

Any changes must be retro-compatible. OpenMoHAA must be compatible as best as possible with the original game:
- Assets must be loaded correctly.
- Changes related to networking must be compatible with existing MOHAA clients and servers, these changes must not cause an error/disconnect.
- Scripts/mods must work as best as possible.
- The singleplayer campaign must be fully playable.

### Naming & Formatting

- Use **camelCase** notation for all variables, and **PascalCase** notation for functions. Even if some existing code doesn't.
- Use `clang-format` to format your code. For example, in VSCode from the command palette: **Format document**

### Event declaration

When declaring a new Event, use the following structure:
```cpp
Event EV_YourEventName  // Pascal Case naming convention
(
    "name",             // Each parameter on a new line
    flags,
    "format specifiers...",
    "argument names...",
    "description"
);
```

### Code annotations

When adding a new class, or when making changes to existing classes/code that differ from the original game, use one of the following comment to annotate your changes:

#### Additions

```cpp
// Added in OPM
//  Description
```

#### Changes

```cpp
// Changed in OPM
//  Description
```

#### Fixes

```cpp
// Fixed in OPM
//  Description
```

#### Removal

```cpp
// Removed in OPM
//  Description
```

If the change matches a specific version of the original game, replace `OPM` with the game version, for example:
```cpp
// Added in 2.11
//  Make sure to clean turret stuff up
//  when the player is deleted
```
The version can be `2.0`, `2.1`, `2.11`, `2.15`, `2.30`, `2.40`.

You can also group multiple related functions like this:
```cpp
// Added in OPM
//====
void Function1();
void Function2();
void Function3();
//====
```

If it's a new source file, the comment must be added right after the license notice.

### Source files

- Always put the license notice as the header of the file. The notice can be obtained [here](./02-license-header.md).
- Only #include files that are really needed.
- Source files must have classes and functions related to the feature.
