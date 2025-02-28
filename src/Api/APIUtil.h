/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:35:18
 * @LastEditTime: 2023-02-09 13:38:24
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: api util
 * @FilePath: /gstore/GRPC/grpcUtil.h
 */
#pragma once
#include <stdio.h>

#include "../Database/Database.h"
#include "../Database/Txn_manager.h"
#include "../Util/Util.h"
#include "../Util/IPWhiteList.h"
#include "../Util/IPBlackList.h"

using namespace std;
using namespace rapidjson;

#define ROOT_USERNAME "root"
#define SYSTEM_DB_NAME "system"
#define TRANSACTION_LOG_PATH "./logs/transaction.json"
#define TRANSACTION_LOG_TEMP_PATH "./logs/transaction_temp.json"

struct DatabaseInfo
{
private:
    std::string db_name;    //! the name of database
    std::string creator;    //! the creator of database
    std::string build_time; //! the built time of database;
    std::string status;

public:
    pthread_rwlock_t db_lock;

    DatabaseInfo()
    {
        pthread_rwlock_init(&db_lock, NULL);
    }
    DatabaseInfo(string _name, string _creator, string _time)
    {
        db_name = _name;
        creator = _creator;
        build_time = _time;
        pthread_rwlock_init(&db_lock, NULL);
    }
    DatabaseInfo(string _name)
    {
        db_name = _name;
        pthread_rwlock_init(&db_lock, NULL);
    }
    ~DatabaseInfo()
    {
        pthread_rwlock_destroy(&db_lock);
    }
    std::string getName()
    {
        return db_name;
    }
    std::string getCreator()
    {
        return creator;
    }
    void setCreator(string _creator)
    {
        creator = _creator;
    }
    std::string getTime()
    {
        return build_time;
    }
    void setTime(string _time)
    {
        build_time = _time;
    }
    std::string getStatus()
    {
        return status;
    }
    void setStatus(string _status)
    {
        status = _status;
    }
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(rapidjson::kObjectType);
        // doc.SetObject();
        doc.AddMember("database", rapidjson::Value().SetString(db_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("creator", rapidjson::Value().SetString(creator.c_str(), allocator).Move(), allocator);
        doc.AddMember("built_time", rapidjson::Value().SetString(build_time.c_str(), allocator).Move(), allocator);
        doc.AddMember("status", rapidjson::Value().SetString(status.c_str(), allocator).Move(), allocator);
        return doc;
    }
};

struct DBUserInfo
{
private:
    std::string username;
    std::string password;

public:
    std::set<std::string> query_priv;//! the query privilege
    std::set<std::string> update_priv;//! the update privilege
    std::set<std::string> load_priv;//! the load privilege
    std::set<std::string> unload_priv;//! the unload privilege
    std::set<std::string> backup_priv;//! the backup privilege
    std::set<std::string> restore_priv;//! the restore privilege
    std::set<std::string> export_priv;//! the export privilege

    pthread_rwlock_t query_priv_set_lock;
    pthread_rwlock_t update_priv_set_lock;
    pthread_rwlock_t load_priv_set_lock;
    pthread_rwlock_t unload_priv_set_lock;
    pthread_rwlock_t backup_priv_set_lock;
    pthread_rwlock_t restore_priv_set_lock;
    pthread_rwlock_t export_priv_set_lock;
    DBUserInfo() {}
    DBUserInfo(std::string _username, std::string _password)
    {
        username = _username;
        password = _password;
        pthread_rwlock_init(&query_priv_set_lock, NULL);
        pthread_rwlock_init(&update_priv_set_lock, NULL);
        pthread_rwlock_init(&load_priv_set_lock, NULL);
        pthread_rwlock_init(&unload_priv_set_lock, NULL);
        pthread_rwlock_init(&backup_priv_set_lock, NULL);
        pthread_rwlock_init(&restore_priv_set_lock, NULL);
        pthread_rwlock_init(&export_priv_set_lock, NULL);
    }
    ~DBUserInfo() {
        pthread_rwlock_destroy(&query_priv_set_lock);
        pthread_rwlock_destroy(&update_priv_set_lock);
        pthread_rwlock_destroy(&load_priv_set_lock);
        pthread_rwlock_destroy(&unload_priv_set_lock);
        pthread_rwlock_destroy(&backup_priv_set_lock);
        pthread_rwlock_destroy(&restore_priv_set_lock);
        pthread_rwlock_destroy(&export_priv_set_lock);
    }
    std::string getUsernname()
    {
        return username;
    }
    std::string getPassword()
    {
        return password;
    }
    void setPassword(std::string _password)
    {
        password = _password;
    }
        /// <summary>
    /// @brief get the database list which the user can query/
    /// </summary>
    /// <returns></returns>
    std::string getQuery(){
        std::string query_db;
        if(username == ROOT_USERNAME)
        {
            query_db = "all";
            return query_db;
        }
        std::set<std::string>::iterator it = query_priv.begin();
        while(it != query_priv.end())
        {
            query_db = query_db + *it + ",";
            ++it;
        }
        return query_db;
    }
    std::string getUpdate(){
        std::string update_db;
        if(username == ROOT_USERNAME)
        {
            update_db = "all";
            return update_db;
        }
        std::set<std::string>::iterator it = update_priv.begin();
        while(it != update_priv.end())
        {
            update_db = update_db + *it + ",";
            ++it;
        }
        return update_db;
    }
    std::string getLoad(){
        std::string load_db;
        if(username == ROOT_USERNAME)
        {
            load_db = "all";
            return load_db;
        }

        std::set<std::string>::iterator it = load_priv.begin();
        while(it != load_priv.end())
        {
            load_db = load_db + *it + ",";
            ++it;
        }
        return load_db;
    }
    std::string getUnload(){
        std::string unload_db;
        if(username == ROOT_USERNAME)
        {
            unload_db = "all";
            return unload_db;
        }

        std::set<std::string>::iterator it = unload_priv.begin();
        while(it != unload_priv.end())
        {
            unload_db = unload_db + *it + ",";
            ++it;
        }
        return unload_db;
    }
    std::string getRestore(){
        std::string restore_db;
        if(username == ROOT_USERNAME)
        {
            restore_db = "all";
            return restore_db;
        }
        std::set<std::string>::iterator it = restore_priv.begin();
        while(it != restore_priv.end())
        {
            restore_db = restore_db + *it + ",";
            ++it;
        }
        return restore_db;
    }
    std::string getBackup(){
        std::string backup_db;
        if(username == ROOT_USERNAME)
        {
            backup_db = "all";
            return backup_db;
        }
        std::set<std::string>::iterator it = backup_priv.begin();
        while(it != backup_priv.end())
        {
            backup_db = backup_db + *it + ",";
            ++it;
        }
        return backup_db;
    }
    std::string getExport(){
        std::string export_db;
        if(username == ROOT_USERNAME)
        {
            export_db = "all";
            return export_db;
        }
        std::set<std::string>::iterator it = export_priv.begin();
        while(it != export_priv.end())
        {
            export_db = export_db + *it + ",";
            ++it;
        }
        return export_db;
    }
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("username", rapidjson::Value().SetString(this->username.c_str(),allocator).Move(), allocator);
        doc.AddMember("password", rapidjson::Value().SetString(this->password.c_str(),allocator).Move(), allocator);
        doc.AddMember("query_privilege", rapidjson::Value().SetString(this->getQuery().c_str(),allocator).Move(), allocator);
        doc.AddMember("update_privilege", rapidjson::Value().SetString(this->getUpdate().c_str(),allocator).Move(), allocator);
        doc.AddMember("load_privilege", rapidjson::Value().SetString(this->getLoad().c_str(),allocator).Move(), allocator);
        doc.AddMember("unload_privilege", rapidjson::Value().SetString(this->getUnload().c_str(),allocator).Move(), allocator);
        doc.AddMember("backup_privilege", rapidjson::Value().SetString(this->getBackup().c_str(),allocator).Move(), allocator);
        doc.AddMember("restore_privilege", rapidjson::Value().SetString(this->getRestore().c_str(),allocator).Move(), allocator);
        doc.AddMember("export_privilege", rapidjson::Value().SetString(this->getExport().c_str(),allocator).Move(), allocator);
        return doc;
    }
};

struct IpInfo
{
private:
    unsigned int accessNum = 0;
    unsigned int successNum = 0;
    unsigned int errorNum = 0;
    std::string ip;

public:
    IpInfo()
    {
    }
    IpInfo(string name)
    {
        ip = name;
        accessNum = 0;
        successNum = 0;
        errorNum = 0;
    }
    unsigned int getAccessNum()
    {
        return accessNum;
    }
    unsigned int getSuccessNum()
    {
        return successNum;
    }
    unsigned int getErrorNum()
    {
        return errorNum;
    }
    string getName()
    {
        return ip;
    }
    void setAccessNum(unsigned int value)
    {
        accessNum = value;
    }
    void addAccessNum()
    {
        accessNum += 1;
    }
    void setSuccessNum(unsigned int value)
    {
        successNum = value;
    }
    void addSuccessNum() 
    {
        successNum += 1;
    }
    void setErrorNum(unsigned int value)
    {
        errorNum = value;
    }
    void addErrorNum() {
        errorNum += 1;
    }
    void setName(string value)
    {
        ip = value;
    }
};

struct DBQueryLogInfo
{
private:
    std::string queryDateTime;
    std::string remoteIP;
    std::string sparql;
    long ansNum;
    std::string format;
    std::string fileName;
    int statusCode;
    size_t queryTime;
    std::string dbName;
public:
    DBQueryLogInfo () {}
    DBQueryLogInfo (string _queryDateTime, string _remoteIP, string _sparql, long _ansNum, string _format, string _fileName, int _statusCode, size_t _queryTime, string _dbName)
    {
        queryDateTime = _queryDateTime;
        remoteIP = _remoteIP;
        sparql = _sparql;
        ansNum = _ansNum;
        format = _format;
        fileName = _fileName;
        statusCode = _statusCode;
        queryTime = _queryTime;
        dbName = _dbName;
    }
    DBQueryLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("QueryDateTime") && doc["QueryDateTime"].IsString())
            {
                queryDateTime = doc["QueryDateTime"].GetString();
            } 
            if (doc.HasMember("RemoteIP") && doc["RemoteIP"].IsString())
            {
                remoteIP = doc["RemoteIP"].GetString();
            }             
            if (doc.HasMember("Sparql") && doc["Sparql"].IsString())
            {
                sparql = doc["Sparql"].GetString();
            }
            if (doc.HasMember("AnsNum") && doc["AnsNum"].IsInt())
            {
                ansNum = doc["AnsNum"].GetInt();
            }
            if (doc.HasMember("Format") && doc["Format"].IsString())
            {
                format = doc["Format"].GetString();
            }                
            if (doc.HasMember("FileName") && doc["FileName"].IsString())
            {
                fileName = doc["FileName"].GetString();
            }               
            if (doc.HasMember("StatusCode") && doc["StatusCode"].IsInt())
            {
                statusCode = doc["StatusCode"].GetInt();
            }               
            if (doc.HasMember("QueryTime") && doc["QueryTime"].IsUint64())
            {
                queryTime = doc["QueryTime"].GetUint64();
            }               
            if (doc.HasMember("DbName") && doc["DbName"].IsString())
            {
                dbName = doc["DbName"].GetString();
            }
        }
    }
    void setQueryDateTime(string _queryDateTime){queryDateTime = _queryDateTime;}
    void setRemoteIP(string _remoteIP) {remoteIP = _remoteIP;}
    void setSparql(string _sparql) {sparql = _sparql;}
    void setAnsNum(long _ansNum) {ansNum = _ansNum;}
    void setFormat(string _format) {format = _format;}
    void setFileName(string _fileName) {fileName = _fileName;}
    void setStatusCode(int _statusCode) {statusCode = _statusCode;}
    void setQueryTime(int _queryTime) {queryTime = _queryTime;}
    void setDbName(string _dbName) {dbName = _dbName;}

    std::string getQueryDateTime(){return queryDateTime;}
    std::string getRemoteIP() {return remoteIP;}
    std::string getSparql() {return sparql;}
    long getAnsNum() {return ansNum;}
    std::string getFormat() {return format;}
    std::string getFileName() {return fileName;}
    int getStatusCode() {return statusCode;}
    int getQueryTime() {return queryTime;}
    std::string getDbName() {return dbName;}
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("QueryDateTime", rapidjson::Value().SetString(queryDateTime.c_str(), allocator).Move(), allocator);
        doc.AddMember("RemoteIP", rapidjson::Value().SetString(remoteIP.c_str(), allocator).Move(), allocator);
        doc.AddMember("Sparql", rapidjson::Value().SetString(sparql.c_str(), allocator).Move(), allocator);
        doc.AddMember("AnsNum", ansNum, allocator);
        doc.AddMember("Format", rapidjson::Value().SetString(format.c_str(), allocator).Move(), allocator);
        doc.AddMember("FileName", rapidjson::Value().SetString(fileName.c_str(), allocator).Move(), allocator);
        doc.AddMember("StatusCode", statusCode, allocator);
        doc.AddMember("QueryTime", queryTime, allocator);
        doc.AddMember("DbName", rapidjson::Value().SetString(dbName.c_str(), allocator).Move(), allocator);
        return doc;
    }
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value jsonValue = toJSON(allocator);

        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        jsonValue.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

struct DBQueryLogs
{
private:
    int totalSize;
    int totalPage;
    vector<struct DBQueryLogInfo> list;
public:
    DBQueryLogs() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    DBQueryLogs(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addQueryLogInfo(const string & json_str)
    {
        DBQueryLogInfo item(json_str);
        list.push_back(item);
    }
    vector<struct DBQueryLogInfo> getQueryLogInfoList()
    {
        return list;
    }
};

struct DBAccessLogInfo
{
private:
    std::string ip;
    std::string operation;
    int code;
    std::string msg;
    std::string createtime;
    std::string opt_id;
    std::string endtime;
    int state;
    int num;
    int fail_num;
    std::string backupfilepath;
public:
    DBAccessLogInfo() {}
    DBAccessLogInfo(string _ip, string _operation, int _code, string _msg, string _createtime) 
    {
        ip = _ip;
        operation = _operation;
        code = _code;
        msg = _msg;
        createtime = _createtime;
        opt_id = "";
        endtime = "";
        state = 0;
        num = 0;
        fail_num = 0;
        backupfilepath = "";
    }
    DBAccessLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("ip") && doc["ip"].IsString())
                ip = doc["ip"].GetString();
            if (doc.HasMember("operation") && doc["operation"].IsString())
                operation = doc["operation"].GetString();
            if (doc.HasMember("code") && doc["code"].IsInt())
                code = doc["code"].GetInt();
            if (doc.HasMember("msg") && doc["msg"].IsString())
                msg = doc["msg"].GetString();
            if (doc.HasMember("createtime") && doc["createtime"].IsString())
                createtime = doc["createtime"].GetString();
            if (checkOperation())
            {
                if (doc.HasMember("opt_id") && doc["opt_id"].IsString())
                    opt_id = doc["opt_id"].GetString();
                if (doc.HasMember("endtime") && doc["endtime"].IsString())
                    endtime = doc["endtime"].GetString();
                if (doc.HasMember("state") && doc["state"].IsInt())
                    state = doc["state"].GetInt();
                if (doc.HasMember("num") && doc["num"].IsInt())
                    num = doc["num"].GetInt();
                if (doc.HasMember("fail_num") && doc["fail_num"].IsInt())
                    fail_num = doc["fail_num"].GetInt();
                if (doc.HasMember("backupfilepath") && doc["backupfilepath"].IsString())
                    backupfilepath = doc["backupfilepath"].GetString();
            }
        }
    }
    std::string getIP() {return ip;}
    std::string getOperation() {return operation;}
    int getCode() {return code;}
    std::string getMsg() {return msg;}
    std::string getCreateTime() {return createtime;}
    std::string getOptId() {return opt_id;}
    int getState() {return state;}
    int getNum() {return num;}
    int getFailNum() {return fail_num;}
    std::string getBackupfilepath() {return backupfilepath;}
    void setOptId(const std::string& value) {opt_id = value;}
    void setCode(int value) {code = value;}
    void setMsg(const std::string& value) {msg = value;}
    void setEndTime(const std::string& value) {endtime = value;}
    void setState(int value) {state = value;}
    void setNum(int value) {num = value;}
    void setFailNum(int value) {fail_num = value;}
    void setBackupfilepath(const std::string& value) {backupfilepath = value;}
    bool checkOperation()
    {
        if (operation == "build" || operation == "batchInsert" || operation == "batchRemove" || operation == "backup" || operation == "restore")
            return true;
        return false;
    }
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("ip", rapidjson::Value().SetString(ip.c_str(), allocator).Move(), allocator);
        doc.AddMember("operation", rapidjson::Value().SetString(operation.c_str(), allocator).Move(), allocator);
        doc.AddMember("code", code, allocator);
        doc.AddMember("msg", rapidjson::Value().SetString(msg.c_str(), allocator).Move(), allocator);
        doc.AddMember("createtime", rapidjson::Value().SetString(createtime.c_str(), allocator).Move(), allocator);
        if (checkOperation() && !opt_id.empty())
        {
            doc.AddMember("opt_id", rapidjson::Value().SetString(opt_id.c_str(), allocator).Move(), allocator);
            doc.AddMember("endtime", rapidjson::Value().SetString(endtime.c_str(), allocator).Move(), allocator);
            doc.AddMember("state", state, allocator);
            if (operation == "build" || operation == "batchInsert" || operation == "batchRemove")
            {
                doc.AddMember("num", num, allocator);
                doc.AddMember("fail_num", fail_num, allocator);
            }
            else if (operation == "backup")
                doc.AddMember("backupfilepath", rapidjson::Value().SetString(backupfilepath.c_str(), allocator).Move(), allocator);
        }
        return doc;
    }
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value jsonValue = toJSON(allocator);

        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        jsonValue.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

struct DBAccessLogs
{
private:
    int totalSize;
    int totalPage;
    vector<struct DBAccessLogInfo> list;
public:
    DBAccessLogs() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    DBAccessLogs(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addAccessLogInfo(const string & json_str)
    {
        DBAccessLogInfo item(json_str);
        list.push_back(item);
    }
    vector<struct DBAccessLogInfo> getAccessLogInfoList()
    {
        return list;
    }
};

struct TransactionLogInfo
{
private:
    std::string db_name;
    std::string TID;
    std::string user;
    std::string state;
    std::string begin_time;
    std::string end_time;
public:
    TransactionLogInfo() {}
    TransactionLogInfo(string _db_name, string _TID, string _user, string _state, string _begin_time, string _end_time) 
    {
        db_name = _db_name;
        TID = _TID;
        user = _user;
        state = _state;
        begin_time = _begin_time;
        end_time = _end_time;

    }
    TransactionLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("db_name") && doc["db_name"].IsString())
                db_name = doc["db_name"].GetString();
            if (doc.HasMember("TID") && doc["TID"].IsString())
                TID = doc["TID"].GetString();
            if (doc.HasMember("user") && doc["user"].IsString())
                user = doc["user"].GetString();
            if (doc.HasMember("state") && doc["state"].IsString())
                state = doc["state"].GetString();
            if (doc.HasMember("begin_time") && doc["begin_time"].IsString())
                begin_time = doc["begin_time"].GetString();
            if (doc.HasMember("end_time") && doc["end_time"].IsString())
                end_time = doc["end_time"].GetString();
        }
    }
    std::string getDbName() {return db_name;}
    std::string getTID() {return TID;}
    std::string getUser() {return user;}
    std::string getState() {return state;}
    std::string getBeginTime() {return begin_time;}
    std::string getEndTime() {return end_time;}
    void setState(string value) {state = value;}
    void setEndTime(string value) {end_time = value;}
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(rapidjson::kObjectType);
        doc.AddMember("db_name", rapidjson::Value().SetString(db_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("TID", rapidjson::Value().SetString(TID.c_str(), allocator).Move(), allocator);
        doc.AddMember("user", rapidjson::Value().SetString(user.c_str(), allocator).Move(), allocator);
        doc.AddMember("state", rapidjson::Value().SetString(state.c_str(), allocator).Move(), allocator);
        doc.AddMember("begin_time", rapidjson::Value().SetString(begin_time.c_str(), allocator).Move(), allocator);
        doc.AddMember("end_time", rapidjson::Value().SetString(end_time.c_str(), allocator).Move(), allocator);
        return doc;
    }
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value jsonValue = toJSON(allocator);

        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        jsonValue.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

struct TransactionLogs
{
private:
    int totalSize;
    int totalPage;
    vector<struct TransactionLogInfo> list;
public:
    TransactionLogs() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    TransactionLogs(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addTransactionLogInfo(const string & json_str)
    {
        TransactionLogInfo item(json_str);
        list.push_back(item);
    }
    vector<struct TransactionLogInfo> getTransactionLogInfoList()
    {
        return list;
    }
};

class APIUtil
{
private:
    Util util;
    GenerateUidManager uid_mgr_;
    string default_port = "9000";
    int thread_pool_num = 30;
    
    int max_output_size = 10000000;
    size_t max_database_num = 100;
    size_t max_user_num = 1000;
    string query_log_mode = "0";
    string query_log_path = "logs/endpoint/";
    string access_log_mode = "0";
    string access_log_path = "logs/ipaccess/";
    string query_result_path = "logs/query_result/";
    std::string upload_path = "./upload/";
    size_t upload_max_body_size = 104857600; // 100M
    std::vector<std::string> upload_allow_extensions;
    std::vector<std::string> upload_allow_compress_packages;

    std::map<std::string, shared_ptr<Database>> databases;
    std::map<std::string, shared_ptr<DBUserInfo>> users;
    std::map<std::string, shared_ptr<DatabaseInfo>> already_build;
    std::map<std::string, shared_ptr<IpInfo>> ips;
    std::map<std::string, shared_ptr<Txn_manager>> txn_managers;
    std::map<std::string, txn_id_t> running_txn;

    shared_ptr<Database> system_database;
    pthread_rwlock_t users_map_lock;
    pthread_rwlock_t databases_map_lock;
    pthread_rwlock_t already_build_map_lock;
    pthread_rwlock_t txn_m_lock;
    pthread_rwlock_t ips_map_lock;
    pthread_rwlock_t system_db_lock;
    string system_username = "system";
    string system_password;
    string system_password_path;
    string system_port_path;
    int connection_num = 0;
    int blackList = 0;
    int whiteList = 0;
    string ipBlackFile = "ipDeny.config";
    string ipWhiteFile = "";
    std::unique_ptr<IPWhiteList> ipWhiteList;
    std::unique_ptr<IPBlackList> ipBlackList;

    pthread_rwlock_t query_log_lock;
    pthread_rwlock_t access_log_lock;
    pthread_rwlock_t transactionlog_lock;

    bool ip_check(const string& ip);
    bool ip_error_num_check(const string& ip);
public:
    APIUtil();
    ~APIUtil();
    int initialize(const std::string server_type, const std::string port, const std::string db_name, bool load_csr);
    bool unlock_already_build_map();
    bool add_database(const std::string& db_name, shared_ptr<Database> &db);
    bool get_database(const std::string& db_name, shared_ptr<Database> &db);
    bool get_databaseinfo(const std::string& db_name, shared_ptr<DatabaseInfo> &dbInfo);
    bool trywrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);
    bool rdlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);
    bool unlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo);
    bool check_already_load(const std::string& db_name);
    bool get_Txn_ptr(const std::string& db_name, shared_ptr<Txn_manager> &txn_manager);
    bool add_already_build(const std::string& db_name, const std::string& creator, const std::string& build_time);
    // std::string get_already_build(const std::string& db_name);
    void get_already_builds(const std::string& username, vector<shared_ptr<DatabaseInfo>> &array);
    bool check_already_build(const std::string& db_name);
    bool trywrlock_database(const std::string& db_name);
    bool rdlock_database(const std::string& db_name);
    bool unlock_database(const std::string& db_name);
    std::string check_indentity(const std::string& username,const std::string& password,const std::string& encryption);
    std::string check_server_indentity(const std::string& password);
    std::string check_param_value(const string& paramname, const string& value);
    bool check_user_exist(const std::string& username);
    bool check_user_count();
    bool check_db_exist(const std::string& db_name);
    bool check_db_count();
    bool add_privilege(const std::string& username, const vector<string>& types, const std::string& db_name);
    bool del_privilege(const std::string& username, const vector<string>& types, const std::string& db_name);
    bool check_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool init_privilege(const std::string& username, const std::string& db_name);
    bool copy_privilege(const std::string& src_db_name, const std::string& dst_db_name);
    bool update_sys_db(string query);
    bool refresh_sys_db();
    std::string query_sys_db(const std::string& sparql);
    bool build_db_user_privilege(std::string db_name, std::string username);
    bool insert_txn_managers(shared_ptr<Database> &current_database, std::string database);
    bool remove_txn_managers(std::string db_name);
    bool db_checkpoint(string db_name);
    // bool db_checkpoint_all();
    bool delete_from_databases(string db_name);
    bool delete_from_already_build(string db_name);
    //used by drop
    int db_copy(string src_path, string dest_path);
    txn_id_t check_txn_id(string TID);
    string get_txn_begin_time(shared_ptr<Txn_manager> ptr, txn_id_t tid);
    string begin_process(string db_name, int level , string username);
    bool commit_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID);
    bool rollback_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID);
    bool aborted_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID);
    bool user_add(const string& username, const string& password);
    bool user_delete(const string& username);
    bool user_pwd_alert(const string& username, const string& password);
    void get_user_info(vector<shared_ptr<struct DBUserInfo>> &_users);
    int clear_user_privilege(string username);
    string check_access_ip(const string& ip, int check_level);
    void update_access_ip_error_num(const string& ip);
    void reset_access_ip_error_num(const string& ip);
    bool ip_save(string ip_type, vector<string> ipVector);
    vector<string> ip_list(string type);
    string ip_enabled_type();
    bool get_file_lines(vector<string> &lines, string &logFile, int &page_no, int &page_size, int &total_size, int &total_page, pthread_rwlock_t *rw_lock);
    // for access log
    void get_access_log_files(std::vector<std::string> &file_list);
    void get_access_log(const string &date, int &page_no, int &page_size, struct DBAccessLogs *dbAccessLogs);
    void write_access_log(string operation, string remoteIP, int statusCode, string statusMsg, string optId = "");
    void update_access_log(int statusCode, string statusMsg, string opt_id, int state, int num, int failnum, string backupfilepath = "");
    bool getAccessLogByOptId(string opt_id, struct DBAccessLogInfo& log);
    // for query log
    void get_query_log_files(std::vector<std::string> &file_list);
    void get_query_log(const string &date, int &page_no, int &page_size, struct DBQueryLogs *dbQueryLogs);
    void write_query_log(struct DBQueryLogInfo *queryLogInfo);
    // for transaction log
    void init_transactionlog();
	int add_transactionlog(std::string db_name, std::string user, std::string TID,  std::string begin_time, std::string status = "RUNNING",  std::string end_time = "INF");
	// int delete_transactionlog(std::string db_name, std::string TID);
	int update_transactionlog(std::string db_name, std::string status, std::string end_time);
	void get_transactionlog(int &page_no, int &page_size, struct TransactionLogs *dbQueryLogs);
	void abort_transactionlog(long end_time);
    // for data get
    string get_Db_path();
    string get_Db_suffix();
    string get_query_result_path();
    string get_default_port();
    int get_thread_pool_num();
    int get_max_output_size();
    string get_root_username();
    string get_system_username();
    int get_connection_num();
    void increase_connection_num();
    string get_configure_value(const string& key, string default_value);
    int get_configure_value(const string& key, int default_value);
    size_t get_configure_value(const string& key, size_t default_value);
    string get_upload_path();
    size_t get_upload_max_body_size();
    bool check_upload_allow_extensions(const string& suffix);
    bool check_upload_allow_compress_packages(const string& suffix);
    std::string generateUid(){ return uid_mgr_.NextID(); }
    std::string getConvertTimeById(const std::string& uid)const{ return uid_mgr_.getConvertTimeById(uid); }
};