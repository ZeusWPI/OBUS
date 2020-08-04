$fn=100;
$high=30;
$offset_tool=3;
$offset_width=2.5;
$total_height=3;
$start_height=2;
$total_width=15;
$inner_width=11;

$eps = 0.01;

$idc_depth=4;

difference() {
  cube([20, 10, $total_height]);
  {
    {
      translate([$offset_tool, $offset_width, 0]) {
        for(i=[0:7]) {
          translate([$inner_width/7*i - 0.5, -5, $total_height]) rotate([-90, 0, 0]) {
           scale([1, 2, 1]) cylinder(h=$high, r=0.6);
          }
        }
        translate([($inner_width - $total_width) / 2, 0, $total_height-$start_height]) cube([$total_width, $idc_depth, $high]);
        color([0, 0, 1])  translate([3, $idc_depth - $eps, $total_height-$start_height - $eps]) cube([4, 1.2 + $eps, $high]);
        translate([5, 2, -1]) cylinder(h=$high, r=1);
      }
    }
  }
}


