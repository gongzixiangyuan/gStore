# -*- coding: UTF-8 -*-
"""
# Filename: POST-example.py
# Author: suxunbin
# Last Modified: 2024/9/18 10:09
# Description: a simple POST-example of python API
"""
import sys
sys.path.append('../src')
import GstoreConnector
import json

# before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
IP = "127.0.0.1"
Port = 9000
httpType = "ghttp"
username = "root"
password = "123456"
sparql = "select ?x where \
                 { \
                     ?x  <rdf:type> <ub:UndergraduateStudent>. \
                     ?y    <ub:name> <Course1>. \
                     ?x    <ub:takesCourse>  ?y. \
                     ?z   <ub:teacherOf>    ?y. \
                     ?z    <ub:name> <FullProfessor1>. \
                     ?z    <ub:worksFor>    ?w. \
                     ?w    <ub:name>    <Department0>. \
                 }"
filename = "res.txt"

# start a gc with given IP, Port, username and password
gc =  GstoreConnector.GstoreConnector(IP, Port, username, password, http_type=httpType)

# uild a database with a RDF graph
res = gc.check("POST")
print(res);

# loginserver
res = gc.login("POST")
print(res);

# query version
res = gc.getCoreVersion("POST")
print(res);

res = gc.check("POST")
print(res);

res = gc.stat("POST")
print(res);


# build a database with a RDF graph
res = gc.build("lubm", "data/lubm/lubm.nt", "POST")
print(res)

# load the database 
res = gc.load("lubm", "0", "POST")
print(res)

# show all users
res = gc.showuser("POST")
print(res)

# query
res = gc.query("lubm", "json", sparql, "POST")
print(res)

# save the database if you have changed the database
res = gc.checkpoint("lubm", "POST")
print(res)

# show information of the database
res = gc.monitor("lubm", "POST")
print(res)

# show all databases
res = gc.show("POST")
print(res)

# export the database
res = gc.exportDB("lubm", "export/lubm/post", "POST")
print(res)

# export the database
res = gc.exportDB("lubm", "export/lubm/get")
print(res)

# insert nt
res = gc.batchInsert("lubm", "data/bbug/bbug.nt", "", "", "", "POST")
print(res)
try:
    res = json.loads(res)
    if res["opt_id"]:
        res = gc.checkOperationState(res["opt_id"], "POST")
        print(res)
except Exception as e:
    print(e)

# user mange
#  show
res = gc.showuser("POST")
print(res)

# add
res = gc.usermanage("1", "test", "123456", "POST")
print(res)

# set user privile
res = gc.userprivilegemanage("1", "test", "1,2,3", "lubm", "POST")
print(res)

# clean user privile
res = gc.userprivilegemanage("3", "test", "POST")
print(res)

# update user password
res = gc.userpassword("test", "123456", "111111", "POST")
print(res)

# delete user
res = gc.usermanage("2", "test", "POST")
print(res)

# reason Mange
reason_info = "{\"rulename\":\"1\",\"description\":\"互相关注的人为朋友\",\"isenable\": 1,\"type\": 1,\"logic\": 1,\"conditions\": [{\"patterns\": [{\"subject\": \"?x \",	\"predicate\": \"<关注>\",\"object\":\"?y\"},{\"subject\": \"?y \",\"predicate\": \"<关注>\",\"object\": \"?x\"}],\"filters\": [],\"count_info\": {}}],\"return\": {\"source\": \"?x\",\"target\": \"?k\",\"label\": \"朋友\",\"value\": \"\"}}";
gc.build("friend_reason", "scripts/test/parser_test/parser_d33.ttl", "", "", "POST")
gc.load("friend_reason", "0", "POST")
res = gc.show("POST")
print(res)
# add reason
res = gc.addReason("1", "friend_reason", reason_info)
print(res)

# list reason
res = gc.reasonManage("2", "friend_reason")
print(res)

# compile reason
res = gc.reasonManage("3", "friend_reason", "1")
print(res)

# excute reason
res = gc.reasonManage("4", "friend_reason", "1")
print(res)

# disable reason
res = gc.reasonManage("5", "friend_reason", "1")
print(res)

# show reason
res = gc.reasonManage("6", "friend_reason", "1")
print(res)

# deletereason
res = gc.reasonManage("7", "friend_reason", "1")
print(res)

gc.unload("friend_reason")
gc.drop("friend_reason", False)


# unload the database
res = gc.unload("lubm", "POST")
print(res)

# drop the database
res = gc.drop("lubm", False, "POST") #delete the database directly
#res = gc.drop("lubm", True, "POST") #leave a backup
print(res)




