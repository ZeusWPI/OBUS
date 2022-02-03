led_display_size=32;
led_display_total_size=50;
border_clearance = 10;
front_panel_thickness=2;
button_size=12;
button_clearance=1.5;
cut=50;
button_under_size=button_size+3;
button_under_thickness=2;
container_wall_thickness=1;
$fn=50;

front_panel_height=border_clearance + led_display_size + 3 * button_under_size + 4 + border_clearance/4;
front_panel_width=border_clearance + led_display_size + border_clearance;

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}

module button() {
	color([1, 0.5, 0]) difference() {
		union() {
			translate([0, 0, -front_panel_thickness]) cylinder(d=button_size, h=front_panel_thickness+button_under_thickness, $fn=3);
			translate([0, 0, -front_panel_thickness]) cylinder(d=button_under_size, h=button_under_thickness, $fn=3);
		}
		translate([0, 0, -front_panel_thickness -0.2]) cylinder(d=3.55, h=1);
	}
}

module front_panel() {
	difference() {
		union() {
			ccube([front_panel_height, front_panel_width, front_panel_thickness], center=[0, 1, 0]);
			translate([0, 0, -4]) {
				translate([container_wall_thickness, -front_panel_width/2 + container_wall_thickness]) cube(5);
				translate([container_wall_thickness, front_panel_width/2 - container_wall_thickness - 5]) cube(5);
				translate([front_panel_height-container_wall_thickness-5, -front_panel_width/2 + container_wall_thickness]) cube(5);
				translate([front_panel_height-container_wall_thickness-5, front_panel_width/2 - container_wall_thickness - 5]) cube(5);
			}
		}
		translate([0, 0, -1]) union() {
			translate([border_clearance, 0, 0]) ccube([led_display_size, led_display_size, cut], center=[0, 1, 0]);
			%translate([border_clearance, 0, -12.6+1+front_panel_thickness]) ccube([50, led_display_size, 12.6], center=[0, 1, 0]);
			up_button_trans=border_clearance+led_display_total_size+button_under_size/2+1;
			// Up button
			translate([up_button_trans, 0]) rotate([0, 0, 180]) {
				translate([0, 0, 1]) %button();
				cylinder(d=button_size+button_clearance, h=cut ,$fn=3);
			}
			// Down button
			translate([up_button_trans+(button_size+button_clearance), 0]) rotate([0, 0, 0]) {
				translate([0, 0, 1]) %button();
				cylinder(d=button_size+button_clearance, h=cut ,$fn=3);
			}
			// Right button
			translate([up_button_trans+(button_size+button_clearance)/2, button_size]) rotate([0, 0, 90]) {
				translate([0, 0, 1]) %button();
				cylinder(d=button_size+button_clearance, h=cut ,$fn=3);
			}
			// Left button
			translate([up_button_trans+(button_size+button_clearance)/2, -button_size]) rotate([0, 0, -90]) {
				translate([0, 0, 1]) %button();
				cylinder(d=button_size+button_clearance, h=cut ,$fn=3);
			}
		}
	}
}

button_height=5;

module back_panel() {
	difference() {
		union() {
			ccube([front_panel_height, front_panel_width, 13+container_wall_thickness], center=[0, 1, 0]);
		}
		union() {
			translate([container_wall_thickness, 0, container_wall_thickness]) ccube([front_panel_height-2*container_wall_thickness, front_panel_width-2*container_wall_thickness, cut], center=[0, 1, 0]);
			translate([0, 0, -1]) {
				translate([container_wall_thickness, -front_panel_width/2 + container_wall_thickness]) cube(5);
				translate([container_wall_thickness, front_panel_width/2 - container_wall_thickness - 5]) cube(5);
				translate([front_panel_height-container_wall_thickness-5, -front_panel_width/2 + container_wall_thickness]) cube(5);
				translate([front_panel_height-container_wall_thickness-5, front_panel_width/2 - container_wall_thickness - 5]) cube(5);
			}
			translate([border_clearance+led_display_total_size, 0, -1]) cylinder(d=10, h=cut);
		}
	}
	up_button_trans=border_clearance+led_display_total_size+button_under_size/2+1;
	button_bottom_size=6;
	pillar_height=13+container_wall_thickness - button_height - button_under_thickness;
	// Up button
	translate([up_button_trans, 0]) rotate([0, 0, 180+45]) {
		cylinder(d=button_bottom_size, h=pillar_height, $fn=4);
	}
	// Down button
	translate([up_button_trans+(button_size+button_clearance), 0]) rotate([0, 0, 45]) {
		cylinder(d=button_bottom_size, h=pillar_height, $fn=4);
	}
	// Right button
	translate([up_button_trans+(button_size+button_clearance)/2, button_size]) rotate([0, 0, 90+45]) {
		cylinder(d=button_bottom_size, h=pillar_height, $fn=4);
	}
	// Left button
	translate([up_button_trans+(button_size+button_clearance)/2, -button_size]) rotate([0, 0, -90+45]) {
		cylinder(d=button_bottom_size, h=pillar_height, $fn=4);
	}
}

module under_panel() {
	difference() {
		union() {
			ccube([front_panel_height, front_panel_width, 40+container_wall_thickness], center=[0, 1, 0]);
		}
		union() {
			translate([container_wall_thickness, 0, container_wall_thickness]) ccube([front_panel_height-2*container_wall_thickness, front_panel_width-2*container_wall_thickness, cut], center=[0, 1, 0]);
			translate([front_panel_height/2, 0, -1]) cylinder(d=10, h=cut);
			translate([container_wall_thickness+1, 0, container_wall_thickness+7]) rotate([0, -90, 0]) cylinder(d=12, h=cut);
			// Holes for M3 bolts
			translate([0, 0, -1]) {
				translate([container_wall_thickness + 10, -front_panel_width/2 + container_wall_thickness + 5]) cylinder(d=3.5, h=cut);
				translate([container_wall_thickness + 10, front_panel_width/2 - container_wall_thickness - 5]) cylinder(d=3.5, h=cut);
				translate([front_panel_height-container_wall_thickness-10, -front_panel_width/2 + container_wall_thickness + 5]) cylinder(d=3.5, h=cut);
				translate([front_panel_height-container_wall_thickness-10, front_panel_width/2 - container_wall_thickness - 5]) cylinder(d=3.5, h=cut);
			}
		}
	}
	translate([0, 0, 0]) {
		translate([container_wall_thickness, -front_panel_width/2 + container_wall_thickness]) cube([4.5, 4.5, 45]);
		translate([container_wall_thickness, front_panel_width/2 - container_wall_thickness - 5 + 0.5]) cube([4.5, 4.5, 45]);
		translate([front_panel_height-container_wall_thickness-5 + 0.5, -front_panel_width/2 + container_wall_thickness]) cube([4.5, 4.5, 45]);
		translate([front_panel_height-container_wall_thickness-5 + 0.5, front_panel_width/2 - container_wall_thickness - 5 + 0.5]) cube([4.5, 4.5, 45]);
	}
}


// front_panel();
// button();
/* translate([0, 0, -13-container_wall_thickness]) back_panel(); */
translate([0, 0, -13 - 40 - 2*container_wall_thickness]) under_panel();
