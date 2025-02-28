<?php
/*
# Filename: GET-example.php
# Author: suxunbin
# Last Modified: 2019-5-16 15:30
# Description: a simple GET-example of php API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
// "GET" is a default parameter that can be omitted
require "../src/GstoreConnector.php";

$IP = "127.0.0.1";
$Port = 9000;
$httpTye = "ghttp";
$username = "root";
$password = "123456";
$sparql = "select ?x where { ?x <rdf:type> <ub:UndergraduateStudent>. ?y <ub:name> <Course1>. ?x <ub:takesCourse> ?y. ?z <ub:teacherOf> ?y. ?z <ub:name> <FullProfessor1>. ?z <ub:worksFor> ?w. ?w <ub:name> <Department0>. }";
$filename = "res.txt";

// start a gc with given IP, Port, username and password
$gc = new GstoreConnector($IP, $Port, $httpTye, $username, $password);

// check server
$res = $gc->login();
echo $res . PHP_EOL;

// query version
$res = $gc->getCoreVersion();
echo $res . PHP_EOL;

$res = $gc->check();
echo $res . PHP_EOL;

$res = $gc->stat();
echo $res . PHP_EOL;

// build a database with a RDF graph
$res = $gc->build("lubm", "data/lubm/lubm.nt");
echo $res . PHP_EOL;

// load the database 
$res = $gc->load("lubm", "1");
echo $res . PHP_EOL;

// show all users
$res = $gc->showuser();
echo $res. PHP_EOL;

// query
$res = $gc->query("lubm", "json", $sparql);
echo $res. PHP_EOL;

// save the database if you have changed the database
$res = $gc->checkpoint("lubm");
echo $res. PHP_EOL;

// show information of the database
$res = $gc->monitor("lubm");
echo $res. PHP_EOL;

// show all databases
$res = $gc->show();
echo $res. PHP_EOL;

// export the database
$res = $gc->exportDB("lubm", "export/lubm/get");
echo $res . PHP_EOL;

// insert nt
$res = $gc->batchInsert("lubm", "data/bbug/bbug.nt");
echo $res . PHP_EOL;

// remove nt
$res = $gc->batchRemove("lubm", "data/bbug/bbug.nt");
echo $res . PHP_EOL;

// show user
$res = $gc->showuser();
echo $res . PHP_EOL;

// add usr
$res = $gc->usermanage("1", "test", "123456");
echo $res . PHP_EOL;

// set user privile
$res = $gc->userprivilegemanage("1", "test", "1,2,3", "lubm");
echo $res . PHP_EOL;

// clean user privile
$res = $gc->userprivilegemanage("3", "test");
echo $res . PHP_EOL;

// update user password
$res = $gc->userpassword("test", "123456", "111111");
echo $res . PHP_EOL;

// delete user
$res = $gc->usermanage("2", "test");
echo $res . PHP_EOL;

// unload the database
$res = $gc->unload("lubm");
echo $res. PHP_EOL;

// drop the database
$res = $gc->drop("lubm", false); #delete the database directly
//$res = $gc->drop("lubm", true); #leave a backup
echo $res. PHP_EOL;

?>




