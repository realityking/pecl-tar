--TEST--
TarArchive::open() member function
--SKIPIF--
<?php 

if(!extension_loaded('tar')) die('skip ');

 ?>
--FILE--
<?php
echo 'OK'; // no test case for this function yet
?>
--EXPECT--
OK