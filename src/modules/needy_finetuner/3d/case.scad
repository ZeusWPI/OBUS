dialWidth = 46;
potWidth = 10.5;
potHeight = 76;

boxLength = 125;
boxWidth = 100;
boxHeight = 30;
lidHeight = 12;
wallSize = 1;

$fn=50;

module ccube(coords, centered) {
    offsets = [
        centered[0] ? -coords[0] / 2 : 0,
        centered[1] ? -coords[1] / 2 : 0,
        centered[2] ? -coords[2] / 2 : 0
    ];
    translate (offsets) {
        cube(coords);
    }
}

xCentered  = [true, false, false];
xyCentered = [true, true,  false];

difference() {
    ccube([boxLength, boxWidth, boxHeight], xyCentered);
    
    // Hollow
    translate ([0, 0, wallSize])
        ccube([boxLength - 2*wallSize, boxWidth - 2*wallSize, boxHeight], xyCentered);
    
    screwHoleOffset = 6;
    // Screw holes
    translate([0, boxWidth / 2 - 8, boxHeight - screwHoleOffset])
        rotate([0, 90, 0])
        cylinder(boxLength + 5, r=2.86/2, center=true);
    translate([0, -boxWidth / 2 + 8, boxHeight - screwHoleOffset])
        rotate([0, 90, 0])
        cylinder(boxLength + 5, r=2.86/2, center=true);
    
    // Cable hole
    translate([-boxLength/2, 0, 5])
        ccube([10, 7, 5], xyCentered);
}

module cornerPiece() {
    translate([0, 0, wallSize]) cube([1, 1, boxHeight - lidHeight]);
}
translate([boxLength/2 - 2*wallSize, boxWidth/2 - 2*wallSize, 0]) cornerPiece();
translate([boxLength/2 - 2*wallSize, -boxWidth/2 + wallSize, 0]) cornerPiece();
translate([-boxLength/2 + wallSize, boxWidth/2 - 2*wallSize, 0]) cornerPiece();
translate([-boxLength/2 + wallSize, -boxWidth/2 + wallSize, 0]) cornerPiece();


translate([boxLength + 10, 0, 0])
//rotate([180, 0, 0])
difference() {
        ccube([
            boxLength - 2*wallSize - 0.5,
            boxWidth  - 2*wallSize - 0.5,
            lidHeight
        ], xyCentered);
    
    // Hollow
    translate ([0, 0, wallSize])
    ccube([
        boxLength - 4*wallSize - 0.5,
        boxWidth  - 4*wallSize - 0.5,
        lidHeight
    ], xyCentered);
    
    translate([0, -35, -1]) {
        // Dial hole
        translate([0, 0, -1])
            ccube([dialWidth, dialWidth, 10], xCentered);
        
        // Pot hole
        translate([0, dialWidth + 4, -1])
            ccube([potHeight, potWidth, 10], xCentered);
        
        // LED hole
        translate([0, dialWidth + 4 + potWidth + 10, -1])
            cylinder(10, 3, center=true);
    }
    
    // Screw holes
    translate([0, boxWidth / 2 - 8, 6])
    rotate([0, 90, 0])
        cylinder(boxLength + 5, r=2.86/2, center=true);
    translate([0, -boxWidth / 2 + 8, 6])
    rotate([0, 90, 0])
        cylinder(boxLength + 5, r=2.86/2, center=true);
}