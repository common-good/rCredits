<?php
// Random network graphic-maker

define('RM', 8); // radius of each member
define('RB', 2.5 * RM); // radius of each business
define('RC', RM * 40); // radius of community
define('RP', RC + RB + 1); // radius of picture (one extra pixel on all sides to prevent blurriness when shrunk)
define('N', 120); // number of active members
define('MPB', 60); // number of members per business
define('TX9', 0); // maximum number of connections per member

$img = imagecreatetruecolor(2 * RP, 2 * RP);
imagesavealpha($img, TRUE);
define('WHITE', imagecolorallocate($img, 255, 255, 255));
define('GREEN', imagecolorallocate($img, 0, 128, 0));
define('BLUE', imagecolorallocate($img, 0, 0, 128));
define('RED', imagecolorallocate($img, 255, 0, 0));
$transparent = imagecolorallocatealpha($img, 0, 0, 0, 127);
imagefill($img, 0, 0, $transparent);

circle(RP, RP, RC, WHITE); 

for ($i = 0; $i < N; $i++) {
  do {foreach (ray('x y') as $k) $$k = randFloat(-RC, RC);} while (sq($x) + sq($y) > sq(RC)); // (x,y) within community circle
  list ($xs[$i], $ys[$i], $cos[$i]) = [$x += RP, $y += RP, $co = N - $i < N / (MPB + 1)]; // (randFloat() < 1 / MPB)];
  $color = imagecolorallocate($img, randByte(), randByte(), randByte());
  circle($x, $y, $co ? RB : RM, $color);
  $p = min(($co ? MPB : 1) * randFloat() / (MPB + 1) / 2, TX9 / N); // fraction of previous members to connect to (approximate)
  for ($j = 0; $j < $i; $j++) if (randFloat() < $p) imageline($img, $x, $y, $xs[$j], $ys[$j], 0); // connect
}

header("Content-type: image/png");
imagepng($img);
imagedestroy($img);
    
function randFloat($b0 = 0, $b9 = 1) {return $b0 + ($b9 - $b0) * mt_rand() / mt_getrandmax();}
function randByte() {return rand(0, 255);}
function sq($a) {return $a * $a;}
function circle($cx, $cy, $r, $color, $a1 = 0, $a2 = 360, $r2 = NULL) {
  global $img;
  if (!@$r2) $r2 = $r;
  imagefilledarc($img, $cx, $cy, 2 * $r, 2 * $r2, $a1, $a2, $color, IMG_ARC_PIE);
}

function ray($a) {return explode(' ', $a);}