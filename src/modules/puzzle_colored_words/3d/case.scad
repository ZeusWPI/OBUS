$fn=50;
container_wall_thickness=1;
module ccube(size = [1,1,1], center = false)
{
  sz = (len(size) == 3)?size:[size,size,size];
  if (len(center) == 1)
    cube(size, center);
  else
    translate([center[0]?-sz[0]/2:0,center[1]?-sz[1]/2:0,center[2]?-sz[2]/2:0])
    cube(size);
}

cut=250;

height = 115;
width = 80;
depth = 70;

button_bottom_to_top_lip = 7.76+1;



bar_height=35;
bar_width=8;

offset_center_bar=width/2 - bar_width/2 - container_wall_thickness - 7;
case_clearance=container_wall_thickness+0.5;

module bottom() {
	difference() {
		ccube([height, width, depth], center=[1, 1, 0]);
		union() {
			translate([0, 0, container_wall_thickness]) ccube([height-container_wall_thickness*2, width-2*container_wall_thickness, cut], center=[1, 1, 0]);
			hole_offset=14;
			translate([0, 0, depth-13]) rotate([90, 0, 0]) cylinder(d=7, h=cut);
			translate([0, 0, 13]) rotate([90, 0, 0]) cylinder(d=10, h=cut);
			translate([0, 0, 13]) rotate([0, 90, 0]) cylinder(d=10, h=cut);
			translate([0, 0, 13]) rotate([0, -90, 0]) cylinder(d=10, h=cut);
			translate([ height/2-hole_offset,  width/2-hole_offset, -1]) cylinder(d=3.5, h=cut);
			translate([-height/2+hole_offset,  width/2-hole_offset, -1]) cylinder(d=3.5, h=cut);
			translate([ height/2-hole_offset, -width/2+hole_offset, -1]) cylinder(d=3.5, h=cut);
			translate([-height/2+hole_offset, -width/2+hole_offset, -1]) cylinder(d=3.5, h=cut);

			
		}
		
		
	}
	translate([0, offset_center_bar, 0]) ccube([bar_height, bar_width, depth - button_bottom_to_top_lip], center=[1, 1, 0]);
	translate([ height/2-5,  width/2-5, 0]) nutpole();
	translate([-height/2+5,  width/2-5, 0]) nutpole();
	translate([ height/2-5, -width/2+5, 0]) nutpole();
	translate([-height/2+5, -width/2+5, 0]) nutpole();
}



button_hole_diameter=12.5;
top_depth=3;

module nutpole() {
	difference() {
		ccube([10, 10, depth], center=[1, 1, 0]);
		union() {
			translate([0, 0, depth-3-1]) cylinder(d=5.5, h=3, $fn=6);
			translate([0, 0, -1]) cylinder(d=3.75, h=cut);
		}
		
	}
}

module top() {
	color([1, 0, 0])
	difference() {
		ccube([height, width, top_depth], center=[1, 1, 0]);
		union() {
			translate([0, offset_center_bar, -1]) {
				translate([-button_hole_diameter/2-5, 0, 0]) cylinder(d=button_hole_diameter, h=cut);
				translate([button_hole_diameter/2+5, 0, 0]) cylinder(d=button_hole_diameter, h=cut);
			}
			translate([0, -10, -1])  {
				ccube([23.57, 14, cut], center=[1, 1, 0]); // window for screen
				ccube([34, 14, 1+2], center=[1, 1, 0]); //cutout for side electronics
				ccube([21, 24, 1+1.7], center=[1, 1, 0]);
			}
			translate([ height/2-5,  width/2-5, -1]) cylinder(d=3.75, h=cut);
			translate([-height/2+5,  width/2-5, -1]) cylinder(d=3.75, h=cut);
			translate([ height/2-5, -width/2+5, -1]) cylinder(d=3.75, h=cut);
			translate([-height/2+5, -width/2+5, -1]) cylinder(d=3.75, h=cut);
		}
		
	}
}



bottom();
// translate([0, width+5, depth+5]) 
// translate([0, 0, depth]) top();
