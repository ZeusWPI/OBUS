# Pressure coupler

## Left side (facing front panel)

From top to bottom:

- input on/off -> connected to single white wire, switch is between that and ground
- bridge balance -> connected to blue wire. This is a voltage divider between ground and VCC, read this with `analogRead()`
- calibration -/off/+ -> connected to twisted white/orange wire, connects
  the white wire to ground, nothing to ground or the orange wire to ground
- multichoice switch -> has 16 positions, each yellow wire is a bit of the number of the position
  so for example, if the switch is in the third position (so 2 when counting 0-based, or 0010 in binary),
	wire 2 will be connected to ground, all other wires will be left floating

Red is connected to VCC (5V), black to ground

## Right side

Switches are connected to white wires (2 positions) or white and orange wires (3 positions). Knobs are connected to blue wires.

Switches connect to ground or keep wire floating. Knobs are a voltage divider between ground (black) and VCC (red).
