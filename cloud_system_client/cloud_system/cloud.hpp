#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<unordered_map>
#ifdef _WIN32
#include<filesystem>
#include "httplib.h"
#include<windows.h>
#else
#include<experimental/filesystem>
#include<unistd.h>
#endif



namespace cloud_sys {
	namespace fs = std::experimental::filesystem;

	class ScanDir {
	private:
		std::string _path;
	public:
		ScanDir(const std::string &path)
			:_path(path)
		{
			if (!fs::exists(path)) {
				fs::create_directories(path);
			}
			if (_path.back() != '/') {
				_path += '/';
			}
		}


		bool Scan(std::vector<std::string> *v) {
			for (auto &file : fs::directory_iterator(_path)) {
				std::string name;
				name = file.path().filename().string();
				v->push_back(_path + name);
				std::string pathname = _path + name;
				if (fs::is_directory(pathname)) {
					continue;
				}
			}
			return true;
		}

	};




	class Util {


	public:
		
		static bool FileRead(const std::string &file, std::string *body) {
			body->clear();
			std::ifstream in;
			in.open(file, std::ios::binary);
			if (in.is_open() == false) {
				std::cout << "files open error" << std::endl;
				return false;
			}

			uint64_t size = fs::file_size(file);
			body->resize(size);
			in.read(&(*body)[0], size);
			if (in.good() == false) {
				std::cout << "files read error" << std::endl;
				return false;
			}
			in.close();
			return true;
		}




		
		static bool FileWrite(const std::string &file, const std::string &body) {

			std::ofstream out;
			out.open(file, std::ios::binary);
			if (out.is_open() == false) {
				std::cout << "files open error" << std::endl;
				return false;
			}

			out.write(&body[0], body.size());
			if (out.good() == false) {
				std::cout << "files write error" << std::endl;
				return false;
			}
			out.close();
			return true;
		}



		static int Split(const std::string &str, const std::string &sp, std::vector<std::string> *v) {
			int count = 0;
			size_t pos, idx = 0;
			while (1) {
				pos = str.find(sp, idx);
				if (pos == std::string::npos) {
					break;
				}
				v->push_back(str.substr(idx, pos - idx));
				idx = pos + sp.size();
				count++;
			}
				
			
			
			if (idx < str.size()) {
				count++;
				v->push_back(str.substr(idx));

			}
			return count;
		
		}
	};





	class DataManage {
	private:
		std::string _path;//标记符文件路径
		std::unordered_map<std::string, std::string> _map;
	public:
		DataManage(const std::string &path)
			:_path(path)
		{}

		bool Read() {
			std::string body;
			std::vector<std::string> v;
			if (Util::FileRead(_path, &body) == false) {
				std::cout << "read data error!" << std::endl;
				return false;
			}
			Util::Split(body, "\n", &v);//按行分割文件名和对应的大小时间信息
			for (auto& line : v) {
				std::vector<std::string> kv;
				Util::Split(line, "=", &kv);
				_map[kv[0]] = kv[1];
			}
			return true;
		}


		bool Write() {//将map的内容读到磁盘永久存储
			std::stringstream ss;
			for (auto & mp : _map) {
				ss << mp.first << "=" << mp.second << std::endl;
			}

			if (Util::FileWrite(_path, ss.str()) == false) {
				std::cout << "write data error;" << std::endl;
				return false;
			}
			return true;
		}



		bool Exists(const std::string &key) {
			auto e = _map.find(key);//返回的是map的迭代器
			if (e == _map.end()) {
				return false;
			}
			return true;
		}


		bool AddorChange(const std::string &key, const std::string &val) {
			
			_map[key] = val;
			return true;
		}


		bool Delete(const std::string &key, const std::string &val) {
			if (!Exists(key)) {
				std::cout << "key no exists" << std::endl;
				return false;
			}
			auto it = _map.find(key);
			_map.erase(it);
			return true;
		}

		

		bool Get(const std::string &key, std::string *val) {
			auto it = _map.find(key);
			if (it == _map.end()) {
				std::cout << "key no exists" << std::endl;
				return false;
			}
			*val = _map[key];
			return true;
		}
	};

#define LISTEN_DIR  "./scandir" 
#define CONFIG_FILE  "./data.conf"

	class Client {
	private:
		httplib::Client* _client;
		DataManage _data;
		ScanDir _scan;	

	public:
		Client(const std::string &host, int port)
			:_scan(LISTEN_DIR) 
			, _data(CONFIG_FILE)
		{
			_client = new httplib::Client(host, port);
		}

		//根据路径获取标识信息
		std::string GetIdent(const std::string &path) {
			uint64_t mtime, fsize;
			fsize = fs::file_size(path);
			auto time_type = std::experimental::filesystem::last_write_time(path);
			mtime = decltype(time_type)::clock::to_time_t(time_type);
			std::stringstream ss;
			ss << fsize << mtime;
			return ss.str();
		}

		
		bool Scan(std::vector<std::pair<std::string, std::string>>* arry) {
			std::vector<std::string> files;
			_scan.Scan(&files);
			for (auto &file : files) {
				std::string ident = GetIdent(file);
				if (_data.Exists(file) == false) {
					arry->push_back(std::make_pair(file, ident));
					continue;
				}
				std::string old;
				_data.Get(file, &old);
				if (old == ident) {
					continue;
				}
				arry->push_back(std::make_pair(file, ident));
			}
			return true;
		}

		//根据文件名上传信息
		bool Upload(const std::string &path) {
			httplib::MultipartFormData file;
			httplib::MultipartFormDataItems items;
			file.name = "file";
			file.content_type = "application/octet-stream";
			fs::path p(path);
			file.filename = p.filename().string();
			Util::FileRead(path, &file.content);

			items.push_back(file);
			auto rsp=_client->Post("/multipart", items);
			if (rsp && rsp->status == 200) {
				std::cout << "upload sucess!\n";
				return true;
			}
			std::cout << "upload error!\n";
			return false;
		}

		bool Start() {
			_data.Read();
			while (1) {
				std::vector<std::pair<std::string, std::string>> arry;
				Scan(&arry);		
				for (auto & file : arry) {		
					std::cout << file.first << "need upload\n";
					if (Upload(file.first) == false) {	
						continue;
					}

					_data.AddorChange(file.first, file.second);
					_data.Write();
				}
				Sleep(1000);
			}
			return true;
		}
	};
}
