# Wireguide

The original plan of connecting the CAN-modules was to use an ethernet cable:
we would strip the outer layer of the cable where we want to connect a module,
then put on an IDC connector. We would then use one twisted pair for CAN data, and
the other pairs for 12v. We wanted to use an ethernetcable because we have a lot of
spare ethernetcable that's too good to throw away, but too long to be practical.
The CAN specifications also demand a twisted cable.

So in theory, this was a great solution. Unfortunately, theory and practice are
the same in theory, but not in pracice: connecting the IDC connectors proved to be
very time-intensive (first stripping the outer cable, then partially untwisting the strands
and putting them in the correct order) and error-prone (the slightest issue in alignment
	will cause two strands to become electrically connected).

To partially fix this, we designed a small tool that would be able to more easily keep
the strands lined up before pressing the IDC-connector together. This also proved
to be rather hard, so we gave up and just ordered some automotive twisted wire that is
made for CAN traffic.

The term you'll need when trying to buy this twisted wire is `canbus wire`.

`wireguide.scad` is a parametric model of a tool to put the individual wires of an ethernet-cable in the correct position to easily attach an IDC-connector (vampire tap)
