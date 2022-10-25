/*
 * @Author: liwenjie
 * @Date: 2021-08-22 20:14:02
 * @LastEditTime: 2022-10-25 11:14:52
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: batch insert data tool
 * @FilePath: /gstore/Main/gadd.cpp
 */

#include "../Database/Database.h"
#include "../Util/Util.h"
//#include "../Util/Slog.h"
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
	// chdir(dirname(argv[0]));
	//#ifdef DEBUG
	//*! [important] you should init the Util class.
	Util util;
	//#endif
	// Log.init("slog.properties");

	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		// Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
		cout << "Invalid arguments! Input \"bin/gadd -h\" for help." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Batch Insert Data Tools(gadd)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gadd -db [dbname] -f [filename] " << endl;
			cout << "      \tbin/gadd -db [dbname] -dir [dirname] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file,\t\tthe file path for inserting data." << endl;
			cout << "\t-dir,--directory,\t\tthe directory path for inserting datas." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			// cout << "the command is not complete." << endl;
			// Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
			cout << "Invalid arguments! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		string db_folder = Util::getArgValue(argc, argv, "db", "database");
		if (db_folder.empty())
		{
			/*		cout << "please input the database name " << endl;*/
			// Log.Error("the database name is empty! Input \"bin/gbackup -h\" for help.");
			cout << "the database name is empty! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
		int len = db_folder.length();

		if (db_folder.substr(len - 3, 3) == ".db")
		{
			/*cout << "your database can not end with .db" << endl;*/
			// Log.Error("your database can not end with .db.! Input \"bin/gadd -h\" for help.");
			cout << "your database can not end with .db.! Input \"bin/gadd -h\" for help." << endl;
			return -1;
		}
		string filename = Util::getArgValue(argc, argv, "f", "file");
		string dirname = Util::getArgValue(argc, argv, "dir", "directory");
		if (filename.empty() && dirname.empty())
		{
			// cout << "please input the file path." << endl;
			// Util::formatPrint("please input the file path.", "Error");
			// Log.Error("the add data file is empty! Input \"bin/gadd -h\" for help.");
			cout << "the add data file is empty! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}

		Database _db(db_folder);
		_db.load();
		cout << "finish loading" << endl;

		// Log.Info("finish loading.");
		//_db.insert(argv[2]);
		//_db.remove(argv[2]);
		long tv_begin = Util::get_cur_time();
		unsigned success_num = 0;
		unsigned total_num = 0;
		if (filename.empty() == false)
		{
			total_num = Util::count_lines(filename);
			success_num = _db.batch_insert(filename, false, nullptr);
		} 
		else if (dirname.empty() == false)
		{
			vector<string> files;
			if (dirname[dirname.length() - 1] != '/')
			{
				dirname.push_back('/');
			}
			Util::dir_files(dirname, "", files);
			for (string rdf_file : files)
			{
				cout << "begin insert data from " << dirname << rdf_file << endl;
				total_num += Util::count_lines(dirname + rdf_file);
				success_num += _db.batch_insert(dirname + rdf_file, false, nullptr);
			}
		}
		long tv_end = Util::get_cur_time();
		// cout << "finish insert data" << endl;
		unsigned parse_error_num = total_num - success_num;
		cout << "after inserted triples num "<< success_num <<",failed num " << parse_error_num <<",used " << (tv_end - tv_begin) << " ms" << endl;
		_db.save();

		/*stringstream ss;
		ss << "after insert,used " << (tv_end - tv_begin) << " ms";
		Log.Info(ss.str().c_str());*/
		/*delete _db;
		long tv_end1 = Util::get_cur_time();
		ss.str("");
		ss << "persistence on disk" << (tv_end1 - tv_end) << " ms";
		Log.Info(ss.str());*/
	}

	return 0;
}
