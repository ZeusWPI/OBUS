$fn=100;
cut=100;



difference() {
	cylinder(d=40, h=12);
	translate([0, 0, -1]) union() {
		translate([-cut/2, 0]) cube(cut);
		translate([0, -10]) cylinder(d=5.1, h=cut);
		translate([0, -10]) cylinder(d=7, h=2);
	}

}
