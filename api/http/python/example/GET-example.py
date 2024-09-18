# -*- coding: UTF-8 -*-
"""
# Filename: GET-example.py
# Author: suxunbin
# Last Modified: 2024/9/18 10:09
# Description: a simple GET-example of python API
"""
import sys
sys.path.append('../src')
import GstoreConnector
import json

# before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
# "GET" is a default parameter that can be omitted
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

# // check
res = gc.check()
print(res)

# // login
res = gc.login()
print(res)

# // query version
res = gc.getCoreVersion()
print(res)

res = gc.check()
print(res)

res = gc.stat()
print(res)

# build a database with a RDF graph
res = gc.build("lubm", "data/lubm/lubm.nt")
print(res)

# load the database 
res = gc.load("lubm", "0")
print(res);

# show all users
res = gc.showuser()
print(res)

# query
res = gc.query("lubm", "json", sparql)
print(res)

# save the database if you have changed the database
res = gc.checkpoint("lubm")
print(res)

# show information of the database
res = gc.monitor("lubm")
print(res)

# show all databases
res = gc.show()
print(res)

# export the database
res = gc.exportDB("lubm", "export/lubm/get")
print(res)

# insert nt
res = gc.batchInsert("lubm", "data/bbug/bbug.nt");
print(res)

try:
    res = json.loads(res)
    if res["opt_id"]:
        res = gc.checkOperationState(res["opt_id"])
        print(res)
except Exception as e:
    print(e)

# user mange
#  show
res = gc.showuser()
print(res)

# add
res = gc.usermanage("1", "test", "123456")
print(res)

# set user privile
res = gc.userprivilegemanage("1", "test", "1,2,3", "lubm")
print(res)

# clean user privile
res = gc.userprivilegemanage("3", "test")
print(res)

# update user password
res = gc.userpassword("test", "123456", "111111")
print(res)

# delete user
res = gc.usermanage("2", "test")
print(res)

# unload the database
res = gc.unload("lubm")
print(res)

# drop the database
res = gc.drop("lubm", False) #delete the database directly
#res = gc.drop("lubm", True) #leave a backup
print(res)

