#include <stdio.h>
#include "workflow/WFFacilities.h"
#include "../GRPC/grpc_server.h"
#include "../GRPC/grpc_status_code.h"
#include "../GRPC/grpc_operation.h"
#include "../Api/APIUtil.h"
#include "../Api/PFNUtil.h"
#include "../Util/CompressFileUtil.h"
#include "../Reason/Reason.h"
#include "../Api/HttpUtil.h"

#define HTTP_TYPE "grpc"

using namespace std;
using namespace grpc;

static WFFacilities::WaitGroup wait_group(1);

APIUtil *apiUtil = nullptr;

PFNUtil *pfnUtil = nullptr;

Latch latch;

std::string _db_home;

std::string _db_suffix;

int initialize(unsigned short port, std::string db_name, bool load_src);

void register_service(GRPCServer &grpcServer);

void shutdown(const GRPCReq *request, GRPCResp *response);
void api(const GRPCReq *request, GRPCResp *response);
void upload_file(const GRPCReq *request, GRPCResp *response);
void download_file(const GRPCReq *request, GRPCResp *response);
// for server
void check_task(const GRPCReq *request, GRPCResp *response);
void login_task(const GRPCReq *request, GRPCResp *response, std::string &ip);
void test_connect_task(const GRPCReq *request, GRPCResp *response);
void core_version_task(const GRPCReq *request, GRPCResp *response);
void ip_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for db
void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void load_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void unload_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void monitor_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void build_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void drop_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void backup_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void backup_path_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void restore_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void export_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void begin_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void tquery_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void commit_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void rollback_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void checkpoint_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void batch_insert_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void batch_remove_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void rename_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for user
void user_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void user_show_task(const GRPCReq *request, GRPCResp *response);
void user_privilege_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void user_password_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for log
void txn_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void query_log_date_task(const GRPCReq *request, GRPCResp *response);
void access_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void access_log_date_task(const GRPCReq *request, GRPCResp *response);
void checkOperationState_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for personalized function
void fun_query_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
void fun_review_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for system stat
void stat_task(const GRPCReq *request, GRPCResp *response, Json &json_data);
// for reason engine
void reason_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data);

std::string to_json_string(const Json& json)
{
	rapidjson::StringBuffer resBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> resWriter(resBuffer);
    json.Accept(resWriter);
	return resBuffer.GetString();
}

std::string jsonParam(const Json &json, const std::string &key)
{
	if (json.HasMember(key.c_str()) && json[key.c_str()].IsString())
	{
		return json[key.c_str()].GetString();
	}
	else
	{
		return "";
	}
}

std::string jsonParam(const Json &json, const std::string &key, const std::string &default_val)
{
	string value;
	if (json.HasMember(key.c_str()) && json[key.c_str()].IsString())
	{
		value = json[key.c_str()].GetString();
	}
	if (value.empty())
	{
		return default_val;
	}
	else
	{
		return value;
	}
}

int jsonParam(const Json &json, const std::string &key, const int &default_val)
{
	if (json.HasMember(key.c_str()))
	{
		if (json[key.c_str()].IsInt())
		{
			return json[key.c_str()].GetInt();
		}
		else if (json[key.c_str()].IsString())
		{
			return atoi(json[key.c_str()].GetString());
		} else {
			return default_val;
		}
	}
	else
	{
		return default_val;
	}
}

bool hasJsonParam(const Json &json, const std::string &key)
{
	return json.HasMember(key.c_str());
}

void sig_handler(int signo)
{
	if (apiUtil)
	{
		delete apiUtil;
		apiUtil = NULL;
	}
	if (pfnUtil)
	{
		delete pfnUtil;
		pfnUtil = NULL;
	}
	SLOG_INFO("grpc server stopped.");
	wait_group.done();
	std::cout.flush();
	_exit(signo);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	apiUtil = new APIUtil();
	pfnUtil = new PFNUtil();
 	_db_home = apiUtil->get_Db_path();
	_db_suffix = apiUtil->get_Db_suffix();
	size_t _len_suffix = _db_suffix.length();
	unsigned short port = 9000;
	string db_name = "";
	string port_str = apiUtil->get_default_port();
	bool loadCSR = 0; // DO NOT load CSR by default
	if (argc < 2)
	{
		SLOG_INFO("Server will use the default port: " + port_str);
		SLOG_INFO("Not load any database!");
		port = atoi(port_str.c_str());
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore RPC Server(grpc)" << endl;
			cout << endl;
			cout << "Usage:\tbin/grpc -p [port] -db [dbname] -c [enable]" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database[option],\t\tthe database name.Default value is empty. Notice that the name can not end with "<< _db_suffix<< endl;
			cout << "\t-p,--port[option],\t\tthe listen port. Default value is 9000." << endl;
			cout << "\t-c,--csr[option],\t\tEnable CSR Struct or not. 0 denote that false, 1 denote that true. Default value is 0." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			cout << "Invalid arguments! Input \"bin/grpc -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		if (db_name.length() > _len_suffix && db_name.substr(db_name.length() - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout<<"The database name can not end with " + _db_suffix + "! Input \"bin/grpc -h\" for help." << endl;
			return -1;
		}
		else if (db_name == "system")
		{
			SLOG_ERROR("The database name can not be system.");
			return -1;
		}
		port_str = Util::getArgValue(argc, argv, "p", "port", port_str);
		port = atoi(port_str.c_str());
		loadCSR = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
	}
	// check grpc thread
	std::string processPath = Util::getExactPath(argv[0]);
	std::string currPid = to_string(getpid());
	if (Util::checkProcessExist(processPath, currPid))
	{
		SLOG_INFO("grpc server already running.");
		return 0;
	}
	// check port
	int max_try = 20;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int bind_return = bind(sock, (struct sockaddr*) &addr,sizeof(addr));
	if (bind_return == -1)
	{
		cout<<"waiting"<<std::flush;
		while (bind_return == -1 && max_try > 0) {
			cout<<"."<<std::flush;
			sleep(3);
			bind_return = bind(sock, (struct sockaddr*) &addr,sizeof(addr));
			max_try --;
		}
		cout<<endl;
		if (bind_return == -1)
		{			
			SLOG_INFO("Server port "<< port_str <<" is already in use.");
			return -1;
		}
	} 
	max_try = 20;
	// relase bind
	int close_status = close(sock);
	while (close_status != 0 && max_try > 0) {
		close_status = close(sock);
		// shutdown_status = shutdown(sock, SHUT_RDWR);
		if (close_status != 0)
		{
			SLOG_DEBUG("close I/O result:" + to_string(close_status));
			sleep(3);
		}
		max_try --;
	}
	sock = -1;
	std::memset(&addr, 0, sizeof(addr));
	while (true)
	{
		pid_t fpid = fork();
		// cout << "fpid:" << fpid << endl;
		if (fpid == 0)
		{
			int ret = initialize(port, db_name, loadCSR);
			std::cout.flush();
			_exit(ret);
		}
		else if (fpid > 0)
		{
			int status;
			waitpid(fpid, &status, 0);
			if (WIFEXITED(status))
			{
				return 0;
			}
			else
			{
				string system_port_path = _db_home + "/system" + _db_suffix + "/port.txt";
				if (Util::file_exist(system_port_path))
				{
					Util::remove_path(system_port_path);
				}
				SLOG_WARN("Stopped abnormally, restarting server...");
				latch.lockExclusive();
				if (apiUtil)
				{
					delete apiUtil;
					apiUtil = NULL;
					apiUtil = new APIUtil();
				}
				latch.unlock();
			}
		}
		else
		{
			SLOG_ERROR("Failed to start server: deamon fork failure.");
			return -1;
		}
	}

	return 0;
}

int initialize(unsigned short port, std::string db_name, bool load_src)
{

	// call apiUtil initialized
	if (apiUtil->initialize("grpc", to_string(port), db_name, load_src) == -1)
	{
		return -1;
	}

	GRPCServer grpcServer;
	// register rest service
	register_service(grpcServer);
	int max_try = 30;
	int start_status = -1;
	do
	{
		start_status = grpcServer.start(port);
		if(start_status == 0)
		{
			SLOG_INFO("grpc server port " + to_string(port));
		}
		else
		{
			SLOG_INFO("grpc server start..." + to_string(start_status));
			sleep(1);
		}
		max_try--;
	} while (start_status == -1 && max_try > 0);
	
	if(start_status != 0)
	{
		SLOG_ERROR("grpc server start failed.");
		delete apiUtil;
		return -1;
	}

	// handle the Ctrl+C signal
	signal(SIGINT, sig_handler);

	wait_group.wait();
	grpcServer.stop();
	SLOG_DEBUG("grpc server stoped.");
	return 0;
}

void register_service(GRPCServer &svr)
{
	std::vector<std::string> methods = {"GET", "POST"};
	svr.ROUTE(
		"/grpc/shutdown", [](const GRPCReq *request, GRPCResp *response)
		{ 
			shutdown(request, response);
		},
		methods);

	svr.ROUTE(
		"/grpc/api", [](const GRPCReq *request, GRPCResp *response)
		{ 
			api(request, response);
		},
		methods);
	svr.ROUTE(
		"/grpc/file/upload", [](const GRPCReq *request, GRPCResp *response)
		{
			upload_file(request, response);
		},
		ReqMethod::POST);

	svr.ROUTE(
		"/grpc/file/upload", [](const GRPCReq *request, GRPCResp *response)
		{
			response->add_header_pair("Access-Control-Allow-Origin", "*");
			response->add_header_pair("Access-Control-Allow-Methods", "POST");
			response->String("ok");
		},
		ReqMethod::OPTIONS);
	svr.ROUTE(
		"/grpc/file/download", [](const GRPCReq *request, GRPCResp *response)
		{
			download_file(request, response);
		},
		ReqMethod::POST);
	svr.ROUTE(
		"/grpc/file/download", [](const GRPCReq *request, GRPCResp *response)
		{
			response->add_header_pair("Access-Control-Allow-Origin", "*");
			response->add_header_pair("Access-Control-Allow-Methods", "POST");
			response->String("ok");
		},
		ReqMethod::OPTIONS);
}

void shutdown(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult = apiUtil->check_access_ip(ip_addr, 0);
	if (ipCheckResult.empty() == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	Json json_data;
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	SLOG_DEBUG("Content-Type:" + ContentType::to_str(request->contentType()));
	if (request->contentType() == APPLICATION_JSON) //for application/json
	{
		Json &json = request->json();
		json_data.CopyFrom(json, allocator);
	}
	else if (request->contentType() == APPLICATION_URLENCODED) //for applicaiton/x-www-form-urlencoded
	{
		std::map<std::string, std::string> &form_data = request->formData();
		std::map<std::string, std::string>::iterator iter = form_data.begin();
		std::string v;
		while (iter != form_data.end())
		{
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				StringUtil::url_decode(v);
			}
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
			iter++;
		}
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::string v;
			while (iter != params.end())
			{
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
				iter++;
			}
		}
	}
	SLOG_INFO("receive [shutdown] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== grpc-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (!request->body().empty())
	{
		ss += "\n  request_body: " + request->body();
	}
	SLOG_DEBUG(ss);
	std::string error;
	std::string username = jsonParam(json_data, "username");
	std::string password = jsonParam(json_data, "password");
	error = apiUtil->check_param_value("username", username);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	error = apiUtil->check_param_value("password", password);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if (username != apiUtil->get_system_username())
	{
		error = "You have no rights to stop the server.";
        response->Error(StatusAuthenticationFailed, error);
		return;
	}
	error = apiUtil->check_server_indentity(password);
	if (error.empty() == false)
	{
		apiUtil->update_access_ip_error_num(ip_addr);
		response->Error(StatusAuthenticationFailed, error);
		return;
	}
	// bool flag = apiUtil->db_checkpoint_all();
	rpc_task->add_callback([](GRPCTask *grpcTask){
		SLOG_DEBUG("Server stopped successfully.");
		// free apiUtil
		delete apiUtil;
		apiUtil = NULL;
		std::cout.flush();
		_exit(1);
	});
	std::string msg = "Server stopped successfully.";
	apiUtil->write_access_log("shutdown", ip_addr, StatusOK, msg);
	response->Success(msg);
}

void upload_file(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult = apiUtil->check_access_ip(ip_addr, 0);
	if (ipCheckResult.empty() == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	SLOG_DEBUG("Content-Type:" + ContentType::to_str(request->contentType()));
	if (request->contentType() != MULTIPART_FORM_DATA) //for application/json
	{
		response->Error(StatusFileReadError, "Content-Type not match");
		return;
	}
	SLOG_INFO("receive [uploadfile] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== grpc-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	ss += "\n  Content-Length: " + request->header("Content-Length");
	SLOG_DEBUG(ss);
	Form &form = request->form();
	if (form.empty())
	{   
		response->Error(StatusFileReadError, "Form data is empty");
		return;
	}
	std::string error;
	if (form.find("username") == form.end() || form.find("password") == form.end())
	{
		error = "username or password is empty";
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	std::string username = form.at("username").second;
	std::string password = form.at("password").second;
	error = apiUtil->check_param_value("username", username);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	error = apiUtil->check_param_value("password", password);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	// filename : filecontent
	std::pair<std::string, std::string>& fileinfo = form.at("file");
	if(fileinfo.first.empty())
	{
		error = "Upload file can not be empty!";
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if(request->has_content_length_header())
	{
		size_t content_length = stoul(request->header("Content-Length"), nullptr, 0);
		size_t max_body_size = apiUtil->get_upload_max_body_size();
		if (content_length > max_body_size)
		{
			SLOG_DEBUG("File size is " + to_string(content_length) + " byte, allowed max size " + to_string(max_body_size) + " byte!");
			error = "Upload file more than max_body_size!";
			response->Error(StatusOperationFailed, error);
			return;
		}
	}
	std::string file_suffix = GRPCUtil::fileSuffix(fileinfo.first);
	if (!apiUtil->check_upload_allow_compress_packages(file_suffix) && apiUtil->check_upload_allow_extensions(file_suffix) == false)
	{
		error = "The type of upload file is not supported!";
		response->Error(StatusOperationFailed, error);
		return;
	}
	
	// remove path info, only return base filename
	std::string file_name = GRPCUtil::fileName(fileinfo.first);
	size_t pos = file_name.size() - file_suffix.size() - 1;
	std::string file_dst = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2() + "." + file_suffix;
	std::string notify_msg = "{\"StatusCode\":0, \"StatusMsg\":\"success\", \"filepath\": \""+file_dst+"\"}";
	response->Save(file_dst, std::move(fileinfo.second), notify_msg);
}

void download_file(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult = apiUtil->check_access_ip(ip_addr, 0);
	if (ipCheckResult.empty() == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	Json json_data;
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	if (request->contentType() == MULTIPART_FORM_DATA) //for multipart/form-data
	{
		Form &form = request->form();
		if (form.empty())
		{   
			response->Error(StatusFileReadError, "Form data is empty");
			return;
		}
		for (Form::iterator iter = form.begin(); iter != form.end(); iter++)
		{
			string v = form.at(iter->first).second;
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
		}
	}
	else if (request->contentType() == APPLICATION_URLENCODED) //for applicaiton/x-www-form-urlencoded
	{
		std::map<std::string, std::string> &form_data = request->formData();
		std::map<std::string, std::string>::iterator iter = form_data.begin();
		std::string v;
		while (iter != form_data.end())
		{
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				StringUtil::url_decode(v);
			}
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
			iter++;
		}
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::string v;
			while (iter != params.end())
			{
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
				iter++;
			}
		}
	}
	SLOG_INFO("receive [downloadfile] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== grpc-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (!request->body().empty())
	{
		ss += "\n  request_body: " + request->body();
	}
	ss += "\n==================================================";
	SLOG_DEBUG(ss);
	std::string error;
	std::string username = jsonParam(json_data, "username");
	std::string password = jsonParam(json_data, "password");
	std::string filepath = jsonParam(json_data, "filepath");
	error = apiUtil->check_param_value("username", username);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	error = apiUtil->check_param_value("password", password);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	error = apiUtil->check_param_value("filepath", filepath);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if (Util::is_file(filepath))
	{
		// the file must in the gstore home dir
		std::string exact_path = Util::getExactPath(filepath.c_str());
		std::string cur_path = Util::get_cur_path();
		SLOG_DEBUG("download file path: " + filepath);
		SLOG_DEBUG("file exact path: " + exact_path);
		if (StringUtil::start_with(exact_path, cur_path) == false)
		{
			error = "Download file must in the gstore home dir";
			response->Error(StatusOperationFailed, error);
			return;
		}
		// std::string compress = jsonParam(json_data, "compress", "0");
		// if (compress == "1") // compress to zip file
		// {
		// 	string filename = GRPCUtil::fileName(exact_path);
		// 	string zipfile = exact_path + ".zip";
		// 	string cmd = "zip " + zipfile + " " + exact_path;
		// 	system(cmd.c_str());
		// 	response->File(zipfile);
		// }
		// else 
		// {
		response->File(exact_path);
		// }
	}
	else
	{
		error = "Download file not exists";
		response->Error(StatusOperationFailed, error);
	}
}

void api(const GRPCReq *request, GRPCResp *response)
{
	// check ip address
	auto *rpc_task = task_of(response);
	std::string ip_addr = rpc_task->peer_addr();
	std::string ipCheckResult = apiUtil->check_access_ip(ip_addr, 1);
	if (ipCheckResult.empty() == false)
	{
		SLOG_DEBUG(ipCheckResult);
		response->Error(StatusIPBlocked, ipCheckResult);
		return;
	}
	Json json_data;
	json_data.SetObject();
	Json::AllocatorType &allocator = json_data.GetAllocator();
	if (request->contentType() == APPLICATION_JSON) //for application/json
	{
		Json &json = request->json();
		json_data.CopyFrom(json, allocator);
	}
	else if (request->contentType() == APPLICATION_URLENCODED) //for applicaiton/x-www-form-urlencoded
	{
		std::map<std::string, std::string> &form_data = request->formData();
		std::map<std::string, std::string>::iterator iter = form_data.begin();
		std::string v;
		while (iter != form_data.end())
		{
			v = iter->second;
			if (UrlEncode::is_url_encode(v))
			{
				StringUtil::url_decode(v);
			}
			json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
			iter++;
		}
	}
	else // for get
	{
		std::map<std::string, std::string> params = request->queryList();
		if (params.empty() == false)
		{
			std::map<std::string, std::string>::iterator iter = params.begin();
			std::string v;
			while (iter != params.end())
			{
				v = iter->second;
				if (UrlEncode::is_url_encode(v))
				{
					StringUtil::url_decode(v);
				}
				json_data.AddMember(rapidjson::Value().SetString(iter->first.c_str(), allocator).Move(), rapidjson::Value().SetString(v.c_str(), allocator).Move(), allocator);
				iter++;
			}
		}
	}
	// add remote_ip param
	json_data.AddMember("remote_ip", StringRef(ip_addr.c_str()), allocator);
	std::string operation = jsonParam(json_data, "operation", "");
	operation_type op_type = OperationType::to_enum(operation);
	if (op_type != OP_LOGIN && op_type != OP_TEST_CONNECT)
	{
		ipCheckResult = apiUtil->check_access_ip(ip_addr, 2);
		if (ipCheckResult.empty() == false)
		{
			SLOG_DEBUG(ipCheckResult);
			response->Error(StatusIPBlocked, ipCheckResult);
			return;
		}
	}
	SLOG_INFO("receive [" << operation << "] request from " << ip_addr);
	std::string ss;
	ss += "\n==================== grpc-api ====================";
	ss += "\n  Content-Type: " + ContentType::to_str(request->contentType());
	ss += "\n  Accept-Encoding: " + request->header("Accept-Encoding");
	ss += "\n  method: " +  string(request->get_method());
	ss += "\n  httpVersion: " +  string(request->get_http_version());
	ss += "\n  requestUri: " +  string(request->get_request_uri());
	if (!request->body().empty())
	{
		ss += "\n  request_body: " + request->body();
	}
	ss += "\n==================================================";
	SLOG_DEBUG(ss);
	// add callback task for access log start
	auto *operation_ptr = new std::string(operation);
	auto *ip_ptr = new std::string(ip_addr);
	rpc_task->add_callback([operation_ptr,ip_ptr](GRPCTask *task) {
		GRPCResp *resp = task->get_resp();
		if (*operation_ptr != "build" && *operation_ptr != "batchInsert" && *operation_ptr != "batchRemove" 
			&& *operation_ptr != "backup" && *operation_ptr != "restore")
			apiUtil->write_access_log(*operation_ptr, *ip_ptr, resp->resp_code, resp->resp_msg);
		else if (resp->resp_code != 0)
			apiUtil->write_access_log(*operation_ptr, *ip_ptr, resp->resp_code, resp->resp_msg);
		delete operation_ptr;
		delete ip_ptr;
	});
	// end
	if (operation.empty())
	{
		SLOG_DEBUG("unkown operation");
		response->Error(StatusOperationUndefined);
		return;
	}
	if (op_type == OP_CHECK)
	{
		check_task(request, response);
		return;
	}

	if (json_data.HasMember("username") == false || json_data.HasMember("password") == false)
	{
		response->Error(StatusParamIsIllegal, "username or password is empty");
		return;
	}
	std::string username = jsonParam(json_data, "username");
	std::string password = jsonParam(json_data, "password");
	std::string encryption = jsonParam(json_data, "encryption", "");
	std::string db_name =jsonParam(json_data, "db_name");
	// check username and password
	std::string checkidentityresult = apiUtil->check_indentity(username, password, encryption);
	if (checkidentityresult.empty() == false)
	{
		apiUtil->update_access_ip_error_num(ip_addr);
		response->Error(StatusAuthenticationFailed, checkidentityresult);
		return;
	}
	// check privilege
	if (apiUtil->check_privilege(username, operation, db_name) == 0)
	{
		std::string msg = "You have no " + operation + " privilege, operation failed";
		response->Error(StatusOperationConditionsAreNotSatisfied, msg);
		return;
	}
	// api operation
	switch (op_type)
	{
	case OP_LOGIN:
		login_task(request, response, ip_addr);
		break;
	case OP_TEST_CONNECT:
		test_connect_task(request, response);
		break;
	case OP_CORE_VERSION:
		core_version_task(request, response);
		break;
	case OP_IP_MANAGE:
		ip_manage_task(request, response, json_data);
		break;
	case OP_SHOW:
		show_task(request, response, json_data);
		break;
	case OP_LOAD:
		load_task(request, response, json_data);
		break;
	case OP_UNLOAD:
		unload_task(request, response, json_data);
		break;
	case OP_MONITOR:
		monitor_task(request, response, json_data);
		break;
	case OP_BUILD:
		build_task(request, response, json_data);
		break;
	case OP_DROP:
		drop_task(request, response, json_data);
		break;
	case OP_BACKUP:
		backup_task(request, response, json_data);
		break;
	case OP_BACKUP_PATH:
		backup_path_task(request, response, json_data);
		break;
	case OP_RESTORE:
		restore_task(request, response, json_data);
		break;
	case OP_QUERY:
		query_task(request, response, json_data);
		break;
	case OP_EXPORT:
		export_task(request, response, json_data);
		break;
	case OP_BEGIN:
		begin_task(request, response, json_data);
		break;
	case OP_TQUERY:
		tquery_task(request, response, json_data);
		break;
	case OP_COMMIT:
		commit_task(request, response, json_data);
		break;
	case OP_ROLLBACK:
		rollback_task(request, response, json_data);
		break;
	case OP_CHECKPOINT:
		checkpoint_task(request, response, json_data);
		break;
	case OP_BATCH_INSERT:
		batch_insert_task(request, response, json_data);
		break;
	case OP_BATCH_REMOVE:
		batch_remove_task(request, response, json_data);
		break;
	case OP_RENAME:
		rename_task(request, response, json_data);
		break;
	case OP_USER_MANAGE:
		user_manage_task(request, response, json_data);
		break;
	case OP_USER_SHOW:
		user_show_task(request, response);
		break;
	case OP_USER_PRIVILEGE:
		user_privilege_task(request, response, json_data);
		break;
	case OP_USER_PASSWORD:
		user_password_task(request, response, json_data);
		break;
	case OP_TXN_LOG:
		txn_log_task(request, response, json_data);
		break;
	case OP_QUERY_LOG:
		query_log_task(request, response, json_data);
		break;
	case OP_QUERY_LOG_DATE:
		query_log_date_task(request, response);
		break;
	case OP_ACCESS_LOG:
		access_log_task(request, response, json_data);
		break;
	case OP_ACCESS_LOG_DATE:
		access_log_date_task(request, response);
		break;
	case OP_FUN_QUERY:
		fun_query_task(request, response, json_data);
		break;
	case OP_FUN_CUDB:
		fun_cudb_task(request, response, json_data);
		break;
	case OP_FUN_REVIEW:
		fun_review_task(request, response, json_data);
		break;
	case OP_STAT:
		stat_task(request, response, json_data);
		break;
	case OP_CHECKOPERATIONSTATE:
		checkOperationState_task(request, response, json_data);
		break;
	case OP_REASON_MANAGE:
	    reason_manage_task(request,response,json_data);
		break;
	default:
		SLOG_ERROR("Unkown operation, request body:\n" + request->body());
		response->Error(StatusOperationUndefined);
		break;
	}
}

/**
 * check the grpc server activity
 * 
 * @param request 
 * @param response 
 */
void check_task(const GRPCReq *request, GRPCResp *response)
{
	std::string success = "the grpc server is running...";
	response->Success(success);
}

/**
 * login grpc server
 * 
 * @param request 
 * @param response 
 */
void login_task(const GRPCReq *request, GRPCResp *response, std::string &ip)
{
	try
	{
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "login successfully", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		string product_name = Util::getConfigureValue("product_name");
		std::transform(product_name.begin(), product_name.end(), product_name.begin(), ::tolower);
		if (product_name != "gstore") {
			licensetype = "";
		}
		resp_data.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		string cur_path = Util::get_cur_path();
		resp_data.AddMember("RootPath", StringRef(cur_path.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		apiUtil->reset_access_ip_error_num(ip);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "login fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * login grpc server
 * 
 * @param request 
 * @param response
 */
void test_connect_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		string product_name = Util::getConfigureValue("product_name");
		std::transform(product_name.begin(), product_name.end(), product_name.begin(), ::tolower);
		if (product_name != "gstore") {
			licensetype = "";
		}
		resp_data.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Test connect fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
/**
 * get core version
 * 
 * @param request 
 * @param response
 */
void core_version_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resp_data.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		resp_data.AddMember("type", HTTP_TYPE, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get core version fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * IP manage
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {type: "1 for query, 2 for save", ip_type: "1 for black list, 2 for white list", ips:"ip list, split with ','"}
 */
void ip_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string error;
		string type = json_data["type"].GetString();
		if (type == "1")
		{
			string IPtype = apiUtil->ip_enabled_type();
			if (IPtype == "3")
			{
				error = "please configure ip_deny_path or ip_allow_path in the conf.ini file first.";
				response->Error(StatusOperationFailed, error);
				return;
			}
			vector<string> ip_list = apiUtil->ip_list(IPtype);
			size_t count = ip_list.size();
			Json resp_data;
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			rapidjson::Value responseBody(kObjectType);
			rapidjson::Value ips(kArrayType);
			for (size_t i = 0; i < count; i++)
			{
				ips.PushBack(rapidjson::Value().SetString(ip_list[i].c_str(), allocator).Move(), allocator);
			}

			responseBody.AddMember("ip_type", rapidjson::Value().SetString(IPtype.c_str(), allocator).Move(), allocator);
			responseBody.AddMember("ips", ips, allocator);

			resp_data.SetObject();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "success", allocator);
			resp_data.AddMember("ResponseBody", responseBody, allocator);
			response->Json(resp_data);
		}
		else if (type == "2")
		{
			std::string ips = json_data["ips"].GetString();
			std::string ip_type = json_data["ip_type"].GetString();
			if (ips.empty())
			{
				std::string error = "the ips can't be empty";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			vector<string> ipVector;
			Util::split(ips, ",", ipVector);
			if (ip_type == "1" || ip_type == "2")
			{
				bool rt = apiUtil->ip_save(ip_type, ipVector);
				if (rt)
				{
					response->Success("success");
				}
				else
				{
					if (ip_type == "1")
					{
						error = "ip_deny_path is not configured, please configure it in the conf.ini file first.";
						response->Error(StatusOperationFailed, error);
					}
					else
					{
						error = "ip_allow_path is not configured, please configure it in the conf.ini file first.";
						response->Error(StatusOperationFailed, error);
					}
				}
			}
			else
			{
				error = "ip_type is invalid, please look up the api document.";
				response->Error(StatusParamIsIllegal, error);
			}
		}
		else
		{
			error = "type is invalid, please look up the api document.";
			response->Error(StatusParamIsIllegal, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "IP manger fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * show the all database list (except system database)
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {username: "the user who is the owner of database or has rights to access the database"}
 */
void show_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string username = json_data["username"].GetString();

		vector<shared_ptr<DatabaseInfo>> array;
		apiUtil->get_already_builds(username, array);

		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		size_t count = array.size();
		
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<DatabaseInfo> dbInfo = array[i];
			array_data.PushBack(dbInfo->toJSON(allocator).Move(), allocator);
		}
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get the database list successfully!", allocator);
		resp_data.AddMember("ResponseBody", array_data, allocator);

		// set response status and message
		response->Json(resp_data);
	}
	catch (std::exception &e)
	{
		std::string error = "Show failed:" + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * load the database to memory.
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database", csr: "load csr resource flag, default '0'"}
 */
void load_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (!apiUtil->check_already_build(db_name))
		{
			error = "The database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}

		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			if (!apiUtil->trywrlock_database(db_name))
			{
				error = "Unable to load due to loss of lock.";
				response->Error(StatusOperationFailed, error);
				return;
			}
			shared_ptr<Database> current_database = make_shared<Database>(db_name);
			SLOG_DEBUG("begin loading...");
			bool load_csr = false;
			if (jsonParam(json_data, "csr", "0") == "1")
			{
				load_csr = true;
			}
			// TODO progress notification
			bool rt  = current_database->load(load_csr);
			SLOG_DEBUG("end loading.");
			if (rt)
			{
				apiUtil->add_database(db_name, current_database);
				// todo insert txn
				if (apiUtil->insert_txn_managers(current_database, db_name) == false)
				{
					SLOG_WARN("when load insert_txn_managers fail.");
				}
				std::string csr_str = "0";
				if (current_database->csr != NULL)
				{
					csr_str = "1";
				}
				Json resp_data;
				resp_data.SetObject();
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", "Database loaded successfully.", allocator);
				resp_data.AddMember("csr", StringRef(csr_str.c_str()), allocator);
				apiUtil->unlock_database(db_name);
				response->Json(resp_data);
			}
			else
			{
				error = "load failed: unknow error.";
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed, error);
			}
		}
		else
		{
			std::string csr_str = "0";
			if (current_database->csr != NULL)
			{
				csr_str = "1";
			}
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "The database already load yet.", allocator);
			resp_data.AddMember("csr", StringRef(csr_str.c_str()), allocator);
			response->Json(resp_data);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "load fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
/**
 * unload a database from memory
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database"}
 */
void unload_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusOperationFailed, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "the database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
			return;
		}
		else
		{
			apiUtil->db_checkpoint(db_name);
			apiUtil->delete_from_databases(db_name);
			apiUtil->unlock_databaseinfo(db_info);

			response->Success("Database unloaded.");
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Unload fail" + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get the database monitor info 
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database"}
 */
void monitor_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string disk = jsonParam(json_data, "disk");
		// check the param value is legal or not.
		string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			response->Error(StatusOperationFailed, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<DatabaseInfo> database_info;
		apiUtil->get_databaseinfo(db_name, database_info);
		if (apiUtil->rdlock_databaseinfo(database_info) == false)
		{
			string error = "Unable to monitor due to loss of lock";
			response->Error(StatusLossOfLock, error);
			return;
		}
		std::string creator = database_info->getCreator();
		std::string time = database_info->getTime();
		apiUtil->unlock_databaseinfo(database_info);
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL) {
			current_database = make_shared<Database>(db_name);
			current_database->loadDBInfoFile();
			current_database->loadStatisticsInfoFile();
		}
		unordered_map<string, unsigned long long> umap = current_database->getStatisticsInfo();
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		rapidjson::Value subjectList(kArrayType);
		for (auto &kv : umap)
		{
			rapidjson::Value item(kObjectType);
			item.AddMember("name", rapidjson::Value().SetString(Util::clear_angle_brackets(kv.first).c_str(), allocator), allocator);
			item.AddMember("value", rapidjson::Value().SetUint64(kv.second), allocator);
			subjectList.PushBack(item.Move(), allocator);
		}
		// /use JSON format to send message
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("database", StringRef(db_name.c_str()), allocator);
		resp_data.AddMember("creator", StringRef(creator.c_str()), allocator);
		resp_data.AddMember("builtTime", StringRef(time.c_str()), allocator);
		char tripleNumString[128];
		sprintf(tripleNumString, "%lld", current_database->getTripleNum());
		resp_data.AddMember("tripleNum", StringRef(tripleNumString), allocator);
		resp_data.AddMember("entityNum", current_database->getEntityNum(), allocator);
		resp_data.AddMember("literalNum", current_database->getLiteralNum(), allocator);
		resp_data.AddMember("subjectNum", current_database->getSubNum(), allocator);
		resp_data.AddMember("predicateNum", current_database->getPreNum(), allocator);
		resp_data.AddMember("connectionNum", apiUtil->get_connection_num(), allocator);
		unsigned diskUsed = 0;
		if (disk != "0") 
		{
			string db_path = _db_home + "/" + db_name + _db_suffix;
			string real_path = Util::getExactPath(db_path.c_str());
			if (!real_path.empty()) {
				long long unsigned count_size_byte = Util::count_dir_size(real_path.c_str());
				// byte to MB
				diskUsed = count_size_byte>>20;
			}
		}
		resp_data.AddMember("diskUsed", diskUsed, allocator);
		resp_data.AddMember("subjectList", subjectList, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Monitor fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * build the database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database that would build", db_path: "the data file path"}
 */
void build_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_path = jsonParam(json_data, "db_path");
		std::string result = "";
		// result = apiUtil->check_param_value("db_path", db_path);
		// if (result.empty() == false)
		// {
		// 	response->Error(StatusParamIsIllegal, result);
		// 	return;
		// }
		if (!db_path.empty()) 
		{
			if (db_path == Util::system_path)
			{
				result = "You have no rights to access system files.";
				response->Error(StatusCheckPrivilegeFailed, result);
				return;
			}
			if (Util::file_exist(db_path) == false)
			{
				result = "RDF file not exist.";
				response->Error(StatusParamIsIllegal, result);
				return;
			}
		}
		std::string db_name = jsonParam(json_data, "db_name");
		result = apiUtil->check_param_value("db_name", db_name);
		if (result.empty() == false)
		{
			response->Error(StatusParamIsIllegal, result);
			return;
		}

		// check if database named [db_name] is already built
		if (apiUtil->check_db_exist(db_name))
		{
			result = "database already built.";
			response->Error(StatusOperationConditionsAreNotSatisfied, result);
			return;
		}
		// check databse number
		if (apiUtil->check_db_count() == false)
		{
			result = "The total number of databases more than max_databse_num.";
			response->Error(StatusOperationConditionsAreNotSatisfied, result);
			return;
		}
		std::vector<std::string> zip_files;
		std::string unz_dir_path;
		std::string file_suffix = GRPCUtil::fileSuffix(db_path);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(db_path,[](std::string filename)->bool
				{
					if (apiUtil->check_upload_allow_extensions(GRPCUtil::fileSuffix(filename)) == false)
						return false;
					return true;
				});
			if (code != CompressUtil::UnZipOK)
			{
				string error = "uncompress is failed error.";
				response->Error(code, error);
				return;
			}
			std::string file_name = GRPCUtil::fileName(db_path);
			size_t pos = file_name.size() - file_suffix.size() - 1;
            unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			mkdir(unz_dir_path.c_str(), 0775);
			CompressUtil::UnCompressZip upfile(db_path, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				result = "uncompress is failed error.";
				response->Error(code, result);
				return;
			}
			upfile.getFileList(zip_files, "");
		}
		std::string opt_id = apiUtil->generateUid();
		string remote_ip = task_of(response)->peer_addr();
		string msg = "Operation Success.";
		string operation = "build";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		string username = jsonParam(json_data, "username");
		string async = jsonParam(json_data, "async");
		string callback = jsonParam(json_data, "callback");
		auto build_helper = [db_name,username,unz_dir_path,is_zip,zip_files,db_path,operation,opt_id,async,callback]
				(GRPCResp *response)
				{
					string _db_path = _db_home + "/" + db_name + _db_suffix;
					string database = db_name;
					SLOG_DEBUG("Import dataset to build database...");
					SLOG_DEBUG("DB_store: " + database + "\tRDF_data: " + db_path);
					string result;
					shared_ptr<Database> current_database = make_shared<Database>(database);
					// TODO progress notification
					bool flag = true;
					if (!db_path.empty())
						flag = current_database->build(db_path);
					else
						flag = current_database->BuildEmptyDB();
					int success_num = current_database->getTripleNum();
					current_database.reset();
					if (flag)
					{
						// if zip file then excuse batchInsert
						if (is_zip && zip_files.size() > 0)
						{
							current_database = make_shared<Database>(db_name);
							bool rt  = current_database->load(false);
							if (!rt)
							{
								result = "Import RDF file to database failed: load error.";
								Util::remove_path(_db_path);
								if (!unz_dir_path.empty())
								{
									Util::remove_path(unz_dir_path);
								}
								apiUtil->update_access_log(1005, result, opt_id, -1, 0, 0);
								if (async != "true")
									response->Error(StatusOperationFailed, result);
								current_database.reset();
								return;
							}
							for (std::string rdf_zip : zip_files)
							{
								current_database->batch_insert(rdf_zip, false, nullptr);
							}
							current_database->save();
							success_num = current_database->getTripleNum();
							current_database.reset();
						}
					}
					// init database info and privilege
					if (apiUtil->build_db_user_privilege(db_name, username) 
						&& apiUtil->init_privilege(username, db_name))
					{
						ofstream f;
						f.open(_db_path + "/success.txt");
						f.close();
						// add backup.log
						Util::add_backuplog(db_name);
						// build response result
						result = "Import RDF file to database done.";
						string error_log = _db_path + "/parse_error.log";
						size_t parse_error_num = Util::count_lines(error_log);
						// exclude Info line
						if (parse_error_num > 0)
							parse_error_num = parse_error_num - 1;
						if (zip_files.size() > 0)
							parse_error_num = parse_error_num - zip_files.size();
						if (parse_error_num > 0)
						{
							SLOG_ERROR("RDF parse error num " + to_string(parse_error_num));
							SLOG_ERROR("See log file for details " + error_log);
						}
						// remove unzip dir
						if (!unz_dir_path.empty())
						{
							Util::remove_path(unz_dir_path);
						}
						Util::add_backuplog(db_name);
						apiUtil->update_access_log(0, result, opt_id, 1, success_num, parse_error_num);
						if (async != "true")
						{
							rapidjson::Document resp_data;
							resp_data.SetObject();
							rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
							resp_data.AddMember("StatusCode", 0, allocator);
							resp_data.AddMember("StatusMsg", StringRef(result.c_str()), allocator);
							resp_data.AddMember("failed_num", parse_error_num, allocator);
							resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
							response->Json(resp_data);
						}
						if (!callback.empty())
						{
							string postdata;
							string res;
							postdata += "{\"StatusCode\":\"0\",";
							postdata += "\"StatusMsg\":\"" + result + "\",";
							postdata += "\"failed_num\":\"" + std::to_string(parse_error_num) + "\",";
							postdata += "\"opt_id\":\"" + opt_id + "\"}";
							HttpUtil::Post(callback, postdata, res);
						}
					}
					else
					{
						result = "Import RDF file to database failed.";
						rmdir(_db_path.c_str());
						Util::remove_path(_db_path);
						if (!unz_dir_path.empty())
						{
							Util::remove_path(unz_dir_path);
						}
						apiUtil->update_access_log(1005, result, opt_id, -1, 0, 0);
						if (async != "true")
							response->Json(result);
					}
				};
		if (async == "true")
		{
			rapidjson::Document resp_data;
			resp_data.SetObject();
			rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			response->Json(resp_data);
			thread t(build_helper, nullptr);
			t.detach();
		}
		else
		{
			build_helper(response);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Build fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * drop a database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database that would drop", "is_backup": "'true' for logic delete, 'false' for physically delete"}
 */
void drop_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string is_backup = jsonParam(json_data, "is_backup", "true");
		std::string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			if (apiUtil->check_already_load(db_name))
			{
				bool rt = apiUtil->remove_txn_managers(db_name);
				if (!rt)
				{
					apiUtil->unlock_databaseinfo(db_info);
					SLOG_DEBUG("remove " + db_name + " from the txn managers fail.");
					error = "the operation can not been excuted due to can not release txn manager.";
					response->Error(StatusOperationFailed, error);
					return;
				}
				SLOG_DEBUG("remove " + db_name + " from the txn managers.");
				//@ the database has loaded, unload it firstly
				apiUtil->delete_from_databases(db_name);
				SLOG_DEBUG("remove " + db_name + " from loaded database list");
			}
			apiUtil->unlock_databaseinfo(db_info);
			//@ delete the database info from the system database
			bool rt = apiUtil->delete_from_already_build(db_name);
			if (!rt)
			{
				SLOG_DEBUG("remove " + db_name + " from the already build database list fail.");
				error = "the operation can not been excuted due to loss of lock.";
				response->Error(StatusLossOfLock, error);
				return;
			}
			SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
			string db_path = _db_home + "/" + db_name + _db_suffix;
			if (is_backup == "false")
			{
				Util::remove_path(db_path);
				SLOG_DEBUG("remove_path"+db_path);
			}
			else
			{
				std::string cmd = "mv " + db_path + " " + _db_home + "/" + db_name + ".bak";
				SLOG_DEBUG(cmd);
				system(cmd.c_str());
			}
			Util::delete_backuplog(db_name);
			string success = "Database " + db_name + " dropped.";
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Drop fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * backup a database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the name of database that would backup", "backup_path": "the backup path, default path ./backups"}
 */
void backup_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string backup_path = jsonParam(json_data, "backup_path");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		std::string compress_zip = jsonParam(json_data, "backup_zip");
		bool backup_zip = false;
		if (compress_zip == "true")
			backup_zip = true;

		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Database> current_db;
		apiUtil->get_database(db_name, current_db);
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
			return;
		}
		// begin backup database
		string path = backup_path;
		string default_backup_path = Util::backup_path;
		if (path.empty())
		{
			path = default_backup_path;
			SLOG_DEBUG("backup_path is empty, set to default path: " + path);
		}
		if (path == "." || Util::getExactPath(path.c_str()) == Util::getExactPath(_db_home.c_str()))
		{
			error = "Failed to backup the database. Backup path can not be root or \"" + _db_home + "\" .";
			apiUtil->unlock_databaseinfo(db_info);
			response->Error(StatusParamIsIllegal, error);
			return;
		}

		std::string opt_id = apiUtil->generateUid();
		string remote_ip = task_of(response)->peer_addr();
		string msg = "Operation Success.";
		string operation = "backup";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		std::string async = jsonParam(json_data, "async");
		std::string callback = jsonParam(json_data, "callback");
		string temp_path = path;
		auto backup_helper = [db_name,operation,opt_id,&current_db,&db_info,temp_path,default_backup_path,backup_zip,async,callback]
			(GRPCResp *response)
			{
				string path = temp_path;
				std::string error = "";
				bool flag = false;
				if (current_db)
				{
					flag = current_db->backup();
				}
				else
				{
					Database _db(db_name);
					flag = _db.backup();
				}
				if(flag == false)
				{
					error = "Failed to backup the database.";
					apiUtil->unlock_databaseinfo(db_info);
					apiUtil->update_access_log(1005, error, opt_id, -1, 0, 0);
					if (async != "true")
						response->Error(StatusOperationFailed, error);
					return;
				}
				else
				{
					string timestamp = Util::get_timestamp();
					string new_folder =  db_name + _db_suffix + "_" + timestamp;
					string sys_cmd, _path, backup_store_path;
					Util::string_suffix(path, '/');
					_path = path + new_folder;
					backup_store_path = default_backup_path + "/" + db_name + _db_suffix;
					if (backup_zip)
					{
						_path = _path + ".zip";
						CompressUtil::CompressZip compress_dir;
						if (!compress_dir.compressDirExportZip(backup_store_path, _path))
						{
							error = "Failed to backup compress the database.";
							apiUtil->unlock_databaseinfo(db_info);
							apiUtil->update_access_log(1005, error, opt_id, -1, 0, 0);
							if (async != "true")
								response->Error(StatusOperationFailed, error);
							return;
						}
						Util::remove_path(backup_store_path);
					}
					else
					{
						sys_cmd = "mv " + backup_store_path + " " + _path;
						system(sys_cmd.c_str());
					}
					vector<string> files;
					Util::dir_files(path, "", files);
					int max_backups = atoi(Util::getConfigureValue("max_backups").c_str());
					std::string db_file_suffix = db_name + _db_suffix;
					int db_file_suffix_size = db_file_suffix.size();
					std::string db_file_min;
					unsigned int backup_num = 0;
					for (auto& file_name : files)
					{
						if (file_name.substr(0, db_file_suffix_size) == db_file_suffix)
						{
							if (db_file_min.empty() || file_name < db_file_min)
								db_file_min = file_name;
							backup_num++;
						}
					}
					if (backup_num > max_backups)
					{
						Util::remove_path(path + db_file_min);
					}

					SLOG_DEBUG("database backup done: " + db_name);
					string success = "Database backup successfully.";
					// current_db = NULL;
					apiUtil->unlock_databaseinfo(db_info);
					apiUtil->update_access_log(0, success, opt_id, 1, 0, 0, _path);

					if (async != "true")
					{
						Document resp_data;
						resp_data.SetObject();
						Document::AllocatorType &allocator = resp_data.GetAllocator();
						resp_data.AddMember("StatusCode", 0, allocator);
						resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resp_data.AddMember("backupfilepath", StringRef(_path.c_str()), allocator);
						resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						response->Json(resp_data);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"backupfilepath\":\"" + _path + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				}
			};
		if (async == "true")
		{
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			response->Json(resp_data);
			thread t(backup_helper, nullptr);
			t.detach();
		}
		else
		{
			backup_helper(response);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Backup fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query backup path
 * 
 * @param request 
 * @param response 
 * @param json_data 
 */
void backup_path_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::vector<std::string> file_list;
		string backup_path = Util::backup_path;
		Util::dir_files(backup_path, db_name, file_list);
		Document resp_data;
		Document pathsDoc;
		resp_data.SetObject();
		pathsDoc.SetArray();
		Document::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < file_list.size(); i++)
		{
			pathsDoc.PushBack(Value().SetString((backup_path + file_list[i]).c_str(), allocator), allocator);
		}
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("paths", pathsDoc, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Query backup path fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * restore the database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", backup_path: "the backup path", username: "the operation username"}
 */
void restore_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string backup_path = jsonParam(json_data, "backup_path");
		std::string username = jsonParam(json_data, "username");

		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		bool is_zip = false;
		std::string path = backup_path;
		if (GRPCUtil::fileSuffix(backup_path) == "zip")
		{
			is_zip = true;
			if (Util::file_exist(path) == false)
			{
				error = "Backup path not exist, restore failed.";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			path = path.substr(0, path.length() - 4);
		}
		else
		{
			if (path[path.length() - 1] == '/')
			{
				path = path.substr(0, path.length() - 1);
			}
			SLOG_DEBUG("backup path:" + path);
			if (Util::dir_exist(path) == false)
			{
				error = "Backup path not exist, restore failed.";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
		}
		// check load status: need unload if already load
		if (apiUtil->check_already_load(db_name))
		{
			string error = "Database alreay load, need unload it first.";
			response->Error(StatusOperationFailed, error);
			return;
		}
		string database = db_name;
		SLOG_DEBUG("restore database:" + database);
		if (apiUtil->check_already_build(db_name) == false)
		{
			error = "Database not built yet, rebuild now.";
			string time = Util::get_backup_time(path, db_name);
			if (time.size() == 0)
			{
				error = "Backup path does not match database name, restore failed";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			if (apiUtil->init_privilege(username, db_name) == 0)
			{
				error = "init privilege failed.";
				response->Error(StatusAddPrivilegeFaied, error);
				return;
			}
			if (apiUtil->build_db_user_privilege(db_name, username))
			{
				Util::add_backuplog(db_name);
			}
			else
			{
				error = "Database not built yet. Rebuild failed.";
				response->Error(StatusOperationFailed, error);
				return;
			}
		}
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "Unable to restore due to loss of lock";
			response->Error(StatusLossOfLock, error);
			return;
		}

		std::string opt_id = apiUtil->generateUid();
		string remote_ip = task_of(response)->peer_addr();
		string msg = "Operation Success.";
		string operation = "restore";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		std::string async = jsonParam(json_data, "async");
		std::string callback = jsonParam(json_data, "callback");
		auto remove_helper = [db_name,operation,opt_id,is_zip,backup_path,&db_info,path,async,callback]
			(GRPCResp *response)
			{
				// TODO why need lock the database_map?
				// apiUtil->trywrlock_database_map();
				int ret = 0;
				if (is_zip)
				{
					if (Util::dir_exist(path))
						Util::remove_path(path);
					mkdir(path.c_str(), 0775);
					CompressUtil::UnCompressZip unzip(backup_path, path);
					if (unzip.unCompress() != CompressUtil::UnZipOK)
					{
						Util::remove_path(path);
						string error = "backup compress fail";
						apiUtil->update_access_log(1003, error, opt_id, -1, 0, 0);
						if (async != "true")
							response->Error(StatusParamIsIllegal, error);
						return;
					}
					ret = apiUtil->db_copy(path, _db_home);
					Util::remove_path(path);
				}
				else
				{
					apiUtil->db_copy(path, _db_home);
				}
				// apiUtil->unlock_database_map();
				// copy failed
				if (ret == 1)
				{
					string error = "Failed to restore the database. Backup path error";
					apiUtil->unlock_databaseinfo(db_info);
					apiUtil->update_access_log(1005, error, opt_id, -1, 0, 0);
					if (async != "true")
						response->Error(StatusOperationFailed, error);
				}
				else
				{
					// remove old folder
					string db_path = _db_home + "/" + db_name + _db_suffix;
					Util::remove_path(db_path);
					// mv backup folder to database folder
					string folder_name = Util::get_folder_name(path, db_name);
					string sys_cmd = "mv " + _db_home + "/" + folder_name + " " + db_path;
					std::system(sys_cmd.c_str());
					apiUtil->unlock_databaseinfo(db_info);
					
					std::string success = "Database " + db_name + " restore successfully.";
					apiUtil->update_access_log(0, success, opt_id, 1, 0, 0);
					if (async != "true")
						response->Success(success);
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				}
			};
		if (async == "true")
		{
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			response->Json(resp_data);
			thread t(remove_helper, nullptr);
			t.detach();
		}
		else
		{
			remove_helper(response);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Restore fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query the database.
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", format: "json/html/file", sparql: "the sparql"}
 */
void query_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string min_memory = Util::getConfigureValue("min_memory").c_str();
		int memoryLeft = Util::memoryLeft();
		if (memoryLeft < atoi(min_memory.c_str()))
		{
			std::string error = "memory not enough, available:" + std::to_string(memoryLeft) + "GB, need minimum:" + min_memory + "GB";
			response->Error(StatusOperationFailed, error);
			return;
		}
		std::string db_name = jsonParam(json_data, "db_name");
		std::string sparql = jsonParam(json_data, "sparql");
		std::string format = jsonParam(json_data, "format", "json");
		std::string username = jsonParam(json_data, "username");
		// check db_name paramter
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		// check sparql paramter
		error = apiUtil->check_param_value("sparql", sparql);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		string thread_id = Util::getThreadID();
		shared_ptr<Database> current_database;
		bool update_flag_bool = true;
		if (apiUtil->check_privilege(username, "update", db_name) == 0)
		{
			update_flag_bool = false;
		}
		try
		{
			// check database exist
			if (apiUtil->check_db_exist(db_name) == false)
			{
				error = "Database not build yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			// check database load status
			apiUtil->get_database(db_name, current_database);
			if (current_database == NULL)
			{
				error = "Database not load yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			bool lock_rt = apiUtil->rdlock_database(db_name);
			if (lock_rt)
			{
				SLOG_DEBUG("get current database read lock success: " + db_name);
			}
			else
			{
				error = "get current database read lock fail.";
				response->Error(StatusLossOfLock, error);
				return;
			}
		}
		catch (const std::exception &e)
		{
			error = "Query fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
			return;
		}

		FILE *output = NULL;

		ResultSet rs;
		int ret_val;
		int query_time = Util::get_cur_time();

		// set query_start_time
		std::string query_start_time;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int s = tv.tv_usec / 1000;
		int y = tv.tv_usec % 1000;
		query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";
		try
		{
			SLOG_DEBUG("begin query...\n" + sparql);
			rs.setUsername(username);
			ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
			query_time = Util::get_cur_time() - query_time;
		}
		catch (const std::exception &e)
		{
			error = "Query fail: " + string(e.what());
			apiUtil->unlock_database(db_name);
			response->Error(StatusOperationFailed, error);
			return;
		}

		bool ret = false, update = false;
		if (ret_val < -1) // non-update query
		{
			ret = (ret_val == -100);
		}
		else // update query, -1 for error, non-negative for num of triples updated
		{
			update = true;
		}

		string filename = thread_id + "_" + Util::getTimeString2() + "_" + Util::int2string(Util::getRandNum()) + ".txt";
		string localname = apiUtil->get_query_result_path() + filename;
		if (ret)
		{
			// SLOG_DEBUG(thread_id + ":search query returned successfully.");

			// record each query operation, including the sparql and the answer number
			// accurate down to microseconds
			// filter the IP from the test server
			std::string remote_ip = jsonParam(json_data, "remote_ip");
			long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
			long rs_outputlimit = (long)rs.output_limit;
			if (rs_outputlimit != -1)
				rs_ansNum = min(rs_ansNum, rs_outputlimit);
			// if (remote_ip != TEST_IP)
			// {
			int status_code = 0;
			string file_name = "";
			if (format.find("file") != string::npos)
			{
				file_name = string(filename.c_str());
			}
			// add callback task for query log start
			struct DBQueryLogInfo* query_log_info = new DBQueryLogInfo(query_start_time, remote_ip, sparql, rs_ansNum, format, file_name, status_code, query_time, db_name);
			auto * query_log_task = task_of(response);
			query_log_task->add_callback([query_log_info](GRPCTask *) {
				apiUtil->write_query_log(query_log_info);
				delete query_log_info;
			});
			// end
			// }

			// to void someone downloading all the data file by sparql query on purpose and to protect the data
			// if the ansNum too large, for example, larger than 100000, we limit the return ans.
			if (rs_ansNum > apiUtil->get_max_output_size())
			{
				if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
				{
					rs_outputlimit = apiUtil->get_max_output_size();
				}
			}
			string query_time_s = Util::int2string(query_time);

			ofstream outfile;
			string ans = "";
			string success = rs.to_JSON();
			rs.release();
			// TODO: if result is stored in Stream instead of memory?  (if out of memory to use to_str)
			// BETTER: divide and transfer, in multiple times, getNext()
			if (format == "json")
			{
				Json resp_data;
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				/* code */
				resp_data.Parse(success.c_str());
				if (resp_data.HasParseError())
				{
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					SLOG_ERROR("result parse error: ErrorCode=" + to_string(resp_data.GetParseError()) 
							+ ", ErrorPosition=" + to_string(resp_data.GetErrorOffset()) + ", ResultFile=" + localname2);
					error = "Query fail: the result parse error.";
					response->Error(StatusOperationFailed, error);
				}
				else
				{
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", "success", allocator);
					resp_data.AddMember("AnsNum", rs_ansNum, allocator);
					resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
					resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
					resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);

					response->set_header_pair("Cache-Control", "no-cache");
					response->set_header_pair("Pragma", "no-cache");
					response->set_header_pair("Expires", "0");
					if (request->hasHeader("Accept-Encoding")) {
						std::string accept_encoding = request->header("Accept-Encoding");
						if (accept_encoding.find("gzip") != std::string::npos)
						{
							response->headers["Content-Encoding"] = "gzip";
						}
					}
					SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
					response->Json(resp_data);
				}
			}
			else if (format == "file")
			{
				outfile.open(localname);
				outfile << success;
				outfile.close();

				Json resp_data;
				resp_data.SetObject();
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", "success", allocator);
				resp_data.AddMember("AnsNum", rs_ansNum, allocator);
				resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
				resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
				resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
				resp_data.AddMember("FileName", StringRef(filename.c_str()), allocator);

				response->set_header_pair("Cache-Control", "no-cache");
				response->set_header_pair("Pragma", "no-cache");
				response->set_header_pair("Expires", "0");

				SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
				response->Json(resp_data);
			}
			else if (format == "json+file" || format == "file+json")
			{
				outfile.open(localname);
				outfile << success;
				outfile.close();

				Json resp_data;
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				/* code */
				resp_data.Parse(success.c_str());
				if (resp_data.HasParseError())
				{
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					SLOG_ERROR("result parse error:\n" + localname2);
					error = "Query fail: the result parse error.";
					response->Error(StatusOperationFailed, error);
				}
				else
				{
					resp_data.AddMember("StatusCode", 0, allocator);
					resp_data.AddMember("StatusMsg", "success", allocator);
					resp_data.AddMember("AnsNum", rs_ansNum, allocator);
					resp_data.AddMember("OutputLimit", rs_outputlimit, allocator);
					resp_data.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
					resp_data.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
					resp_data.AddMember("FileName", StringRef(filename.c_str()), allocator);

					response->set_header_pair("Cache-Control", "no-cache");
					response->set_header_pair("Pragma", "no-cache");
					response->set_header_pair("Expires", "0");

					SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
					response->Json(resp_data);
				}
			}
			else if (format == "sparql-results+json")
			{
				response->set_header_pair("Content-Type", "application/sparql-results+json");
				response->set_header_pair("Cache-Control", "no-cache");
				response->set_header_pair("Pragma", "no-cache");
				response->set_header_pair("Expires", "0");

				response->String(success);
			}
			else
			{
				error = "Unkown result format.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else if (update)
		{
			SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
			Json resp_data;
			resp_data.SetObject();
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "update query returns true.", allocator);
			resp_data.AddMember("AnsNum", ret_val, allocator);
			resp_data.AddMember("QueryTime", query_time, allocator);
			SLOG_DEBUG("response result:\n" << to_json_string(resp_data));
			response->Json(resp_data);
		}
		else
		{
			error = "search query returns false.";
			SLOG_DEBUG(error);
			response->Error(StatusOperationFailed, error);
		}
		apiUtil->unlock_database(db_name);
		SLOG_DEBUG("query complete!");
	}
	catch (const std::exception &e)
	{
		std::string error = "Query fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * export the database
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", "db_path": "the export path"}
 */
void export_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string db_path = jsonParam(json_data, "db_path");
		error = apiUtil->check_param_value("db_path", db_path);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not build yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		// check if database named [db_name] is already load
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			string error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		Util::string_suffix(db_path, '/');
		if (Util::dir_exist(db_path) == false)
		{
			Util::create_dirs(db_path);
		}
		std::string compress = jsonParam(json_data, "compress", "0");
		std::string zip_path;
		if (compress=="0")
			db_path = db_path + db_name + "_" + Util::get_timestamp() + ".nt";
		else
		{
			zip_path = db_path + db_name + "_" + Util::get_timestamp() + ".zip";
			db_path = db_name + "_" + Util::get_timestamp() + ".nt";
		}
		apiUtil->rdlock_database(db_name); // lock database
		SLOG_DEBUG("export_path: " + db_path);
		FILE *ofp = fopen(db_path.c_str(), "w");
		current_database->export_db(ofp);
		fflush(ofp);
		fclose(ofp);
		ofp = NULL;
		current_database = NULL;
		apiUtil->unlock_database(db_name); // unlock
		std::string success = "Export the database successfully.";

		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);

		if (compress=="0")
		{
			resp_data.AddMember("filepath", StringRef(db_path.c_str()), allocator);
			response->Json(resp_data);
		}
		else
		{
			if (!CompressUtil::FileHelper::compressExportZip(db_path, zip_path))
			{
				error = "export compress fail.";
				response->Error(StatusCompressError, error);
				Util::remove_path(zip_path);
				return;
			}
			resp_data.AddMember("filepath", StringRef(zip_path.c_str()), allocator);
			response->Json(resp_data);
			Util::remove_path(db_path);
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "Export fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * begin a transaction
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", isolevel: "the Isolation level， 1:RC(read committed) 2:SI(snapshot isolation) 3:SR(seriablizable）"}
 */
void begin_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string isolevel = jsonParam(json_data, "isolevel");
		error = apiUtil->check_param_value("isolevel", isolevel);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		int level = Util::string2int(isolevel);
		if (level <= 0 || level > 3)
		{
			error = "The isolation level's value only can been 1/2/3";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		string TID_s = apiUtil->begin_process(db_name, level, username);
		if (TID_s.empty())
		{
			error = "Transaction begin failed.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		StringBuffer s;
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Transaction begin success", allocator);
		resp_data.AddMember("TID", StringRef(TID_s.c_str()), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Transaction begin fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query database with transcation
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", sparql: "the sparql", tid: "the transcation id while begin transcation"}
 */
void tquery_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string TID_s = jsonParam(json_data, "tid");
		error = apiUtil->check_param_value("tid", TID_s);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}

		txn_id_t TID;
		TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID "+TID_s+" is not a pure number.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string sparql = jsonParam(json_data, "sparql");
		error = apiUtil->check_param_value("sparql", sparql);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		apiUtil->get_Txn_ptr(db_name, txn_m);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		SLOG_DEBUG("tquery sparql: " + sparql);
		std::string res;
		int ret = txn_m->Query(TID, sparql, res);
		if (ret == -1)
		{
			error = "Transaction query failed due to wrong TID";
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -10)
		{
			error = "Transaction query failed due to wrong database status";
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -99)
		{
			error = "Transaction query failed. This transaction is not in running status!";
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -100)
		{
			Json resp_data;
			Json::AllocatorType &allocator = resp_data.GetAllocator();
			resp_data.Parse(res.c_str());
			if (resp_data.HasParseError())
			{
				SLOG_ERROR("tquery result parse error.\n" + res);
				resp_data.Parse("{}");
				resp_data.AddMember("result", StringRef(res.c_str()), allocator);
			}
			resp_data.AddMember("StatusCode", 0, allocator);
			resp_data.AddMember("StatusMsg", "success", allocator);
			response->Json(resp_data);
		}
		else if (ret == -20)
		{
			error = "Transaction query failed. This transaction is set abort due to conflict!";
			apiUtil->aborted_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -101)
		{
			error = "Transaction query failed. Unknown query error";
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			string success = "Transaction query success, update num: " + Util::int2string(ret);
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction query fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * commit a transcation
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", tid: "the transcation id while begin transcation"}
 */
void commit_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string TID_s = jsonParam(json_data, "tid");
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		auto TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID " + TID_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		apiUtil->get_Txn_ptr(db_name, txn_m);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		int ret = txn_m->Commit(TID);
		if (ret == -1)
		{
			error = "Transaction not found, commit failed. TID: " + TID_s;
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == 1)
		{
			error = "Transaction not in running state! commit failed. TID: " + TID_s;
			apiUtil->rollback_process(txn_m, TID);
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			apiUtil->commit_process(txn_m, TID);
			// TODO auto checkpoint are sometimes blocked
			// auto latest_tid = txn_m->find_latest_txn();
			// SLOG_DEBUG("latest TID: "+ to_string(latest_tid));
			// if (latest_tid == 0)
			// {
			// 	SLOG_DEBUG("This is latest TID, auto checkpoint and save.");
			// 	txn_m->Checkpoint();
			// 	SLOG_DEBUG("Transaction checkpoint done.");
			// 	if (apiUtil->trywrlock_database(db_name))
			// 	{
			// 		current_database->save();
			// 		apiUtil->unlock_database(db_name);
			// 	}
			// 	else
			// 	{
			// 		SLOG_ERROR("The save operation can not been excuted due to loss of lock.");
			// 	}
			// }
			string success = "Transaction commit success. TID: " + TID_s;
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction commit fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * rollback a transcation
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", tid: "the transcation id while begin transcation"}
 */
void rollback_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string TID_s = jsonParam(json_data, "tid");
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		auto TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID " + TID_s + " is not a pure number.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		apiUtil->get_Txn_ptr(db_name, txn_m);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			response->Error(StatusTranscationManageFailed, error);
			return;
		}
		int ret = txn_m->Rollback(TID);
		if (ret == 1)
		{
			error = "Transaction not in running state! rollback failed. TID: " + TID_s;
			response->Error(StatusOperationFailed, error);
		}
		else if (ret == -1)
		{
			error = "Transaction not found, rollback failed. TID: " + TID_s;
			response->Error(StatusOperationFailed, error);
		}
		else
		{
			apiUtil->rollback_process(txn_m, TID);
			string success = "Transaction rollback success. TID: " + TID_s;
			response->Success(success);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction rollback fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * this is used for checkpoint, we must ensure that modification is written to disk
 * 
 * @param request 
 * @param response 
 * @param json_data 
 */
void checkpoint_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			shared_ptr<Txn_manager> txn_m;
			apiUtil->get_Txn_ptr(db_name, txn_m);
			if (txn_m == NULL)
			{
				error = "Get database transaction manager error.";
				apiUtil->unlock_database(db_name);
				response->Error(StatusTranscationManageFailed, error);
			}
			else
			{
				txn_m->Checkpoint();
				current_database->save();
				apiUtil->unlock_database(db_name);
				string success = "Database saved successfully.";
				response->Success(success);
			}
		}
	}
	catch (const std::exception &e)
	{
		string error = "Checkpoint fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}

}

/**
 * batch insert data
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", file: "the insert data file"}
 */
void batch_insert_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		bool is_file = true;
		std::string file = jsonParam(json_data, "file");
		std::string dir = jsonParam(json_data, "dir");
		error = apiUtil->check_param_value("file", file);
		if (error.empty() == false)
		{
			is_file = false;
			error = apiUtil->check_param_value("dir", dir);
			if (error.empty() == false)
			{
				error = "file and dir cannot be empty at the same time!";
				response->Error(StatusParamIsIllegal, error);
				return;
			}
		}
		if (is_file && Util::file_exist(file) == false)
		{
			error = "The data file is not exist";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (!is_file && Util::file_exist(dir) == false)
		{
			error = "The data directory is not exist";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		std::vector<std::string> zip_files;
		std::string unz_dir_path;
		std::string file_suffix = GRPCUtil::fileSuffix(file);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(file,[](std::string filename)->bool
				{
					if( apiUtil->check_upload_allow_extensions(GRPCUtil::fileSuffix(filename)) == false )
						return false;
					return true;
				});
			if( code != CompressUtil::UnZipOK )
			{
				string error = "uncompress is failed error.";
				response->Error(code, error);
				return;
			}
			std::string file_name = GRPCUtil::fileName(file);
			size_t pos = file_name.size() - file_suffix.size() - 1;
            unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			mkdir(unz_dir_path.c_str(), 0775);
			CompressUtil::UnCompressZip upfile(file, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				string error = "uncompress is failed error.";
				response->Error(code, error);
				return;
			}
			upfile.getFileList(zip_files, "");
		}

		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
			if (!unz_dir_path.empty())
			{
				Util::remove_path(unz_dir_path);
			}
		}
		else
		{
			std::string opt_id = apiUtil->generateUid();
			string remote_ip = task_of(response)->peer_addr();
			string msg = "Operation Success.";
			string operation = "batchInsert";
			apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
			string _dir = dir;
			std::string async = jsonParam(json_data, "async");
			std::string callback = jsonParam(json_data, "callback");
			auto insert_helper = [db_name,is_file,is_zip,file,zip_files,_dir,unz_dir_path,opt_id,async,callback]
				(GRPCResp *response)
				{
					string success = "Batch insert data successfully.";
					shared_ptr<Database> current_database;
					apiUtil->get_database(db_name, current_database);
					unsigned success_num = 0;
					unsigned total_num = 0;
					unsigned parse_error_num = 0;
					string error_log = _db_home +  "/" + db_name + _db_suffix + "/parse_error.log";
					if (is_file)
					{
						if (!is_zip)
						{
							total_num = Util::count_lines(error_log);
							success_num = current_database->batch_insert(file, false, nullptr);
							// exclude Info line
							parse_error_num = Util::count_lines(error_log) - total_num - 1;
						}
						else
						{
							total_num = Util::count_lines(error_log);
							for (std::string rdf_zip : zip_files)
							{
								SLOG_DEBUG("begin insert data from " + rdf_zip);
								success_num += current_database->batch_insert(rdf_zip, false, nullptr);
							}
							// exclude Info line
							parse_error_num = Util::count_lines(error_log) - total_num - zip_files.size();
						}
					}
					else
					{
						vector<string> files;
						string dir = _dir;
						Util::string_suffix(dir, '/');
						Util::dir_files(dir, "", files);
						total_num = Util::count_lines(error_log);
						for (string rdf_file : files)
						{
							SLOG_DEBUG("begin insert data from " + dir + rdf_file);
							success_num += current_database->batch_insert(dir + rdf_file, false, nullptr);
						}
						// exclude Info line
						parse_error_num = Util::count_lines(error_log) - total_num - files.size();
					}
					current_database->save();
					apiUtil->unlock_database(db_name);
					if (!unz_dir_path.empty())
					{
						Util::remove_path(unz_dir_path);
					}
					apiUtil->update_access_log(0, "Batch insert data successfully.", opt_id, 1, success_num, parse_error_num);
					if (async != "true")
					{
						Json resp_data;
						resp_data.SetObject();
						Json::AllocatorType &allocator = resp_data.GetAllocator();
						resp_data.AddMember("StatusCode", 0, allocator);
						resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resp_data.AddMember("success_num", success_num, allocator);
						resp_data.AddMember("failed_num", parse_error_num, allocator);
						resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						response->Json(resp_data);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"success_num\":\"" + std::to_string(success_num) + "\",";
						postdata += "\"failed_num\":\"" + std::to_string(parse_error_num) + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				};
			if (async == "true")
			{
				Json resp_data;
				resp_data.SetObject();
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
				response->Json(resp_data);
				thread t(insert_helper, nullptr);
				t.detach();
			}
			else
			{
				insert_helper(response);
			}
			
		}
			
			
	}
	catch (const std::exception &e)
	{
		string error = "Batch insert fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * batch remove data
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {db_name: "the operation database name", file: "the insert data file"}
 */
void batch_remove_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string file = jsonParam(json_data, "file");
		error = apiUtil->check_param_value("file", file);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (Util::file_exist(file) == false)
		{
			error = "The data file is not exist";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		std::vector<std::string> zip_files;
		std::string unz_dir_path;
		std::string file_suffix = GRPCUtil::fileSuffix(file);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(file,[](std::string filename)->bool
				{
					if( apiUtil->check_upload_allow_extensions(GRPCUtil::fileSuffix(filename)) == false )
						return false;
					return true;
				});
			if( code != CompressUtil::UnZipOK )
			{
				string error = "uncompress is failed error.";
				response->Error(code, error);
				return;
			}
			std::string file_name = GRPCUtil::fileName(file);
			size_t pos = file_name.size() - file_suffix.size() - 1;
            unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			mkdir(unz_dir_path.c_str(), 0775);
			CompressUtil::UnCompressZip upfile(file, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				string error = "uncompress is failed error.";
				response->Error(code, error);
				return;
			}
			upfile.getFileList(zip_files, "");
		}
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			response->Error(StatusLossOfLock, error);
		}
		else
		{
			std::string opt_id = apiUtil->generateUid();
			string remote_ip = task_of(response)->peer_addr();
			string msg = "Operation Success.";
			string operation = "batchRemove";
			apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
			std::string async = jsonParam(json_data, "async");
			std::string callback = jsonParam(json_data, "callback");
			auto remove_helper = [db_name,operation,file,opt_id,is_zip,zip_files,async,callback]
				(GRPCResp *response)
				{
					shared_ptr<Database> current_database;
					apiUtil->get_database(db_name, current_database);
					string success = "Batch remove data successfully.";
					unsigned success_num = 0;
					if (is_zip)
					{
						for (string rdf_file : zip_files)
						{
							success_num += current_database->batch_remove(rdf_file, false, nullptr);
						}
					}
					else
						current_database->batch_remove(file, false, nullptr);
					current_database->save();
					apiUtil->unlock_database(db_name);
					apiUtil->update_access_log(0, success, opt_id, 1, success_num, 0);
					if (async != "true")
					{
						Json resp_data;
						resp_data.SetObject();
						Json::AllocatorType &allocator = resp_data.GetAllocator();
						resp_data.AddMember("StatusCode", 0, allocator);
						resp_data.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resp_data.AddMember("success_num", StringRef(Util::int2string(success_num).c_str()), allocator);
						resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						response->Json(resp_data);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"success_num\":\"" + std::to_string(success_num) + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				};
			if (async == "true")
			{
				Json resp_data;
				resp_data.SetObject();
				Json::AllocatorType &allocator = resp_data.GetAllocator();
				resp_data.AddMember("StatusCode", 0, allocator);
				resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
				response->Json(resp_data);
				thread t(remove_helper, nullptr);
				t.detach();
			}
			else
			{
				remove_helper(response);
			}
		}
	}
	catch (const std::exception &e)
	{
		string error = "Batch remove fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void rename_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string db_name = jsonParam(json_data, "db_name");
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string new_name = jsonParam(json_data, "new_name");
		error = apiUtil->check_param_value("new_name", new_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		if (apiUtil->check_already_load(db_name) == true)
		{
			error = "Database is loaded, need unload first.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		// check new_name available
		if (apiUtil->check_db_exist(new_name) == true)
		{
			error = "Database name " + new_name + " already exists.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}

		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "Unable to rename due to loss of lock";
			response->Error(StatusLossOfLock, error);
			return;
		}
		string new_db_path = _db_home + "/" + new_name + _db_suffix;
		// check new_db_path.
		if (Util::dir_exist(new_db_path))
		{
			error = "Database local path " + new_db_path + " already exists.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			apiUtil->unlock_databaseinfo(db_info);
			return;
		}
		
		// mv old folder to new folder
		string db_path = _db_home + "/" + db_name + _db_suffix;
		string sys_cmd = "mv " + db_path + " " + new_db_path;
		std::system(sys_cmd.c_str());
		apiUtil->unlock_databaseinfo(db_info);
		// insert new_name
		apiUtil->add_already_build(new_name, db_info->getCreator(), db_info->getTime());
		// copy privileges
		apiUtil->copy_privilege(db_name, new_name);		
		// add backuplog
		Util::add_backuplog(new_name);

		// remove old_name
		apiUtil->delete_from_already_build(db_name);
		// remove backuplog
		Util::delete_backuplog(db_name);

		std::string success = "Database rename successfully.";
		response->Success(success);
	}
	catch (const std::exception &e)
	{
		std::string error = "rename fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

// for user

/**
 * user manager (include add user,delete user,alter user),notice that only support alter user's password
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {type: "1 for add user, 2 for delete user, 3 for alert user password", op_username: "the user who be operated", op_password: "new password(for type 3)"}
 */
void user_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string op_username = jsonParam(json_data, "op_username");
		std::string error = apiUtil->check_param_value("op_username", op_username);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string op_password = jsonParam(json_data, "op_password");
		std::string type = jsonParam(json_data, "type");
		if (type != "2")
		{
			error = apiUtil->check_param_value("op_password", op_password);
			if (error.empty() == false)
			{
				response->Error(StatusParamIsIllegal, error);
				return;
			}
		}
		
		if (type == "1") // add user
		{
			// check user number
			if (apiUtil->check_user_count() == false)
			{
				string error = "The total number of users more than max_user_num.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			if (apiUtil->user_add(op_username, op_password))
			{
				response->Success("Add user done.");
			}
			else
			{
				error = "Username already existed, add user failed.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else if (type == "2") // delete user
		{
			if (op_username == apiUtil->get_root_username())
			{
				error = "You cannot delete root, delete user failed.";
				response->Error(StatusOperationFailed, error);
			}
			else if (apiUtil->user_delete(op_username))
			{
				response->Success("Delete user done.");
			}
			else
			{
				error = "Username not exist, delete user failed.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else if (type == "3") // alert password
		{
			if (apiUtil->user_pwd_alert(op_username, op_password))
			{
				response->Success("Change password done.");
			}
			else
			{
				error = "Username not exist, change password failed.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else
		{
			error = "The operation is not support.";
			response->Error(StatusParamIsIllegal, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "User manage fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void reason_manage_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		string type="";
		if(json_data.HasMember("type"))
		{
          if(json_data["type"].IsInt())
		  {
			type=json_data["type"].GetInt()+"";
		  }
		  else if(json_data["type"].IsString())
		  {
			type=json_data["type"].GetString();
		  }
		  

		}
		
		std::string db_name = jsonParam(json_data, "db_name");
		std::string operation="AddReason";
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		if(type.empty())
		{
			response->Error(StatusParamIsIllegal, "The type parameter is illegal");
			return;
		}
		if (type == "1") // add Reason Rule
		{
			operation="AddReasonRule";
			// check user number
			if(json_data.HasMember("ruleinfo")==false)
			{
				error="the data has not the rule information";
			 	
			 	response->Error(StatusParamIsIllegal, error);
				return;
			}
			Value reasonInfo=json_data["ruleinfo"].GetObject();
			Document::AllocatorType &allocator = json_data.GetAllocator();
			std::string createtime = Util::get_date_time();
			reasonInfo.AddMember("status","新建",allocator);
			reasonInfo.AddMember("createtime",StringRef(createtime.c_str()),allocator);
			ReasonOperationResult resultInfo= ReasonHelper::saveReasonRuleInfo(reasonInfo,db_name,_db_home,_db_suffix);
			if(resultInfo.issuccess==1)
			{
				response->Success("Add Reason Rule done."+resultInfo.error_message);
			}
			else
			{
				error="Add Reason Rule Fail. "+resultInfo.error_message;
				response->Error(StatusOperationFailed,error);
			}
			
		}
		else if (type == "2") // listReason
		{
			operation = "listReasonRules";
            string db_path=_db_home+db_name+_db_suffix;
			vector<string> liststr = ReasonHelper::getReasonRuleList(db_path);
			stringstream str_stream;
			str_stream << "[";
            for(int i=0;i<liststr.size();i++)
			{
				if (i > 0) 
					str_stream << ",";
				str_stream << liststr[i];
			}
			str_stream << "]";
			string arrayStr = str_stream.str();
			Document list;
			list.SetArray();
			list.Parse(arrayStr.c_str());
			if (list.HasParseError()) 
			{

				error="pasrse rulefiles error:" + arrayStr;
				response->Error(StatusOperationFailed,error);
			} 
			else 
			{
				Json doc;
				Document::AllocatorType &allocator = doc.GetAllocator();
				unsigned num = list.Size();
				doc.SetObject();
				doc.AddMember("StatusCode", 0, allocator);
				doc.AddMember("StatusMsg", "ok", allocator);
				doc.AddMember("list", list.Move(), allocator);
				doc.AddMember("num",  num, allocator);
           		response->Json(doc);
			}
		}
		else if (type == "3") // Compile Reason Rule
		{
			operation = "compileReasonRule";
			string rulename=json_data["rulename"].GetString();
            ReasonSparql resultInfo=ReasonHelper::compileReasonRule(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
				
			}
			else{
			doc.AddMember("StatusCode",0,allocator);
			doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
			doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
			doc.AddMember("check_sparql",StringRef(resultInfo.check_sparql.c_str()),allocator);
		
			 response->Json(doc);
			}
		}
		else if(type=="4") //execute Reason
		{
			operation = "executeReasonRule";
			string rulename=json_data["rulename"].GetString();
			string username=json_data["username"].GetString();
			ReasonSparql resultInfo= ReasonHelper::executeReasonRule(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
				
			}
			else{
			doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
	        if(apiUtil->check_db_exist(db_name)==false)
			{
				error="the database is not exist!";
				response->Error(StatusOperationFailed,error);
			    return;
			}
            shared_ptr<Database> current_database;
			
			// check database load status
			apiUtil->get_database(db_name, current_database);
			if (current_database == NULL)
			{
				throw runtime_error("Database not load yet.");
			}
			bool lock_rt = apiUtil->rdlock_database(db_name);
			if (lock_rt)
			{
				SLOG_DEBUG("get current database read lock success: " + db_name);
			}
			else
			{
				//throw runtime_error("get current database read lock fail.");
				error="get current database read lock fail.";
				response->Error(StatusOperationFailed,error);
			    return;
			}
			ResultSet rs;
			int ret_val;
			FILE *output = NULL;
			string sparql = resultInfo.insert_sparql;
			bool update_flag_bool = true;
			try
			{
				// SLOG_DEBUG("begin query...");
				rs.setUsername(username);
				ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
			}
			catch (string exception_msg)
			{
				string content = exception_msg;
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				return;
			}
            // cout<<"rs.number:"<<rs.ansNum<<",rs.answer:"<<rs.answer<<endl;
			
			// cout << "ans:" << rs_ansNum << endl;
			doc.AddMember("AnsNum", ret_val, allocator);
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			current_database->save();
			apiUtil->unlock_database(db_name);
            ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已执行",_db_home,_db_suffix);
		    response->Json(doc);
			}
			
		}
		else if(type=="5")
		{
			operation = "disableReasonRule";
			string rulename=json_data["rulename"].GetString();
			ReasonSparql resultInfo= ReasonHelper::disableReasonRule(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
			    
			}
			else
			{
			doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
			string username=json_data["username"].GetString();
			if(apiUtil->check_db_exist(db_name)==false)
			{
				error="the database is not exist!";
				response->Error(StatusOperationFailed,error);
			    return;
			}
            shared_ptr<Database> current_database;
			bool update_flag_bool=true;
			// check database load status
			apiUtil->get_database(db_name, current_database);
			if (current_database == NULL)
			{
				throw runtime_error("Database not load yet.");
			}
			bool lock_rt = apiUtil->rdlock_database(db_name);
			if (lock_rt)
			{
				SLOG_DEBUG("get current database read lock success: " + db_name);
			}
			else
			{
				//throw runtime_error("get current database read lock fail.");
				error="get current database read lock fail.";
				response->Error(StatusOperationFailed,error);
			    return;
			}
			ResultSet rs;
			int ret_val;
			FILE *output = NULL;
			string sparql = resultInfo.delete_sparql;
			try
			{
				// SLOG_DEBUG("begin query...");
				rs.setUsername(username);
				ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
			}
			catch (string exception_msg)
			{
				string content = exception_msg;
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				return;
			}
			
            // long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
            //cout << "ans:" << ret_val << endl;
			doc.AddMember("AnsNum",ret_val,allocator);
			
          
		
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			current_database->save();
			apiUtil->unlock_database(db_name);
             ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已失效",_db_home,_db_suffix);
		      response->Json(doc);
			}

		}
		else if(type=="6")
		{
		     operation = "showReasonRule";
			string rulename=json_data["rulename"].GetString();
			ReasonOperationResult resultInfo= ReasonHelper::getReasonInfo(rulename,db_name,_db_home,_db_suffix);
		
			
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
			    
			}
			
			
			else
			{
				// 输出格式化的JSON
				std::string ruleinfo = "{\"ruleinfo\":" + resultInfo.error_message + "}";
				Document doc;
				doc.SetObject();
				doc.Parse(ruleinfo.c_str());
				Document::AllocatorType &allocator = doc.GetAllocator();
				doc.AddMember("StatusCode",0,allocator);
				doc.AddMember("StatusMsg","ok",allocator);
				response->Json(doc);
			}
			
		}
		else if (type == "7")
		{
			operation = "deleteReasonRule";
			string rulename = json_data["rulename"].GetString();
			ReasonOperationResult resultInfo = ReasonHelper::getReasonInfo(rulename, db_name, _db_home, _db_suffix);
			if (resultInfo.issuccess == 0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
				return;
			}

			else
			{
				// 输出格式化的JSON
				Document doc;
				doc.SetObject();
				doc.Parse(resultInfo.error_message.c_str());
				Document::AllocatorType &allocator = doc.GetAllocator();
				if (doc.HasMember("status")&&doc["status"].GetString() == "已执行")
				{
					string error_msg = "该规则已经执行，请先让该规则失效后再执行";
					response->Error(StatusOperationFailed,error_msg);
					return;
				}
				
				ReasonOperationResult resultInfo2 = ReasonHelper::removeReasonRule(rulename, db_name, _db_home, _db_suffix);
				if (resultInfo2.issuccess == 1)
				{
					Document doc2;
				    doc2.SetObject();
					doc2.AddMember("StatusCode", 0, allocator);
					doc2.AddMember("StatusMsg", StringRef(resultInfo2.error_message.c_str()), allocator);
					response->Json(doc2);
					return;
				}
				else
				{

					response->Error(StatusOperationFailed,resultInfo2.error_message);
					return;
				}
			}
		}
		else if (type == "8")
		{
			operation = "checkReasonRule";
			string rulename=json_data["rulename"].GetString();
			ReasonSparql resultInfo= ReasonHelper::getCheckSparql(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			doc.SetObject();
			int effectNum=0;
			string checkMsg="ok";
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				response->Error(StatusOperationFailed,resultInfo.error_message);
			    
			}
			else
			{
			doc.AddMember("check_sparql",StringRef(resultInfo.check_sparql.c_str()),allocator);
			string username=json_data["username"].GetString();
			if(apiUtil->check_db_exist(db_name)==false)
			{
				error="the database is not exist!";
				response->Error(StatusOperationFailed,error);
			    return;
			}
            shared_ptr<Database> current_database;
			bool update_flag_bool=true;
			// check database load status
			apiUtil->get_database(db_name, current_database);
			if (current_database == NULL)
			{
				throw runtime_error("Database not load yet.");
			}
			bool lock_rt = apiUtil->rdlock_database(db_name);
			if (lock_rt)
			{
				SLOG_DEBUG("get current database read lock success: " + db_name);
			}
			else
			{
				//throw runtime_error("get current database read lock fail.");
				error="get current database read lock fail.";
				response->Error(StatusOperationFailed,error);
			    return;
			}
			ResultSet rs;
			int ret_val;
			FILE *output = NULL;
			string sparql = resultInfo.check_sparql;
			try
			{
				// SLOG_DEBUG("begin query...");
				rs.setUsername(username);
				ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
			}
			catch (string exception_msg)
			{
				string content = exception_msg;
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_database(db_name);
				response->Error(StatusOperationFailed,content);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			
            // long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
           // cout << "ans:" << ret_val << endl;
			//doc.AddMember("AnsNum",ret_val,allocator);
			string json=rs.to_JSON();
            //cout << "ans:" << ret_val << endl;
			// doc.AddMember("result",StringRef(json.c_str()),allocator);
          
		    Document doc2;
			doc2.SetObject();
			doc2.Parse(json.c_str());
			if(doc2.HasParseError())
			{
				checkMsg="query result is not json format!";
				effectNum=0;
				doc.AddMember("effectNum",effectNum,allocator);
				doc.AddMember("checkMsg",StringRef(checkMsg.c_str()),allocator);
			}
			if(doc2.HasMember("results"))
			{
				Value results=doc2["results"].GetObject();
				if(results.HasMember("bindings"))
				{
					Value bindings=results["bindings"].GetArray();
					if(bindings.Size()>0)
					{
						Value resultobj=bindings[0]["result"].GetObject();
						if(resultobj.HasMember("value"))
						{
							string result_value=resultobj["value"].GetString();
							// int result_value_int=Util::string2int(result_value);
							effectNum=Util::string2int(result_value);
							doc.AddMember("effectNum",effectNum,allocator);
						}
						else
						{
							effectNum=0;
						    doc.AddMember("effectNum",effectNum,allocator);
						}
					}
					else
					{
						effectNum=0;
						doc.AddMember("effectNum",effectNum,allocator);
					}
				}
				doc.AddMember("checkMsg",StringRef(checkMsg.c_str()),allocator);
			}
			ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,effectNum,_db_home,_db_suffix,checkMsg);
		
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			current_database->save();
			apiUtil->unlock_database(db_name);
             ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已校验",_db_home,_db_suffix);
		      response->Json(doc);
			}
		}
		else
		{
			error = "The operation is not support.";
			response->Error(StatusParamIsIllegal, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Reason Manage fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
/**
 * show all user list
 * 
 * @param request 
 * @param response 
 */
void user_show_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		vector<shared_ptr<struct DBUserInfo>> userList;
		apiUtil->get_user_info(userList);
		if (userList.empty())
		{
			response->Success("No Users");
			return;
		}
		size_t count = userList.size();
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<struct DBUserInfo> useInfo = userList[i];
			array_data.PushBack(useInfo->toJSON(allocator).Move(), allocator);
		}

		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("ResponseBody", array_data, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Show user fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * manage the user privilege, include add user privilege, delete user privilege
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {
 *   type: "1 for add privilege, 2 for delete privilege, 3 for clear privilege", 
 *   op_username: "the user who be operated",
 *   privileges: "1:query,2:load,3:unload,4:update,5:backup,6:restore,7:export(multi privileges by using ',' to split)",
 *   db_name: "the operation database name"
 * }
 */
void user_privilege_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string type = jsonParam(json_data, "type");
		std::string error = apiUtil->check_param_value("type", type);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}  
		else if (type != "1" && type != "2" && type != "3")
		{
			error = "The type " + type + " is not support.";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string op_username = jsonParam(json_data, "op_username");
		error = apiUtil->check_param_value("op_username", op_username);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		} 
		else if (apiUtil->check_user_exist(op_username) == false)
		{
			error = "The username is not exists.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		}
		else if (op_username == apiUtil->get_root_username())
		{
			error = "You can't change privileges for root user.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
			return;
		} 
		
		std::string db_name = jsonParam(json_data, "db_name");
		std::string privileges = jsonParam(json_data, "privileges");
		if (type != "3")
		{
			error = apiUtil->check_param_value("db_name", db_name);
			if (error.empty() == false)
			{
				response->Error(StatusParamIsIllegal, error);
				return;
			}
			// check database exist
			if (apiUtil->check_db_exist(db_name) == false)
			{
				error = "Database not build yet.";
				response->Error(StatusOperationConditionsAreNotSatisfied, error);
				return;
			}
			error = apiUtil->check_param_value("privileges", privileges);
			if (error.empty() == false)
			{
				response->Error(StatusParamIsIllegal, error);
				return;
			}
		}
		
		if (type == "3")
		{
			// clear the user all privileges
			int resultint = apiUtil->clear_user_privilege(op_username);
			if (resultint == 1)
			{
				response->Success("Clear the all privileges for the user successfully!");
			}
			else
			{
				error = "Clear the all privileges for the user fail.";
				response->Error(StatusOperationFailed, error);
			}
		}
		else
		{
			vector<string> privilege_vector;
			if (privileges.substr(privileges.length() - 1, 1) != ",")
			{
				privileges = privileges + ",";
			}
			Util::split(privileges, ",", privilege_vector);
			vector<string> privilegeTypes;
			for (unsigned i = 0; i < privilege_vector.size(); i++)
			{
				std::string temp_privilege_int = privilege_vector[i];
				if (temp_privilege_int.empty())
				{
					continue;
				}
				if (temp_privilege_int == "1")
				{
					privilegeTypes.push_back("query");
				}
				else if (temp_privilege_int == "2")
				{
					privilegeTypes.push_back("load");
				}
				else if (temp_privilege_int == "3")
				{
					privilegeTypes.push_back("unload");
				}
				else if (temp_privilege_int == "4")
				{
					privilegeTypes.push_back("update");
				}
				else if (temp_privilege_int == "5")
				{
					privilegeTypes.push_back("backup");
				}
				else if (temp_privilege_int == "6")
				{
					privilegeTypes.push_back("restore");
				}
				else if (temp_privilege_int == "7")
				{
					privilegeTypes.push_back("export");
				} 
				else
				{
					SLOG_DEBUG("The privilege " + temp_privilege_int + " undefined.");
					continue;
				} 
			}
			string result="";
			if (privilegeTypes.size() > 0) 
			{
				string privilegeNames="";
				for (size_t i = 0; i < privilegeTypes.size(); i++)
				{
					if (i > 0) 
					{
						privilegeNames = privilegeNames + ",";
					}
					privilegeNames = privilegeNames + privilegeTypes[i];
				}
				if (type == "1")
				{
					if (apiUtil->add_privilege(op_username, privilegeTypes, db_name) == 0)
					{
						result = result + "add privilege " + privilegeNames + " failed.";
						response->Error(StatusOperationFailed, result);
					}
					else
					{
						result = result + "add privilege " + privilegeNames + " successfully.";
						response->Success(result);
					}
				}
				else if (type == "2")
				{
					if (apiUtil->del_privilege(op_username, privilegeTypes, db_name) == 0)
					{
						result += "delete privilege " + privilegeNames + " failed.";
						response->Error(StatusOperationFailed, result);
					}
					else
					{
						result += "delete privilege " + privilegeNames + " successfully.";
						response->Success(result);
					}
				}
				else
				{
					result = "the operation type is not support.";
					response->Error(StatusParamIsIllegal, result);
				}
			}
			else
			{
				result = "not match any valid privilege, valid values between 1 and 7.";
				response->Error(StatusParamIsIllegal, result);
			}
		}
	}
	catch (const std::exception &e)
	{
		std::string error = "User privilege manage fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * change user password
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {op_password: "new password"}
 */
void user_password_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string op_password = jsonParam(json_data, "op_password");
		std::string error = apiUtil->check_param_value("op_password", op_password);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		if (apiUtil->user_pwd_alert(username, op_password))
		{
			response->Success("Change password done.");
		}
		else
		{
			error = "Username not exist.";
			response->Error(StatusOperationConditionsAreNotSatisfied, error);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Change password fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}
// for log
/**
 * get transcation log
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {pageNo: "page number, default 1", pageSize: "number of per page, default 10"}
 */
void txn_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		// std::string username = jsonParam(json_data, "username");
		// if (username != apiUtil->get_root_username())
		// {
		// 	std::string error = "Root User Only!";
		// 	response->Error(StatusOperationConditionsAreNotSatisfied, error);
		// 	return;
		// }
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		struct TransactionLogs transactionLogs;
		apiUtil->get_transactionlog(page_no, page_size, &transactionLogs);
		vector<struct TransactionLogInfo> logList = transactionLogs.getTransactionLogInfoList();
		size_t count = logList.size();
		Json resp_data;
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			TransactionLogInfo log_info = logList[i];
			array_data.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}
		int totalSize = transactionLogs.getTotalSize();
		int totalPage = transactionLogs.getTotalPage();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get transaction log success.", allocator);
		resp_data.AddMember("totalSize", totalSize, allocator);
		resp_data.AddMember("totalPage", totalPage, allocator);
		resp_data.AddMember("pageNo", page_no, allocator);
		resp_data.AddMember("pageSize", page_size, allocator);
		resp_data.AddMember("list", array_data, allocator);

		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get transaction log fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get query log
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {date:"log date, required format is yyyyMMdd", pageNo: "page number, default 1", pageSize: "number of per page, default 10"}
 */
void query_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string date = jsonParam(json_data, "date");
		std::string error = apiUtil->check_param_value("date", date);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		struct DBQueryLogs dbQueryLogs;
		apiUtil->get_query_log(date, page_no, page_size, &dbQueryLogs);
		vector<struct DBQueryLogInfo> logList = dbQueryLogs.getQueryLogInfoList();
		size_t count = logList.size();
		
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			DBQueryLogInfo log_info = logList[i];
			array_data.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}

		int totalSize = dbQueryLogs.getTotalSize();
		int totalPage = dbQueryLogs.getTotalPage();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get query log success", allocator);
		resp_data.AddMember("totalSize", totalSize, allocator);
		resp_data.AddMember("totalPage", totalPage, allocator);
		resp_data.AddMember("pageNo", page_no, allocator);
		resp_data.AddMember("pageSize", page_size, allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get query log fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get query log date
 * 
 * @param request 
 * @param response 
 */
void query_log_date_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		vector<string> logfiles;
		apiUtil->get_query_log_files(logfiles);
		sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
			return a > b;
		});
		size_t count = logfiles.size();
		std::string item;
		Json resp_data;
		rapidjson::Value array_data(rapidjson::kArrayType);
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			array_data.PushBack(rapidjson::Value().SetString(item.c_str(), allocator).Move(), allocator);
		}
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get query log date success", allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch(const std::exception& e)
	{
		string error = "Get query log date fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get access log
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {date:"log date, required format is yyyyMMdd", pageNo: "page number, default 1", pageSize: "number of per page, default 10"}
 */
void access_log_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string date = jsonParam(json_data, "date");
		std::string error = apiUtil->check_param_value("date", date);
		if (error.empty() == false)
		{
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		int page_no = jsonParam(json_data, "pageNo", 1);
		int page_size = jsonParam(json_data, "pageSize", 10);
		struct DBAccessLogs dbAccessLogs;
		apiUtil->get_access_log(date, page_no, page_size, &dbAccessLogs);
		vector<struct DBAccessLogInfo> logList = dbAccessLogs.getAccessLogInfoList();
		size_t count = logList.size();
		Json resp_data;
		rapidjson::Value array_data(rapidjson::kArrayType);
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			DBAccessLogInfo log_info = logList[i];
			array_data.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}
		int totalSize = dbAccessLogs.getTotalSize();
		int totalPage = dbAccessLogs.getTotalPage();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get access log success", allocator);
		resp_data.AddMember("totalSize", totalSize, allocator);
		resp_data.AddMember("totalPage", totalPage, allocator);
		resp_data.AddMember("pageNo", page_no, allocator);
		resp_data.AddMember("pageSize", page_size, allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		string error = "Get access log fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * get access log date
 * 
 * @param request 
 * @param response 
 */
void access_log_date_task(const GRPCReq *request, GRPCResp *response)
{
	try
	{
		vector<string> logfiles;
		apiUtil->get_access_log_files(logfiles);
		sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
			return a > b;
		});
		size_t count = logfiles.size();
		std::string item;
		Json resp_data;
		rapidjson::Value array_data(rapidjson::kArrayType);
		resp_data.SetObject();
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			array_data.PushBack(rapidjson::Value().SetString(item.c_str(), allocator).Move(), allocator);
		}
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get access log date success", allocator);
		resp_data.AddMember("list", array_data, allocator);
		response->Json(resp_data);
	}
	catch(const std::exception& e)
	{
		string error = "Get access log date fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * query personalized function
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {
 *   funInfo:{
 *     funName: "search function name", 
 *     funStatus: "1:unbuild,2:builded,3:error"
 *   }
 * }
 */
void fun_query_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		struct PFNInfo pfn_info;
		if (hasJsonParam(json_data, "funInfo"))
		{
			rapidjson::Value &fun_info = json_data["funInfo"];
			pfnUtil->build_PFNInfo(fun_info, &pfn_info);
		}
		std::string username =  jsonParam(json_data, "username");
		struct PFNInfos *pfn_infos = new PFNInfos();
		pfnUtil->fun_query(pfn_info.getFunName(), pfn_info.getFunStatus(), username, pfn_infos);
		vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
		size_t count = list.size();
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		rapidjson::Value array_data(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			PFNInfo pfn_info = list[i];
			array_data.PushBack(pfn_info.toJSON(allocator).Move(), allocator);
		}
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("list", array_data, allocator);

		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Fun query fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

/**
 * personalized function manager, include create、update、delete and build
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * { 
 *   type: "1:create,2:update,3:delete", 
 *   funInfo: {
 *     funName: "function name",
 *     funDesc: "function description",
 *     funArgs: "1: no K-Hop param, 2: has K-Hop param",
 *     funBody: "function body",
 *     funSubs: "sub function",
 *     funReturn: "path/multipath/kv/value"
 *    }
 * }
 */
void fun_cudb_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	std::string type = jsonParam(json_data, "type");
	std::string error = apiUtil->check_param_value("type", type);
	if (error.empty() == false)
	{
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	if (hasJsonParam(json_data, "funInfo") == false)
	{
		error = "the value of funInfo can not be empty!";
		response->Error(StatusParamIsIllegal, error);
		return;
	}
	std::string username = jsonParam(json_data, "username");
	struct PFNInfo pfn_info;
	rapidjson::Value &fun_info = json_data["funInfo"];
	pfnUtil->build_PFNInfo(fun_info, &pfn_info);
	if (type == "1")
	{
		try
		{
			pfnUtil->fun_create(username, &pfn_info);
			response->Success("Function create success.");
		}
		catch(const std::exception& e)
		{
			std::string error = "Function create fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else if (type == "2")
	{
		try
		{
			pfnUtil->fun_update(username, &pfn_info);
			response->Success("Function update success.");
		}
		catch(const std::exception& e)
		{
			std::string error = "Function update fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else if (type == "3")
	{
		try
		{
			pfnUtil->fun_delete(username, &pfn_info);
			response->Success("Function delete success.");
		}
		catch(const std::exception& e)
		{
			std::string error = "Function delete fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else if (type == "4")
	{
		try
		{
			string result = pfnUtil->fun_build(username, pfn_info.getFunName());
			if (result == "")
			{
				response->Success("Function build success.");
			}
			else
			{
				response->Error(StatusOperationFailed, result);
			}
		}
		catch(const std::exception& e)
		{
			std::string error = "Function build fail: " + string(e.what());
			response->Error(StatusOperationFailed, error);
		}
	}
	else
	{
		error = "The type is invalid, please look up the api document.";
		response->Error(StatusParamIsIllegal, error);
	}
}

/**
 * review personalized function
 * 
 * @param request 
 * @param response 
 * @param json_data 
 * {  
 *   funInfo: {
 *     funName: "function name",
 *     funDesc: "function description",
 *     funArgs: "1: no K-Hop param, 2: has K-Hop param",
 *     funBody: "function body",
 *     funSubs: "sub function",
 *     funReturn: "path/multipath/kv/value"
 *    }
 * }
 */
void fun_review_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		std::string error;
		if (hasJsonParam(json_data, "funInfo") == false)
		{
			error = "the value of funInfo can not be empty!";
			response->Error(StatusParamIsIllegal, error);
			return;
		}
		std::string username = jsonParam(json_data, "username");
		struct PFNInfo pfn_info;
		rapidjson::Value &fun_info = json_data["funInfo"];
		pfnUtil->build_PFNInfo(fun_info, &pfn_info);
		pfnUtil->fun_review(username, &pfn_info);
		string content = pfn_info.getFunBody();
		content = Util::urlEncode(content);
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("Result", StringRef(content.c_str()), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "Function review fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void stat_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	try
	{
		int pid = getpid();
		float cup_usage = Util::get_cpu_usage(pid) * 100; // %
		char cup_usage_char[32];
		sprintf(cup_usage_char, "%f", cup_usage);
		float mem_usage = Util::get_memory_usage(pid); // MB
		char mem_usage_char[32];
		sprintf(mem_usage_char, "%f", mem_usage);
		unsigned long long disk_available = Util::get_disk_free(); // MB
		char disk_available_char[32];
		sprintf(disk_available_char, "%llu", disk_available);
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("cup_usage", StringRef(cup_usage_char), allocator);
		resp_data.AddMember("mem_usage", StringRef(mem_usage_char), allocator);
		resp_data.AddMember("disk_available", StringRef(disk_available_char), allocator);
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		std::string error = "stat fail: " + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}

void checkOperationState_task(const GRPCReq *request, GRPCResp *response, Json &json_data)
{
	string error;
	string operation = "checkOperationState";
	try
	{
		std::string opt_id = jsonParam(json_data, "opt_id");
		if (opt_id.empty())
		{
			error = "opt_id is empty.";
			response->Error(StatusOperationFailed, error);
			return;
		}
		struct DBAccessLogInfo log;
		if (!apiUtil->getAccessLogByOptId(opt_id, log))
		{
			error = "opt_id not found.";
			response->Error(StatusOperationFailed, error);
			return;
		}
		Json resp_data;
		Json::AllocatorType &allocator = resp_data.GetAllocator();
		string msg = log.getMsg();
		std::string backupfilepath = log.getBackupfilepath();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", log.getCode(), allocator);
		resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
		resp_data.AddMember("state", log.getState(), allocator);
		std::string log_operation = log.getOperation();
		if (log_operation == "backup")
		{
			resp_data.AddMember("backupfilepath", StringRef(backupfilepath.c_str()), allocator);
		}
		else if(log_operation != "restore")
		{
			resp_data.AddMember("success_num", log.getNum(), allocator);
			resp_data.AddMember("failed_num", log.getFailNum(), allocator);
		}
		response->Json(resp_data);
	}
	catch (const std::exception &e)
	{
		error = "checkbatchInsertUid fail:" + string(e.what());
		response->Error(StatusOperationFailed, error);
	}
}