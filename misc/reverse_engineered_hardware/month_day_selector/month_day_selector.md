brown = GND
blue = VCC

yellow = clock
orange = q8 on chip 4
green = read data from buttons

The PCB contains 4 TC4021BP shift register connected in series. To put the current state of the board into the registers, pulse the green wire. Then read bit by bit by first reading from the orange wire, then pulsing the yellow clock wire to advance every bit
