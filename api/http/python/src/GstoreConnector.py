import requests
from urllib import parse

defaultServerIP = "127.0.0.1"
defaultServerPort = "9000"


class GstoreConnector:
    def __init__(self, ip, port, username, password, http_type='ghttp'):
        if ip == "localhost":
            self.serverIP = defaultServerIP
            self.serverIP = defaultServerIP
        else:
            self.serverIP = ip
        self.serverPort = port
        self.base_url = "http://" + self.serverIP + ":" + str(self.serverPort) + "/"
        self.http_type = http_type
        if self.http_type == 'grpc':
            self.base_url += 'grpc/api'
        self.auth_params = {
            'username': username,
            'password': password
        }
        self.request = {"GET": self.get, "POST": self.post}


    def get(self, params, stream=False):
        if stream:
            return requests.get(self.base_url, params=params, stream=stream)
        else:
            return requests.get(self.base_url, params=parse.urlencode(params, quote_via=parse.quote), stream=stream).text

    def post(self, params, stream=False):
        if stream:
            return requests.post(self.base_url, json=params, stream=stream)
        else:
            return requests.post(self.base_url, json=params, stream=stream).text

    def save(self, filename, res_iter):
        with open(filename, 'wb') as fd:
            for chunk in res_iter.iter_content(4096):
                fd.write(chunk)
        return

    def check(self, request_type='GET'):
        query_params = {
            'operation': 'check'
        }
        return self.request[request_type](query_params)
    def login(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'login'
        }
        return self.request[request_type](query_params)
    def testConnect(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'testConnnect'
        }
        return self.request[request_type](query_params)
    def getCoreVersion(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'getCoreVersion'
        }
        return self.request[request_type](query_params)

    def ipmanage(self, type, ip_type, ips, request_type='GET'):
        query_params = {
            **self.auth_params,
            'type':type,
            'ip_type':ip_type,
            'ips':ips,
            'operation': 'ipmanage'
        }
        return self.request[request_type](query_params)

    def upload(self, file_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'file_path':file_path,
            'operation': 'upload'
        }
        return self.request[request_type](query_params)

    def download(self, file_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'file_path':file_path,
            'operation': 'download'
        }
        return self.request[request_type](query_params)

    def stat(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'stat'
        }
        return self.request[request_type](query_params)

    def show(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'show'
        }
        return self.request[request_type](query_params)

    def load(self, db_name, csr='0', request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'load',
            'db_name': db_name,
            'csr': csr
        }
        return self.request[request_type](query_params)

    def unload(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'unload',
            'db_name': db_name
        }
        return self.request[request_type](query_params)


    def monitor(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'monitor',
            'db_name': db_name
        }
        return self.request[request_type](query_params)



    def build(self, db_name, db_path, Async ="", callback="", request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'build',
            'db_name': db_name,
            'db_path': db_path,
            'async': Async,
            'callback': callback
        }
        return self.request[request_type](query_params)

    def drop(self, db_name, is_backup, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'drop',
            'db_name': db_name,
            'is_backup': is_backup
        }
        return self.request[request_type](query_params)

    def backup(self, db_name, backup_path, Async = "", callback = "", request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'backup',
            'db_name': db_name,
            'backup_path': backup_path,
            'async': Async,
            'callback': callback
        }
        return self.request[request_type](query_params)

    def backuppath(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'db_name': db_name,
            'operation': 'backuppath'
        }
        return self.request[request_type](query_params)

    def restore(self, db_name, backup_path, Async = "", callback = "", request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'restore',
            'db_name': db_name,
            'backup_path': backup_path,
            'async': Async,
            'callback': callback
        }
        return self.request[request_type](query_params)

    def query(self, db_name, format, sparql, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'query',
            'db_name': db_name,
            'format': format,
            'sparql': sparql
        }
        return self.request[request_type](query_params)

    def fquery(self, db_name, format, sparql, filename, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'fquery',
            'db_name': db_name,
            'format': format,
            'sparql': sparql
        }
        r = self.request[request_type](query_params, stream=True)
        self.save(filename, r)
        return

    def exportDB(self, db_name, db_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'export',
            'db_name': db_name,
            'db_path': db_path
        }
        return self.request[request_type](query_params)

    def batchInsert(self, db_name, file, dir = "", Async = "", callback = "", request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'batchInsert',
            'db_name': db_name,
            'dir': dir,
            'file': file,
            'async': Async,
            'callback': callback
        }
        return self.request[request_type](query_params)

    def batchRemove(self, db_name, file,  Async = "", callback = "", request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'batchRemove',
            'db_name': db_name,
            'file': file,
            'async': Async,
            'callback': callback
        }
        return self.request[request_type](query_params)

    def rename(self, db_name, new_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'rename',
            'db_name': db_name,
            'new_name': new_name
        }
        return self.request[request_type](query_params)

    def checkOperationState(self,opt_id, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'checkOperationState',
            'opt_id': opt_id,
        }
        return self.request[request_type](query_params)

    def begin(self, db_name, isolevel, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'begin',
            'db_name': db_name,
            'isolevel': isolevel
        }
        return self.request[request_type](query_params)

    def tquery(self, db_name, tid, sparql, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'tquery',
            'db_name': db_name,
            'tid': tid,
            'sparql': sparql
        }
        return self.request[request_type](query_params)

    def commit(self, db_name, tid, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'commit',
            'db_name': db_name,
            'tid': tid
        }
        return self.request[request_type](query_params)

    def rollback(self, db_name, tid, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'rollback',
            'db_name': db_name,
            'tid': tid
        }
        return self.request[request_type](query_params)

    def checkpoint(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'checkpoint',
            'db_name': db_name
        }
        return self.request[request_type](query_params)

    def showuser(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'showuser'
        }
        return self.request[request_type](query_params)

    def usermanage(self, type, op_username, op_password='', request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'usermanage',
            'type': type,
            'op_username': op_username,
            'op_password': op_password
        }
        return self.request[request_type](query_params)

    def userprivilegemanage(self, type, op_username, privileges='', db_name='', request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'userprivilegemanage',
            'type': type,
            'op_username': op_username,
            'privileges': privileges,
            'db_name': db_name
        }
        return self.request[request_type](query_params)

    def userpassword(self, re_username, re_password, op_password, request_type='GET'):
        query_params = {
            # **self.auth_params,
            'operation': 'userpassword',
            'username': re_username,
            'password': re_password,
            'op_password': op_password
        }
        return self.request[request_type](query_params)

    def funquery(self, funInfo, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'funquery',
            'funInfo': funInfo

        }
        return self.request[request_type](query_params)

    def funcudb(self, type, funInfo, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'funcudb',
            'type': type,
            'funInfo': funInfo

        }
        return self.request[request_type](query_params)

    def funreview(self, funInfo, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'funreview',
            'funInfo': funInfo

        }
        return self.request[request_type](query_params)

    def txnlog(self, page_no=1, page_size=10, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'txnlog',
            'pageNo': page_no,
            'pageSize': page_size
        }
        return self.request[request_type](query_params)

    def querylogdate(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'querylogdate'

        }
        return self.request[request_type](query_params)

    def querylog(self, date, pageNo, pageSize,request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'querylog',
            'date': date,
            'pageNo': pageNo,
            'pageSize': pageSize

        }
        return self.request[request_type](query_params)

    def accesslogdate(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'accesslogdate'

        }
        return self.request[request_type](query_params)

    def accesslog(self, date, pageNo, pageSize,request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'accesslog',
            'date': date,
            'pageNo': pageNo,
            'pageSize': pageSize

        }
        return self.request[request_type](query_params)

    def addReason(self, type, db_name, ruleinfo,request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'addReason',
            'type': type,
            'db_name': db_name,
            'ruleinfo': ruleinfo

        }
        return self.request[request_type](query_params)

    def reasonManage(self, type, db_name, rulename = "",request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'reasonManage',
            'type': type,
            'db_name': db_name,
            'rulename': rulename

        }
        return self.request[request_type](query_params)


    # def login(self, username, password, request_type='GET'):
    #     query_params = {
    #         'username': username,
    #         'password': password,
    #         'operation': 'login'
    #     }
    #     return self.request[request_type](query_params)
