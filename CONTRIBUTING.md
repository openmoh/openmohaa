# Contributing guidelines

Whether you're reporting issues or coding, follow the guidelines below to help keep the project consistent and maintainable.

All contributions, including **issues**, **discussions**, **code comments**, **commit messages**, and **documentation**, must be written in *English*. This ensures clarity and consistency for all contributors.

There are many ways to contribute, like:
- Reporting an [issue](#issues).
- Submitting [pull request](#pull-requests).
- Suggesting enhancements.
- Testing.

---

## Issues

Before opening a new issue:
1. Make sure you're running the [latest development version](https://github.com/openmoh/openmohaa/releases/tag/dev).
2. Check if the issue already exists.

When filing an issue, include as much detail as possible, this helps reproducing and fixing the problem effectively:
- Clear description of the bug.
- Steps to reproduce.
- Log file (`qconsole.log`) and crash output if applicable:
    - Attach the relevant log file.
    - For crashes, paste the console output that includes the backtrace.
- Operating System and version. Example: `Debian 13`.
- Full game version. Example: `0.70.0-alpha+0.0b1a20dcf win_msvc64-x86_64-debug`. The version is obtained at the start of the log file, or by executing the `version` command in console.

## Coding

### Compatibility

All changes must stay retro-compatible with the original game:
- Assets must be loaded correctly.
- Changes related to networking must be compatible with existing MOHAA clients and servers, these changes must not cause an error/disconnect.
- Scripts/mods must remain functional.
- The singleplayer campaign must be fully playable.

### Coding style & Guidelines

- Match the existing code style in any files you modify.
- Avoid personal tags or author comments (.e.g., `// @Name: note`). Git already tracks authorship.
- Ensure the code builds successfully using CI.
- Use **camelCase** notation for all variables, and **PascalCase** notation for functions. Even if some existing code doesn't.
- Format code using `clang-format`.
    - In VSCode from the command palette: **Format document**

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

When modifying existing code from the original game or adding new classes, annotate your changes using one of the following patterns:

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

If referencing a specific game version, replace OPM with that version number. Example:
```cpp
// Added in 2.11
//  Make sure to clean turret stuff up
//  when the player is deleted
```
Known versions are `1.00`, `1.10`, `1.11`, `2.0`, `2.1`, `2.11`, `2.15`, `2.30`, `2.40`.

You can also group multiple related functions like this:
```cpp
// Added in OPM
//====
void Function1();
void Function2();
void Function3();
//====
```

For new source files, include the annotation after the license notice.

### Source files

- Always put the license notice as the header of the file (template [here](docs/markdown/05-contributing/01-license-header.md)).
- Only `#include` what's necessary.
- Each source file should contain related classes and functions.

---

## Pull Requests

Pull Requests are the preferred way of submitting code changes.

### Before you start

1. **Discuss major changes first**: open an issue or discuss about it.
2. **Create a branch** based on the `main` branch (you shouldn't commit directly on `main`)
3. **Keep PRs as small as possible and focused**: One clear purpose per PR.

### Submitting a PR

- Target the correct branch (usually `main`).
- Add a short and clear description of what you changed and why.
- Reference matching issues (e.g. `Fixes #123`).
- Make sure your code builds and passes tests.
- Test locally to ensure your changes don't break functionality or compatibility.

### Reviews

- Maintainers may ask for tweaks or commit changes directly to your PR, that's normal.
- Stay polite and technical.
- Once approved, your changes will be merged.

### Additional Notes

- Contributions follow the same license as the project.
- Keep things consistent with existing code and docs.
- If you're unsure about anything, you can discuss it on Discord via [this link](https://discord.gg/NYtH58R).
