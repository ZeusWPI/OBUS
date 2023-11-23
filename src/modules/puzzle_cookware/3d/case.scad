// the inner case is 100 x 150 mm
$fn=50;

module clip() {
	translate([0, -3, -3]) difference() {
		cube([10, 106, 45+3+1.5+0.5]);
		translate([-1, 1.5-0.25, 1.5]) cube([20, 103 + 0.5, 45+1.5+0.5]);
		translate([-1, 1.5+ 6, 20]) cube([20, 90, 50]);
	}
	
}

module case_bottom() {
	difference() {
		translate([-3, -3, -3]) cube([150+6, 100+6, 45+3]);
		union() {
			cube([150, 100, 500]);
			translate([-0.5, -0.5, 44]) cube([151, 101, 50]);
			translate([10, 100/2, 45]) rotate([0, 90, 0]) cylinder(d=20, h=500);
			translate([10, 100/2, 20]) rotate([0, -90, 0]) cylinder(d=10, h=500);
			translate([75-5, -30, -3-1]) cube([10.5, 200, 1.5+1]);
			translate([75-5, -3-1, -10]) cube([10.5, 1.5+1, 200]);
			translate([75-5, 103-1.5, -10]) cube([10.5, 1.5+1, 200]);
		}
	}
	cube([5, 5, 45-5]);
	translate([0, 100-5]) cube([5, 5, 45-5]);
	translate([150-5, 100-5]) cube([5, 5, 45-5]);
	translate([150-5, 0]) cube([5, 5, 45-5]);
}

case_bottom();
// translate([-30, 0, 0]) clip();
