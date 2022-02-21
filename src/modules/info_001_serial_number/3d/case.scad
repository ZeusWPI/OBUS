$fn=500;
screen_width=72;
screen_height=26;
case_wall=0.9;
inner_size=95;
outer_size=inner_size+2*case_wall;
cut=100;
layer=0.3;
height=50;
m3_hole = 3.4;

module top() {
    difference() {
        cylinder(d=inner_size, h=case_wall);
        cube(size=[screen_width, screen_height, cut], center=true);
        translate([0, inner_size/2, 0]) cylinder(r=5, h=cut);
        translate([0, -inner_size/2, 0]) cylinder(r=5, h=cut);
    }
    translate([0, screen_height, case_wall]) 
    linear_extrude(height=case_wall+2*layer, center=true, convexity=10, twist=0) {
        text("Serial", halign="center", font="Liberation Mono:style=Bold");
    }
}

module bottom() {
    difference() {
        cylinder(d=outer_size, h=height);
        union() {
            translate([0, 0, case_wall]) cylinder(d=inner_size-4, h=cut);
            translate([0, 0, 50-case_wall]) cylinder(d=inner_size+0.5, h=cut);
            translate([0, 0, 10]) rotate([0, 90, 0]) cylinder(d=12, h=cut);
            translate([20, 20, -1]) cylinder(d=m3_hole, h=cut);
            translate([20, -20, -1]) cylinder(d=m3_hole, h=cut);
            translate([-20, 20, -1]) cylinder(d=m3_hole, h=cut);
            translate([-20, -20, -1]) cylinder(d=m3_hole, h=cut);
        }
    }
    intersection() {
        cylinder(d=outer_size, h=height);
        union() {
            translate([0, inner_size/2, 0]) cylinder(r=5-0.25, h=height);
            translate([0, -inner_size/2, 0]) cylinder(r=5-0.25, h=height);
        }
    }
    
}

%translate([0, 0, height-case_wall]) top();
bottom();