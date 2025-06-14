# Reading and writing files.

Functions are provided trough scripting functions that start with `fs_`. Older file-related scripting functions will be deprecated in the future for safety and security reasons.

For a list of scripting functions, check the `fs_` ScriptThread functions in the **Game Module Classes**, and functions in the class `FSFile`.

## Examples

### Reading files

Here is an example on how to read files:

```cpp
main:
    // This will read test_file.txt using the engine file API.
    // The file can either be in a pk3 file, or in one of the main folders of the game.
    local.content = fs_read_content "test_file.txt"

    // Prints its content
    println ("File content: " + local.content)
end
```

### Writing files

Here is an example on how to write to a file:

```cpp
main:
    local.content = "Hello, world!"

    // Create (or replace) a file with the specified content
    fs_write_content "test_file.txt" local.content

    // The file will only have:
    // Hello, world!
    // Read it back
    local.content = fs_read_content "test_file.txt"

    // Prints its content
    println ("File content: " + local.content)
end
```
