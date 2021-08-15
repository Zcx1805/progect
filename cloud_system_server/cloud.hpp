#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<unordered_map>
#include"httplib.h"
#include<experimental/filesystem>
#include<unistd.h>
#include"bundle.h"
#include<sys/stat.h>


namespace cloud_sys{
  namespace fs=std::experimental::filesystem;
  
  
  class ScanDir{
    private:
      std::string _path;
    public:
      ScanDir(const std::string &path)
        :_path(path)
      {
        if(!fs::exists(path)){
            fs::create_directories(path);
        }
        if(_path.back()!='/'){
            _path+='/';
        }
      }



      bool Scan(std::vector<std::string> *v){
        for(auto &file:fs::directory_iterator(_path)){
          std::string name;
          name=file.path().filename().string();
          v->push_back(_path+name);
        }
        return true;
      }

  };



//通过路径信息获取
      class Util{
        private:
          std::string _path;
        
        public:
          static bool RangeRead(const std::string &file,std::string *body,int *start,int *end){
            body->clear();
            std::ifstream in;
            in.open(file,std::ios::binary);
            if(in.is_open()==false){
              std::cout<<"open file error!\n";
              return false;
            }

            uint64_t fsize=fs::file_size(file);
            uint64_t rlen;
            if(*end==-1){
              *end=fsize-1;
            }
            rlen=*end-*start+1;//0-1就是读2个
            std::cout<<fsize<<";"<<*start<<";"<<*end<<std::endl;
            std::cout<<rlen<<std::endl;
            in.seekg(*start,std::ios::beg);
            body->resize(rlen);//开辟范围
            in.read(&(*body)[0],rlen);
            if(in.good()==false){
              std::cout<<"read file error!\n";
              return false;
            }
            in.close();
            return true;
          }

//通过文件名获取文件内容
          static bool FileRead(const std::string &file,std::string *body){
            body->clear();
            std::ifstream in;
            in.open(file,std::ios::binary);
            if(in.is_open()==false){
              std::cout<<"files open error"<<std::endl;
              return false;
            }

            uint64_t size=fs::file_size(file);
            body->resize(size);
            in.read(&(*body)[0],size);//字符串首地址，c_str是const的，无法修改
            if(in.good()==false){
              std::cout<<"files read error"<<std::endl;
              return false;
            }
            in.close();
            return true;
          }




//字符串写入文件
          static bool FileWrite(const std::string &file,const std::string &body){
            
            std::ofstream out;
            out.open(file,std::ios::binary);
            if(out.is_open()==false){
              std::cout<<"files open error"<<std::endl;
              return false;
            }

            out.write(&body[0],body.size());//数据写入文件
            if(out.good()==false){
              std::cout<<"files write error"<<std::endl;
              return false;
            }
            out.close();
            return true;
          }


//将str以sp分割，存储到vector里
          static int Split(const std::string &str,const std::string &sp,std::vector<std::string> *v){
            int count=0;
            size_t pos,idx=0;//pos为找到分隔符的位置，idx为起始位置
            while(1){
              pos=str.find(sp,idx);//从idx位置开始找sp子串
              if(pos==std::string::npos)
                break;
              v->push_back(str.substr(idx,pos-idx));//从分割的位置到起始的位置个刚刚好
              idx=pos+sp.size();//更新起始位置
              ++count;

            }
            //最后一个字符串后面没有分割，需要单独加入
            if(idx<str.size()){ 
              v->push_back(str.substr(idx));
              ++count;
            }
            return count;
          }
      };




      class DataManage{
        private:
          std::string _path;
          pthread_rwlock_t _rwlock;
          std::unordered_map<std::string,std::string> _map;
        public:

          DataManage(const std::string &path)
            :_path(path)
          {
            pthread_rwlock_init(&_rwlock,NULL);
          }

          ~DataManage()
          {
                    pthread_rwlock_destroy(&_rwlock);
                          
          }

          bool Read(){
            std::string body;
            std::vector<std::string> v;
            if(Util::FileRead(_path,&body)==false)
              std::cout<<"read data error!"<<std::endl;
              return false;
    
              Util::Split(body,"\n",&v);
              for(auto& line:v){
                std::vector<std::string> kv;
                Util::Split(line,"=",&kv);
                pthread_rwlock_wrlock(&_rwlock);
                _map[kv[0]]=kv[1];
                pthread_rwlock_unlock(&_rwlock);
              }
              return true;
          }


          bool Write(){
            std::stringstream ss;
            pthread_rwlock_rdlock(&_rwlock);
            for(auto & mp:_map){
              ss<<mp.first<<"="<<mp.second<<std::endl;
            }
            pthread_rwlock_unlock(&_rwlock);

            if(Util::FileWrite(_path,ss.str())==false){
              std::cout<<"write data error;"<<std::endl;
              return false;
            }
            return true;
          }
        

//从内存中通过key查找val
          bool Exists(const std::string &key){
            pthread_rwlock_rdlock(&_rwlock);
            auto e=_map.find(key);
            if(e==_map.end()){
              return false;
            }
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }


          bool AddorChange(const std::string &key,const std::string &val){
            pthread_rwlock_wrlock(&_rwlock);
            _map[key]=val;
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }


          bool Delete(const std::string &key,const std::string &val){
            pthread_rwlock_wrlock(&_rwlock);
            if(!Exists(key)){
              std::cout<<"key no exists"<<std::endl;
              return false;
            }
            auto it=_map.find(key);
            _map.erase(it);
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }

//通过文件路径获取时间大小信息
          bool Get(const std::string &key,std::string *val){
           pthread_rwlock_rdlock(&_rwlock);
            auto it=_map.find(key);
            if(it==_map.end()){
              std::cout<<"key no exists"<<std::endl;
              return false;
            }
            *val=_map[key];
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }
//获取全部文件名
          bool GetAllName(std::vector<std::string> *arry){
            arry->clear();
            pthread_rwlock_rdlock(&_rwlock);
            for(auto& file:_map){
              arry->push_back(file.first);
            }
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }

      };

//压缩解压缩
#define PACK_PATH "./packdir/"
      class Compress{
        public:
          static bool Pack(const std::string &filename,const std::string &packname){
            if(fs::exists(PACK_PATH)==false){//创建压缩文件
              fs::create_directories(PACK_PATH);
            }
            std::string body;
            Util::FileRead(filename,&body);
            Util::FileWrite(packname,bundle::pack(bundle::LZIP,body));//写到新的文件
            return true;
          }


          static bool UnPack(const std::string &packname,const std::string &filename){
            std::string body;
            Util::FileRead(packname,&body);
            Util::FileWrite(filename,bundle::unpack(body));
            return true;
          }
      };




#define BACKUP_PATH  "./packup/"
#define CONFIG_PATH "./backup.conf"
      DataManage g_data(CONFIG_PATH);
      class Server{
        private:
          httplib::Server _srv;
        public:
          static void Upload(const httplib::Request &req,httplib::Response &rsp){
            std::cout<<"into upload\n";
            auto ret=req.has_file("file");//判断是否存在对应name的信息
            if(ret==false){
              std::cout<<"have no upload file\n";
              rsp.status=400;
              return ;
            }
            if(fs::exists(BACKUP_PATH)==false){
              fs::create_directories(BACKUP_PATH);
            }
            const auto& file=req.get_file_value("file");
            std::string filename=BACKUP_PATH+file.filename;
            if(Util::FileWrite(filename,file.content)==false){
              std::cout<<"write file error\n";
              rsp.status=500;
              return ; 
            }
            g_data.AddorChange(file.filename,file.filename);
            g_data.Write();
            return ;
          }


          static void List(const httplib::Request &req,httplib::Response &rsp){
            std::stringstream ss;
            ss<<"<html><head><meta http-equiv='content-type' content='text/html;charset=utf-8'>";
            ss<<"</head><body>";
            std::vector<std::string> arry;
            g_data.GetAllName(&arry);
            for(auto& filename:arry){
              ss<<"<hr />";
              ss<<"<a href='/backup/"<<filename<<"'><strong>"<<filename<<"</strong></a>";
            }
            ss<<"<hr /></body></html>";
            rsp.body=ss.str();
            rsp.set_header("Content-Type","text/html");
            return;
          }

          static std::string GetIdentfiler(const std::string &path){
            uint64_t mtime,fsize;
            fsize=fs::file_size(path);
            auto time_type =std::experimental::filesystem::last_write_time(path);
            mtime=decltype(time_type)::clock::to_time_t(time_type);
            std::stringstream ss;
            ss<<fsize<<mtime;
            return ss.str();

          }
  

          static void Download(const httplib::Request &req,httplib::Response &rsp){
            std::string name=req.matches[1];
            std::string pathname=BACKUP_PATH+name;
            std::string newetag=GetIdentfiler(pathname);
            uint64_t fsize=fs::file_size(pathname);

            if(g_data.Exists(name)){
              std::string realname;
              g_data.Get(name,&realname);
              if(name!=realname){//如果不相同证明被压缩了
                Compress::UnPack(realname,pathname);//解压缩
                
                unlink(realname.c_str());//删除压缩的内容
                g_data.AddorChange(name,name);
                g_data.Write();
              }
            }

            
            if(req.has_header("If-Range")){断点续传
              std::string oldetag=req.get_header_value("If-Range");
              if(oldetag==newetag){
                std::cout<<req.ranges[0].first<<"-"<<req.ranges[0].second<<std::endl;
                
                std::cout<<"oldetag==newetag,range download\n";
                int start=req.ranges[0].first;
                int end=req.ranges[0].second;
                Util::RangeRead(pathname,&rsp.body,&start,&end);
                rsp.set_header("Content-Type","application/octet-stream");
                rsp.set_header("ETag",newetag);
                std::stringstream ss;
                ss<<"bytes "<<start<<"-"<<end<<"/"<<fsize;
                std::cout<<ss.str()<<std::endl;
                rsp.set_header("Content-Range",ss.str());
                rsp.status=206;
                return ;
              }
            }
          
            if(Util::FileRead(pathname,&rsp.body)==false){
              std::cout<<"read file "<<pathname<<"failed\n";
              rsp.status=500;
              return ;
            }

            
            rsp.set_header("Content-Type","application/octet-stream");
            rsp.set_header("Accept-Ranges","bytes");
            rsp.set_header("ETag",newetag);
            rsp.status=200;
            return;
          }
        public:
          bool Start(int port=9000){
            g_data.Read();
            _srv.Post("/multipart",Upload);
            _srv.Get("/list",List);
            _srv.Get("/backup/(.*)",Download);
            _srv.listen("0.0.0.0",port);
            return true;
          }

      };

//整合功能模块，负责对云盘的文件热点判断和压缩问题
      class FileManager{
        private:
          ScanDir _scan;//用于遍历上传的路径，进行后续的热点判断和压缩
          time_t _hot_time =60;//热点时间

        public:
          time_t LastAccessTime(const std::string &filename){
            struct stat st;
            stat(filename.c_str(),&st);
            return st.st_atime;
          }

        
          FileManager()
            :_scan(BACKUP_PATH){}

          bool Start(){
            while(1){
            std::vector<std::string> arry;
            _scan.Scan(&arry);
            for(auto& file:arry){
              time_t atime =LastAccessTime(file);
              time_t ctime =time(NULL);
              if((ctime-atime)>_hot_time){
                fs::path fpath(file);
                std::string pack=PACK_PATH+fpath.filename().string()+".pack";
              
                Compress::Pack(file,pack);
                unlink(file.c_str());//删除源文件
                
                g_data.AddorChange(fpath.filename().string(),fpath.filename().string()+".pack");
                g_data.Write();
              }

            }
            usleep(1000);
            }
            return true;
          }

      };







}
