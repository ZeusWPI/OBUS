# Ontmijnen van een Bom vereist Uitstekende Samenwerking

You see an armed time bomb but don't know how to disarm it. Your friends found a manual to defuse the bomb and you have them on call. This is the premise of the game OBUS, a hardware project by [Zeus WPI](https://zeus.ugent.be). Inspired by the amazing software game [Keep Talking and Nobody Explodes](https://www.keeptalkinggame.com/).

# Get started writing a module

1. [Install](https://www.arduino.cc/en/Guide/#install-the-arduino-desktop-ide) the Arduino IDE.
2. Clone this repository with Git in a permanent location on your drive.
3. Symlink the library: `ln -s /ABSOLUTE/PATH/TO/REPO/lib /PATH/TO/Arduino/libraries/obus`
  (on most Linux distro's, this the Arduino folder is in `$HOME/Arduino`)
4. Follow [these steps](https://github.com/autowp/arduino-mcp2515/#software-usage) to install the CAN library
5. Execute `./src/new_module.sh` to create a new module

# Background
## Game

The game is played by at least two players. The goal is to defuse a bomb,
this is accomplished by defusing every module on that bomb before the bomb
timer runs out.

There are two roles:

- Expert: this person can read the manual about how to defuse the bomb, but cannot see nor interact with the bomb
- Defuser: this person can see and interact with the bomb, but cannot read the manual

These two roles can communicate with each other. To successfully defuse the bomb, they must
communicate efficiently and clearly. If a mistake is made, the team gets a strike.
If they get too many strikes, the bomb explodes, even if the timer hasn't run out yet.

## Implementation details

Now we want to implement this game in hardware. As in the computer version, we want this game to be modular:
it should be easy to 1) make new modules and 2) attach them to a bomb. To do this, we need to settle on
a protocol, both in hardware and in software.

The idea is to have one bomb controller that keeps track of the timer, the amount of strikes and of whether
the bomb has been successfully disarmed, and to have multiple modules that have one or more challenges on them
that need to be solved.

### Hardware

The bomb controller and modules have to be able to communicate with each other. Ideally, we would like a hardware bus where it is easy to add more modules. We would also like to minimise the amount of wires that are needed.

Multiple protocols were considered:

- I2C: very standard, most microcontrollers have this built in; unfortunately, limited in the amount of nodes that can connect to the same network (255), limited in distance between nodes (about 1 meter), and the bus needs 4 wires (GND, VCC and two data lines)
- SPI: needs even more wires, and requires a separate wire per module
- Serial: this is not a bus architecture, so a lot of wires will need to be used

But eventually, CAN was picked. CAN is widely used in vehicles and has several desired properties:
- We only need two wires (CAN uses a differential pair)
- Distance can be up to 500m
- There is built-in packet collision avoidance and per-node priorities
- CAN modules are very cheap
- There are existing Arduino libraries for the CAN module we'll be using

The payload of a CAN packet is 8 bytes long, this should be enough.

## Software

We needed to decide on a protocol to communicate between the bomb controller and the modules (and possibly also between modules?).

Some things we had to consider:

- payload is 8 bytes per packet
- packets can be delayed or not received on every node, so detection of this and retransmission might be needed: if the bomb interactor solves a module and the packet that communicates this with the bomb does not get delivered to the controller, the bomb will still go off, even if all modules have been solved
- we can't send an infinite amount of packets; the higher our bitrate is, the shorter our wires need to be

## Development setup

We use [this](https://github.com/autowp/arduino-mcp2515/) library for CAN communications. See [this](https://github.com/autowp/arduino-mcp2515/#software-usage) header for 3 simple steps on how to use it in the arduino IDE
