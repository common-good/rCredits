<?php
$goto = urldecode(@$_GET['goto']);
/// FAILS  if ($goto) header("Location: $goto", TRUE, 301);
/**/ if ($goto) echo "<script>location.href='$goto';</script>"; else echo 'No goto!';

