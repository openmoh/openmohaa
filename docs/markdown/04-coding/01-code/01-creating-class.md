# Creating a new class

This documentation will guide you through creating a new class for scripts, inside the game code.

## Creating source files

The best practice is for each class to have their own source file.

Let's call our class `ExampleObject`, derived from `SimpleEntity`. inside the `code/fgame`, create 2 source files:
  - `exampleobject.h` for the header
  - `exampleobject.cpp` for the source

## Preparing the header file

The header file will contain the class prototype and will use `CLASS_PROTOTYPE(classname)` to define the information about the class for scripts to use it.
Sample code for the class:

```cpp
#include "simpleentity.h"

class ExampleObject : public SimpleEntity {
    CLASS_PROTOTYPE(ExampleObject);

public:
    ExampleObject();
    ~ExampleObject();
};
```

## Declaring the class in the source file

The next step is to declare the class. The class declaration defines the information for the class, such as the parent class, the class name, the class id (which is an alternate name) and the list of events that the class supports.

```cpp
#include "exampleobject.h"
#include "g_main.h" // for printing

// The first argument is the parent (SimpleEntity)
// The second argument is the class name (ExampleObject)
// The this argument (can be NULL) is the alternate class name alias the class ID (info_exampleobject)
//
// The class can be spawned using "local.ent = spawn ExampleObject" or "local.ent = spawn info_exampleobject"
CLASS_DECLARATION(SimpleEntity, ExampleObject, "info_exampleobject")
{
    // This line is mandatory and defines the end of the event response
    {NULL, NULL}
};

// The class constructor
ExampleObject::ExampleObject()
{
    gi.Printf("Hello, world!\n");
}

// The class destructor
ExampleObject::~ExampleObject()
{
    gi.Printf("I'm being deleted!\n");
}
```

## Creating script commands for the class

Each class has a response list, containing a list of `Event` followed by the associated method for each.
This is the constructor for `Event`, inside `code/qcommon/listener.h`:
```cpp
/**
 * @brief Construct a new Event object
 * 
 * @param command The command name
 * @param flags flags See event flags in listener.h. Default value is EV_DEFAULT
 * @param formatspec Format specifier. Arguments are : 'e' (Entity) 'v' (Vector) 'i' (Integer) 'f' (Float) 's' (String) 'b' (Boolean). Upper case letter = optional
 * @param argument_names Name of each argument separated by spaces.
 * @param documentation The event description
 * @param type For scripts, can be the following value:
 *   EV_NORMAL - Normal command (local.inst Command)
 *   EV_RETURN - Return as a function (local.result = local.inst ReturnCommand)
 *   EV_GETTER - Return as a variable (local.result = local.listener.some_getter)
 *   EV_SETTER - Set as a variable (local.inst.some_setter = "value")
 */
Event(const char *command, int flags, const char * formatspec, cons char *argument_names, const char *documentation, byte type = EV_NORMAL);
```

Let's tweak ExampleObject prototype inside `exampleobject.h` to add a test method.

```cpp
#include "simpleentity.h"

class ExampleObject : public SimpleEntity
{
    CLASS_PROTOTYPE(ExampleObject);

public:
    ExampleObject();
    ~ExampleObject();

    // New test method
    void TestMethod(Event *ev);
};
```

In `exampleobject.cpp`, instantiate a new `Event` and link it in the response list of the ExampleObject class:

```cpp

#include "exampleobject.h"

// Define an event EV_ExampleObject_TestMethod with the name "test_method", accepting 1 argument of type "integer"
Event EV_ExampleObject_TestMethod
(
    "test_method",
    EV_DEFAULT,
    "i",
    "num_to_print",
    "This is a test method.",
    EV_NORMAL
);

CLASS_DECLARATION(SimpleEntity, ExampleObject, "info_exampleobject")
{
    // Link the event with the member method of the class
    {&EV_ExampleObject_TestMethod, &ExampleObject::TestMethod},
    {NULL, NULL}
};

// The class constructor
ExampleObject::ExampleObject()
{
    gi.Printf("Hello, world!\n");
}

// The class destructor
ExampleObject::~ExampleObject()
{
    gi.Printf("I'm being deleted!\n");
}

// The test method
void ExampleObject::TestMethod(Event *ev)
{
    int value;

    value = ev->GetInteger(1);
    gi.Printf("TestMethod called! With value: %d\n", value);
}
```

### Testing the class

Time to test this class.
1. Create a folder `tests` inside the `main` folder of the game.
2. Create a file `test_exampleobject.scr` with the following content:

```cpp
main:
    // Spawn an instance of the ExampleObject class.
    // Should print: "Hello, world!"
    local.ent = spawn ExampleObject

    // Call the custom "test_method".
    // Should print: "TestMethod called! With value: 3"
    local.ent test_method 3

    // delete it
    // Should print: "I'm being deleted!"
    local.ent delete
end
```

3. Start the game
4. In the main menu, open the console and type the following commands to enable cheats:
  - `set cheats 1`
  - `set thereisnomonkey 1`
5. Start a map: `set g_gametype 1;devmap dm/mohdm6`
6. In the console, type: `testthread tests/test_exampleobject.scr`

You should see some stuff being printed into console.
