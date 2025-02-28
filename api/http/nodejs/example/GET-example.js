/*
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2022-09-29 20:56:24
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2022-09-29 20:56:24
 * @FilePath: /gstore/api/http/nodejs/example/GET-example.js
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
# Filename: GET-example.js
# Author: suxunbin
# Last Modified: 2019-5-20 17:26
# Description: a simple GET-example of nodejs API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
// "GET" is a default parameter that can be omitted
const GstoreConnector = require("../src/GstoreConnector.js");

const sparql = "select ?x where{" +
    "?x  <rdf:type> <ub:UndergraduateStudent>. " +
    "?y    <ub:name> <Course1>. " +
    "?x    <ub:takesCourse>  ?y. " +
    "?z   <ub:teacherOf>    ?y. " +
    "?z    <ub:name> <FullProfessor1>. " +
    "?z    <ub:worksFor>    ?w. " +
    "?w    <ub:name>    <Department0>. }";

// start a gc with given IP, Port, username and password
const gc = new GstoreConnector(
    "127.0.0.1",
    9000,
    "ghttp",
    "root",
    "123456"
);

(async function () {
    // check server
    var res = await gc.check();
    console.log(JSON.stringify(res, ","));

    // login server
    res = await gc.login();
    console.log(JSON.stringify(res, ","));

    // query version
    res = await gc.getCoreVersion();
    console.log(JSON.stringify(res, ","));

    res = await gc.stat();
    console.log(JSON.stringify(res, ","));

    // build a database with a RDF graph
    res = await gc.build("lubm", "data/lubm/lubm.nt");
    console.log(JSON.stringify(res, ","));

    // load the database
    res = await gc.load("lubm", "0");
    console.log(JSON.stringify(res, ","));

    // show all users
    res = await gc.showuser();
    console.log(JSON.stringify(res, ","));

    // query
    res = await gc.query("lubm", "json", sparql);
    console.log(JSON.stringify(res, ","));

    // save the database if you have changed the database
    res = await gc.checkpoint("lubm");
    console.log(JSON.stringify(res, ","));

    // show information of the database
    res = await gc.monitor("lubm");
    console.log(JSON.stringify(res, ","));

    // show all databases
    res = await gc.show();
    console.log(JSON.stringify(res, ","));

    // export the database
    res = await gc.exportDB("lubm", "export/lubm/get");
    console.log(JSON.stringify(res, ","));

    // insert nt
    res = await gc.batchInsert("lubm", "data/bbug/bbug.nt");
    console.log(JSON.stringify(res, ","));
    res = await gc.checkOperationState(res.opt_id);
    console.log(JSON.stringify(res, ","));

    // remove nt
    res = await gc.batchRemove("lubm", "data/bbug/bbug.nt");
    console.log(JSON.stringify(res, ","));
    res = await gc.checkOperationState(res.opt_id);
    console.log(JSON.stringify(res, ","));

    // user mange
    // show user
    res = await gc.showuser();
    console.log(JSON.stringify(res, ","));
    // add usr
    res = await gc.usermanage("1", "test", "123456");
    console.log(JSON.stringify(res, ","));
    // set user privile
    res = await gc.userprivilegemanage("1", "test", "1,2,3", "lubm");
    console.log(JSON.stringify(res, ","));
    // clean user privile
    res = await gc.userprivilegemanage("3", "test");
    console.log(JSON.stringify(res, ","));
    // update user password
    res = await gc.userpassword("test", "123456", "111111");
    console.log(JSON.stringify(res, ","));
    // delete user
    res = await gc.usermanage("2", "test");
    console.log(JSON.stringify(res, ","));

    // unload the database
    res = await gc.unload("lubm");
    console.log(JSON.stringify(res, ","));

    // drop the database
    res = await gc.drop("lubm", false); //delete the database directly
    //res = await gc.drop("lubm", true) //leave a backup
    console.log(JSON.stringify(res, ","));
})();
