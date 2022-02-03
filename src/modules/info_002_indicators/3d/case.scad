front_panel_thickness=2;
cut=200;
container_wall_thickness=1;
$fn=50;

front_panel_height=100;
front_panel_width=50;

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}


module front_panel() {
	difference() {
		union() {
			ccube([front_panel_height, front_panel_width, front_panel_thickness], center=[1, 1, 0]);
			translate([0, 0, -4]) {
				translate([front_panel_height/2 - 5 - container_wall_thickness, -front_panel_width/2 + container_wall_thickness]) cube(5);
				translate([front_panel_height/2 - 5 - container_wall_thickness, front_panel_width/2 - container_wall_thickness - 5]) cube(5);
				translate([-front_panel_height/2 + container_wall_thickness, -front_panel_width/2 + container_wall_thickness]) cube(5);
				translate([-front_panel_height/2 + container_wall_thickness, front_panel_width/2 - container_wall_thickness - 5]) cube(5);
			}
		}
		translate([0, 0, -1]) union() {
			translate([0, front_panel_width/3]) ccube([55, 6, cut], center=[1, 1, 0]);
			translate([0, -front_panel_width/3]) for ( i = [1 : 8] ){
		    translate([10*i - front_panel_height/2 + 5, 0])
		    cylinder(d = 3, h=cut);
			}
		}
	}
}

module back_panel() {
	difference() {
		union() {
			ccube([front_panel_height, front_panel_width, 50+container_wall_thickness], center=[0, 1, 0]);
		}
		union() {
			translate([container_wall_thickness, 0, container_wall_thickness]) ccube([front_panel_height-2*container_wall_thickness, front_panel_width-2*container_wall_thickness, cut], center=[0, 1, 0]);
			translate([front_panel_height/2, 0, -1]) cylinder(d=10, h=cut);
			translate([-1, 0, container_wall_thickness+7]) rotate([0, 90, 0]) cylinder(d=12, h=cut);
			// Holes for M3 bolts
			translate([0, 0, -1]) {
				translate([container_wall_thickness + 5, -front_panel_width/2 + container_wall_thickness + 5]) cylinder(d=3.5, h=cut);
				translate([container_wall_thickness + 5, front_panel_width/2 - container_wall_thickness - 5]) cylinder(d=3.5, h=cut);
				translate([front_panel_height-container_wall_thickness-5, -front_panel_width/2 + container_wall_thickness + 5]) cylinder(d=3.5, h=cut);
				translate([front_panel_height-container_wall_thickness-5, front_panel_width/2 - container_wall_thickness - 5]) cylinder(d=3.5, h=cut);
			}
		}
	}
	translate([container_wall_thickness+13, 0]) ccube([5, 10, container_wall_thickness+10], center=[0, 1, 0]);
}

/* front_panel(); */
/* back_panel(); */

// plate to insert between button and main case
cube([10, 10, 4.8]);
