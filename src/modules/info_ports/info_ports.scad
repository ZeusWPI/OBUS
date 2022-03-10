$fn=100;
/* portheight = 37; */

// distance between mounting holes
mounting_hole_dist=[25, 25, 22, 46];

// size of physical ports
port_sizes=[30, 30, 29 ,53];
ports = len(port_sizes);
distance_between_ports=5;

cutout_radius=100; // not a physical dimension
cut=cutout_radius;
safe_angle=1;

eps=0.0001;

outer_radius=40; // outer radius of inner cylinder (the one with holes)
wall_thickness=2;
gear_height=13;

big_gear=30;
small_gear=14;
modul=1;
gear_distance=(big_gear+small_gear)*modul/2;

clip_thickness=7;
clip_offset=5;
clip_depth=1;


cylinder_height = sum(port_sizes)+distance_between_ports*(ports + 1);

function shl(x,y) = x * pow(2,y);
function shr(x,y) = floor(x * pow(2,-y));
function sum(v, i = 0, r = 0) = i < len(v) ? sum(v, i + 1, r + v[i]) : r;
function sum_up_to(v, idx, i = 0, r = 0) = (i < idx && i < len(v)) ? sum_up_to(v, idx, i + 1, r + v[i]) : r;

module rotating_cylinder() {
	echo("window size = ", 2 * (outer_radius - wall_thickness)*cos(90 + safe_angle - 360 / pow(2, ports + 1)));
	difference() {
		union() {
			/* %translate([0, 0, cylinder_height-0.005]) small_gear_cylinder(); */


			%translate([0, gear_distance, cylinder_height-0.005]) rotate([0, 0, 12.5]) {
				extragear();
			}


			cylinder(r=outer_radius, h=cylinder_height);
		}
		rotate([0, 0, (360 / 2^ports)*2]) union() {
			// hex hole for small gear
			translate([0, 0, cylinder_height - wall_thickness -distance_between_ports - 0.05]) cylinder(r=5, h=100, $fn=6);

			translate([0, 0, -eps]) cylinder(r=outer_radius-wall_thickness, h=cylinder_height - distance_between_ports + eps*2);
			for (rot=[0:pow(2, ports)]) {
				for (bit=[0:(ports - 1)]) {
					if (shr(rot, ports - 1 - bit) % 2 == 1) {
						current_height = sum_up_to(port_sizes, bit) + (bit + 1)*distance_between_ports;
						translate([0, 0, current_height])
						rotate([0,0,(360 / pow(2, ports))*rot])
						linear_extrude(height = port_sizes[bit]) {
					  	polygon(points=[[0,0],[cutout_radius*sin(90 + safe_angle - 360 / pow(2, ports + 1)), cutout_radius*cos(90 + safe_angle - 360 / pow(2, ports + 1))],[cutout_radius*sin(90 - safe_angle + 360 / pow(2, ports + 1)),cutout_radius*cos(90 - safe_angle + 360 / pow(2, ports + 1))]]);
						}
					}
				}
			}
		}
	}
}

// http://sammy76.free.fr/conseils/electronique/arduino/SG90.php
clearance=0.5;
size_outer = 2*(outer_radius+wall_thickness+clearance);


// Print upside-down
module small_gear_cylinder() {
	b_diam=5;
	difference() {
		union() {
			cylinder(h=gear_height, r=7);
			translate([0, 0, b_diam/2]) rotate([0, 90, 0]) cylinder(d=b_diam, h=25);
			cylinder(d=10-clearance, h=gear_height+5, $fn=6);
			mirror(v=[0, 0, 1]) cylinder(r=5-clearance/2, h=wall_thickness+distance_between_ports+2-5);
		}
		mirror(v=[0, 0, 1]) cube([3, 5, 12], center=true);
	}
	
}

ignore_height=4;
ignore_diameter=11.8;
extra_ignore_part=2.9;
distance_to_mount=22.7 - 15.9 - 2.5;

module container() {
	translation_outer = -outer_radius - wall_thickness - clearance;
	module clip_hole() {
		cube([clip_thickness+clearance, 100, 10+1]);
		translate([0, -clip_depth-clearance, 10]) cube([clip_thickness+clearance, 100, 10+clearance]);
	}
	echo("height of box = ", cylinder_height + clearance + gear_height + clearance + ignore_height +  distance_to_mount)
	echo("width of box = ", size_outer);
	difference() {
	 translate([translation_outer, translation_outer, 0]) cube([size_outer, size_outer, cylinder_height + clearance + gear_height + clearance + ignore_height +  distance_to_mount]);
	 union() {
     // hole for inner cylinder
		 translate([0, 0, -eps]) cylinder(r=outer_radius + clearance, h=cylinder_height + gear_height + clearance*2 + eps);

		 // hole for main cylinder shaft
		 translate([0, 0, cylinder_height]) cylinder(r=5+clearance/2, h=50);

		 // hole for gear shaft
		 translate([0, gear_distance, cylinder_height]) {
			 hull() {
				 cylinder(r=(ignore_diameter+clearance)/2, h=50);
				 translate([(ignore_diameter)/2+extra_ignore_part, 0, 0]) cylinder(r=extra_ignore_part+clearance, h=50);
			 }
		 }

		 // rectangular hole for motor

		 translate([-5.9 - clearance/2, gear_distance - 11.8/2 - clearance/2, cylinder_height + clearance + gear_height + clearance + ignore_height]) {
			 cube([22.5 + clearance, 11.8 + clearance, distance_to_mount]);
		 }

		 // indentations on side for clips
		 translate([clip_offset + translation_outer - clearance/2, size_outer/2 - wall_thickness, 0]) {
			 clip_hole();
		 }
		 translate([size_outer/2 -clip_offset - clip_thickness - clearance/2, size_outer/2 - wall_thickness, 0]) {
			 clip_hole();
		 }
		 mirror(v=[0, 1, 0]) {
			 translate([clip_offset + translation_outer - clearance/2, size_outer/2 - wall_thickness, 0]) {
				 clip_hole();
			 }
			 translate([size_outer/2 -clip_offset - clip_thickness - clearance/2, size_outer/2 - wall_thickness, 0]) {
				 clip_hole();
			 }
		 }



		 // holes on side for ports
		 rot = 0;
		 for (bit=[0:(ports - 1)]) {
			 current_height = sum_up_to(port_sizes, bit) + (bit + 1)*distance_between_ports;
			 translate([0, 0, current_height])
			 rotate([0,0,(360 / pow(2, ports))*rot])
			 linear_extrude(height = port_sizes[bit]) {
				 polygon(points=[[0,0],[cutout_radius*sin(90 + safe_angle - 360 / pow(2, ports + 1)), cutout_radius*cos(90 + safe_angle - 360 / pow(2, ports + 1))],[cutout_radius*sin(90 - safe_angle + 360 / pow(2, ports + 1)),cutout_radius*cos(90 - safe_angle + 360 / pow(2, ports + 1))]]);
			 }
		 }
	 	}
	}
}

module prism(l, w, h){
	polyhedron(
	  points=[[0,0,0], [l,0,0], [l,w,0], [0,w,0], [0,w,h], [l,w,h]],
	  faces=[[0,1,2,3],[5,4,3,2],[0,4,5,1],[0,3,4],[5,2,1]]
	);
 }

module clip(thickness=5, clip_depth=1) {
	translate([0, -wall_thickness, 0]) {
		translate([0, -clip_depth, 10]) prism(thickness, wall_thickness + clip_depth, 10);
	  cube([thickness, wall_thickness, 10]);
	}
}

module container_bottom() {
	translate([0, -size_outer/2, 0]) {
		cube([size_outer, size_outer, wall_thickness]);
	}
	translate([0, 0, wall_thickness]) {
		translate([size_outer/2, 0, 0])
		difference() {
			cylinder(r=outer_radius - wall_thickness - clearance, h=distance_between_ports);
			translate([0, 0, -1]) cylinder(r=outer_radius - wall_thickness*2, h=distance_between_ports+5);
		}
		translate([clip_offset, size_outer/2, 0]) clip(thickness=clip_thickness, clip_depth=clip_depth);
		translate([size_outer - clip_thickness - clip_offset , size_outer/2, 0]) clip(thickness=clip_thickness, clip_depth=clip_depth);
		mirror(v=[0,1,0]) {
			translate([clip_offset, size_outer/2, 0]) clip(thickness=clip_thickness, clip_depth=clip_depth);
			translate([size_outer - clip_thickness - clip_offset, size_outer/2, 0]) clip(thickness=clip_thickness, clip_depth=clip_depth);
		}

		holder_depth=1;
		holder_width=25;
		port_depth=7;
		port_width = 12.5;
		hole_clearance = 2.5;
		translate([2*outer_radius - holder_depth - port_depth, -holder_width/2, 0])
		difference() {
			union() {
				cube([holder_depth, holder_width, cylinder_height - distance_between_ports]);
				translate([0, holder_width, 0]) cube([port_depth, 5, cylinder_height-distance_between_ports]);
				translate([0, -5, 0]) cube([port_depth, 5, cylinder_height-distance_between_ports]);
			}

			union() {
				for (bit=[0:(ports - 1)]) {
					current_height = sum_up_to(port_sizes, bit) + (bit + 1)*distance_between_ports;
					dist_to_hole = (port_sizes[bit] - mounting_hole_dist[bit]) / 2;
					translate([-1, holder_width/2, current_height + dist_to_hole]) {
						rotate([0, 90, 0]) cylinder(r=1.5, h=100);
						translate([0, 0, mounting_hole_dist[bit]]) rotate([0, 90, 0]) cylinder(r=1.5, h=100);
						translate([0, -(port_width) / 2, hole_clearance]) cube([100, port_width, mounting_hole_dist[bit] - 2*hole_clearance]);
					}
				}
				translate([-(2*outer_radius - holder_depth - port_depth), -(-holder_width/2), 0]) translate([size_outer/2, 0, 0])
				difference() {
					cylinder(r=100, h=450);
					translate([0, 0, -1]) cylinder(r=outer_radius - wall_thickness - clearance, h=500);
				}
			}
		}
	}
}

module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}


module keycap() {
	translate([0, 0, -1]) difference() {
		scale([12, 7, 15]) sphere(d=1);
		union() {
			ccube([cut, cut, cut], center=[true, true, false]); 
		}
	}
	rotate([0, 180, 0]) ccube([12, 7, 1], center=[1, 1, 0]);
	translate([-5.8/2, 0, 0]) ccube([1, 2.5, 3], center=[1, 1, 0]);
	translate([5.8/2, 0, 0]) ccube([1, 2.5, 3], center=[1, 1, 0]);
}

/* color([1, 1, 0]) */
translation_outer = -outer_radius - wall_thickness - clearance;


module extracase() {
	difference() {
		ccube([size_outer, size_outer, 61.5], center=[1, 1, 0]);
		translate([0, 0, 1.5]) ccube([size_outer-3, size_outer-3, cut], center=[1, 1, 0]);
		translate([0, 0, -1]) cylinder(d=15, h=cut);
	}
	
}


/* rotating_cylinder(); */

/* translate([125, 0, 0]) container(); */

// small_gear_cylinder();


/* translate([125 + translation_outer, 125, -wall_thickness]) container_bottom(); */

// keycap();

extracase();