# Getting started

This is a guide for writing your own puzzle module. We'll first start by
describing an OBUS game, then describe the minimum things you need to implement
to get a working puzzle module. We'll then finish by explaining some nice-to-haves
(like using info modules or game state in a puzzle module).

## OBUS game

### Parts of the game

The goal of OBUS is to defuse a bomb by communicating clearly. There are (at least) two players: the defuser and the expert.
The defuser can see and interact with the bomb, the expert can read the manual that describes how to defuse the bomb.
You win the game by defusing the bomb, you lose if the bomb explodes. The bomb is defused when all puzzle modules are
solved; the bomb explodes if the time runs out or too much mistakes ("strikes") have been made.

The bomb consists of a controller and multiple modules: the controller shows the time left, the amount of mistakes made
while defusing the bomb (strikes) and the amount of strikes left. It's responsible for starting games, enumerating modules,
keeping track of game state, ...
The modules on the other hand are the parts of the bomb that can be interacted with by the defuser. There are several kinds of
modules: puzzle modules, needy modules and info modules.

Puzzle modules are the most common kind of modules. The bomb is defused as soon as all puzzle modules are solved.
Puzzle modules are generally solved by having the defuser observe some kind of state of the game, then having them
communicate that information to the expert. The expert then uses this information and the manual to communicate a set of
instructions to the defuser. If the instructions are correct and the defuser executes them correctly, the module is solved.
If one of the two players makes an error and the defuser executes an incorrect action, the module generates a "strike".

An example of a puzzle module is an RGB led with a red and a green button under it. The defuser looks at the module and
sees that the RGB led is colored blue. They then tell that to the expert, who then looks up this module in the manual.
The manual instructs to press the green button if the color starts with the letter "b" or "o". The expert then asks the
defuser to press the green button. When the defuser does this, the module is solved.

There are also needy modules. These can't be "solved", but they can generate strikes: they require periodic action
from the defuser to prevent them from generating a strike.

An example of a needy module is a buzzer with a red button. If the buzzer goes off, the defuser has to press the button within 5 seconds
or a strike is generated. Needy modules can also have the expert look something up in the manual (if you do this, make sure
to balance the time needed by the defuser against the time between the signal that action is needed and the strike).

Finally, there are also info modules. These don't serve a purpose on their own, they are only useful in combination with
puzzle or needy modules. They provide extra information to those modules. These modules were added to the game
to make it possible to make puzzle/needy modules with less hardware: puzzle modules generally consist of two "parts":
a part that shows information to the defuser that needs to be communicated to the expert and a part that the defuser
can interact with to solve the module. With info modules, this first part can be moved to a dedicated module that can
be shared between multiple puzzle modules.

An example of an info module is an LCD display that displays a serial number. There can then be a puzzle module with
just two buttons on it. When the defuser is solving that module, the expert reads in the manual "press the second button
if the last digit of the serial number is even, otherwise press the first". The expert then asks for the serial number
and after that has the defuser press the correct button, solving the puzzle module.

### Hardware

In our OBUS implementation, every module has its own microcontroller and CAN module. CAN is a hardware protocol
that allows the modules to communicate with the controller (and each other). To write your own module, you don't
have to know how CAN works, this is all abstracted away with the OBUS framework. The only thing you need to do
is to set the type (puzzle/needy/info) of the module and it's ID. The combination type/ID needs to be unique across
the OBUS game, so in order to avoid collisions, you can register your module in TODO LINK MOANA and get an ID.

In addition to a CAN module, every module also has an RGB LED. This LED is used to show both the
state of the module and to indicate if the module has an error.
If the module is solved, the LED is green, if a module generates a strike, the module blinks red. There are also several blinking
patterns for errors. In order to save pins on the microcontroller for implementing the puzzle, only the red and green
parts of the RGB led are connected. You CANNOT use the RGB LED for the puzzle itself: this would be confusing for the player
and when debugging a game.

### A sample game

This is a description of a sample game with only one module: the `puzzle_testmodule_buttons`. When reading this part, it's useful to have the `puzzle_testmodule_buttons.ino` file next to you as well.

We'll start this story from the start: the puzzle module boots up.
It calls the `obus_module::setup` function to register its module type and ID. It then keeps calling the `obus_module::loopPuzzle` in a loop.
It's important that the `loopPuzzle` function is executed very frequently without delays: if this doesn't happen enough, important CAN messages can get dropped.

Then after a while, a button gets pressed on the controller and the controller starts preparing to start the game. It first asks all info modules to broadcast their information. After some time, it then asks all puzzle/needy module to register themselves. The controller then confirms that that module will be active in the next round with an ACK message. After some time, the controller broadcasts that the game has started and starts counting down.

This broadcasts is received on the puzzle module and results in the `callback_game_start` function getting called the next time the
`obus::loopPuzzle` is called. The `callback_game_start` function is responsible for setting up the module for a new game. Here, we
randomly turn the blue LED on or off, and enable the main loop to start checking button presses.

After the `callback_game_start` function returns, the microcontroller stays inside the `loop` function, and keeps executing the `obus_module::loopPuzzle`
frequently. If the correct button is pressed, it calls the `obus_module::solve` function and turns off input checking. Turning off imput checking is
important: that way it's impossible for the puzzle module to generate strikes after it has been solved. If an incorrect button is pressed, it calls the
`obus_module::strike` function. That function will send a strike to the controller.

When `obus_module::solve` is called, the module sends an "I'm solved" CAN packet to the controller. The controller then sees that all modules have been solved,
and broadcasts a "solved" packet to every module. The next time the `obus_module::loopPuzzle` function gets called, the
`callback_game_stop` will be called. This function is responsible for tearing down the puzzle state. This should have the
same effect as just resetting the microcontroller, so if your state is too hard to clean up, you can just reboot the microcontroller.

## What you need to implement for your own puzzle module

- The setup code, initializing your microcontroller and setting the type and id of the module with `obus_module::setup`
- The main loop code. This should call the `loopPuzzle` function frequently so that all CAN packets can be handled.
- A call to the `obus_module::solve` function
- A description for the expert of how to defuse the module in `doc/index.md` of your module folder
- The `callback_game_start` and `callback_game_stop` functions. These can be empty.

Some tips:

- If you are using calls to `delay()`, try to replace them with a timer (a variable that keeps track of when something should happen).
  That way the function that handles CAN messages (`loopPuzzle`) can continue executing, without the microcontroller being stuck in the `delay()` function.
	The `loopPuzzle` function will automatically put itself into an error state if it has been too long since it has been called. This is meant to make
	writing correct code easier: that way you discover your code needs to be rewritten easily instead of having to figure out that and why messages are dropped.
- Every time you play the game, it should be different (otherwise the defuser could just memorize what to do, this would be no fun for the experts).
  To accomplish this, you can use the `random()` function. You don't need to seed it (we recommend against it), because the OBUS framework already
	seeds this for you every game. That way, we can replay games for debugging if needed.

## More advanced puzzle modules

### Receiving game updates

It's possible to use the current state of the game in your module: the amount of
strikes, amount of allowed strikes and time left (in milliseconds) regularly get
broadcast to all modules. That way, you can spice up your puzzle, for example by
making the defuser press a button when the countdown timer has a `1` in it, or by
having the instructions in the manual vary based on the amount of strikes.

See `docs/snippet.timeleft.cpp` for how to use the time remaining on the countdown
timer in your module.

### Using info modules in your puzzle/needy modules

Using info modules is a great way to reduce the amount of hardware components needed in a puzzle:
you can then still have puzzles that change every game, without having to add components that show information
to the defuser. Info modules broadcast their information to all modules in the phase before the game starts.
Every module can listen to these info messages with the `callback_info` callback. This callback will get
the ID of the info module and 7 bytes, as specified by the specific module. The `callback_info` function is
responsible for filtering out the info messages the module is interested in, and saving that info for the upcoming game.

For example, the serial number module has ID 1 and sends in its message 7 random character, chosen randomly from numbers
and the uppercase letters.
