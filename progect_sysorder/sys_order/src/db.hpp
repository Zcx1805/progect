#include<iostream>
#include<mutex>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
#include<string>
//using namespace std;
namespace order_system{
#define MYSQL_SERVER "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PASSWD ""
#define MYSQL_DBNAME "system_order"



  static MYSQL* MysqlInit(){
    MYSQL* mysql=NULL;
    mysql=mysql_init(NULL);
    if(mysql==NULL){
      std::cout<<"mysql init error"<<std::endl;
      return NULL;
    }

    if(mysql_real_connect(mysql,MYSQL_SERVER,MYSQL_USER,MYSQL_PASSWD,MYSQL_DBNAME,0,NULL,0)==NULL){
      std::cout<<mysql_error(mysql)<<std::endl;
      return NULL;
    }


    if(mysql_set_character_set(mysql,"utf8")!=0){
      std::cout<<mysql_error(mysql)<<std::endl;
      return NULL;
    }


    //mysql_select_db(mysql,MYSQL_DBNAME);
    return mysql;
  }




  static void MysqlRelease(MYSQL* mysql){
    if(mysql!=NULL){
      mysql_close(mysql);
    }
    return;
  }



  static bool MysqlQuery(MYSQL* mysql,const std::string &sql){
    if(mysql_query(mysql,sql.c_str())!=0){
      std::cout<<sql<<std::endl;
      std::cout<<mysql_error(mysql)<<std::endl;
      return false;
    }
    return true;
  }
  


  class TableDish{
    private:
      MYSQL *_mysql;
      std::mutex _mutex;


    public:
      TableDish(){
          _mysql=MysqlInit();
          if(_mysql==NULL){
            exit(-1);//构造函数不能判

        }
      }
        ~TableDish(){
          MysqlRelease(_mysql);
          _mysql=NULL;//删除后置空避免使用
        }
        //插入需要知道全部信息，所以参数为value对象
        bool Insert(const Json::Value &dish)
        {
          //组织sql语句
#define DISH_INSERT "insert table_dish values(null,'%s',%d,now());"
          char str_sql[4096]={0};
          sprintf(str_sql,DISH_INSERT,
              dish["name"].asCString(),
              dish["price"].asInt());

          return MysqlQuery(_mysql,str_sql);
        }





        //删除仅仅依靠id
        bool Delete(int dish_id){
#define DISH_DELETE "delete from table_dish where id=%d;"
          char str_sql[4096]={0};
          sprintf(str_sql,DISH_DELETE,dish_id);
          return MysqlQuery(_mysql,str_sql);
        }




        //修改需要知道全部信息和id,id包含在value对象里
        bool Update(const Json::Value &dish){
#define DISH_UPDATE "update table_dish set name='%s',price=%d where id=%d;"
          char str_sql[4096]={0};
          sprintf(str_sql,DISH_UPDATE,
              dish["name"].asCString(),
              dish["price"].asInt(),
              dish["id"].asInt());
          return MysqlQuery(_mysql,str_sql);

        }




        //选择需要返回反序列化的结果，保存在value对象
        bool SelectAll(Json::Value *dishes){
#define DISH_SELECTALL "select * from table_dish;"

          _mutex.lock();
          bool ret=MysqlQuery(_mysql,DISH_SELECTALL);
          if(ret==false){
            _mutex.unlock();
            return false;
          }
          MYSQL_RES *res=mysql_store_result(_mysql);
          _mutex.unlock();
          if(res==NULL){
            std::cout<<"store result error"<<std::endl;
            return false;
          }

          int num=mysql_num_rows(res);
          for(int i=0;i<num;++i){
            MYSQL_ROW row =mysql_fetch_row(res);
            Json::Value dish;
            dish["id"]=std::stoi(row[0]);
            dish["name"]=row[1];
            dish["price"]=std::stoi(row[2]);
            dish["ctime"]=row[3];

            //把菜品信息当做数组元素传入dishes
            dishes->append(dish);
          }
          mysql_free_result(res);
          return true;
        }





        bool SelectOne(int dish_id,Json::Value *dish){
#define DISH_SELECTONE "select * from table_dish where id=%d;"
          char str_sql[4096]={0};
          sprintf(str_sql,DISH_SELECTONE,dish_id);
          _mutex.lock();
          bool ret=MysqlQuery(_mysql,str_sql);
          if(ret==false){
            _mutex.unlock();
            return false;
          }

          MYSQL_RES* res=mysql_store_result(_mysql);
          _mutex.unlock();
          if(res==NULL){
            std::cout<<"store result failed!"<<std::endl;
            return false;
          }
          int num=mysql_num_rows(res);
          if(num!=1){
            std::cout<<"result error\n";
            mysql_free_result(res);
            return false;
          }

          MYSQL_ROW row=mysql_fetch_row(res);
          (*dish)["id"]=dish_id;
          (*dish)["name"]=row[1];
          (*dish)["price"]=std::stoi(row[2]);
          (*dish)["ctime"]=row[3];
          mysql_free_result(res);
          return true;

        }
      };







      class TableOrder{
        private:
          MYSQL *_mysql;
          std::mutex _mutex;


        public:
          TableOrder(){
            _mysql=MysqlInit();
          }
          ~TableOrder(){
            MysqlRelease(_mysql);
          }
          //插入需要知道全部信息，所以参数为value对象
          bool Insert(const Json::Value &order){

            //组织sql语句
#define RODER_INSERT "insert table_order values(null,'%s',0,now());"
            char str_sql[4096]={0};
            Json::FastWriter writer;
            std::string dishes=writer.write(order["dishes"]);
            
            dishes[dishes.size()-1]='\0';
            sprintf(str_sql,RODER_INSERT,dishes.c_str());
            return MysqlQuery(_mysql,str_sql);
          }



          //删除仅仅依靠id
          bool Delete(int order_id){
#define ORDER_DELETE "delete from table_order where id=%d;"
            char str_sql[4096]={0};
            sprintf(str_sql,ORDER_DELETE,order_id);
            return MysqlQuery(_mysql,str_sql);
          }

          //修改需要知道全部信息和id,id包含在value对象里
          bool Update(const Json::Value &order){
#define ORDER_UPDATE "update table_order set dishes='%s',status=%d where id=%d;"
            Json::FastWriter writer;
            int order_id=order["id"].asInt();
            int status=order["status"].asInt();

            std::string dishes=writer.write(order["dishes"]);
            char sqlstr[4096]={0};
            sprintf(sqlstr,ORDER_UPDATE,dishes.c_str(),status,order_id);
            return MysqlQuery(_mysql,sqlstr);
          }



          //选择需要返回反序列化的结果，保存在value对象
          bool SelectAll(Json::Value *orders){
#define ORDER_SELECTALL "select * from table_order;"
            _mutex.lock();
            bool ret=MysqlQuery(_mysql,ORDER_SELECTALL);
            if(ret==false){
              _mutex.unlock();
              return false;
            }
            MYSQL_RES * res=mysql_store_result(_mysql);
            _mutex.unlock();
            if(res==NULL){
              std::cout<<mysql_error(_mysql)<<std::endl;
              return false;
            }
            int num=mysql_num_rows(res);
            for(int i=0;i<num;++i){
              MYSQL_ROW row = mysql_fetch_row(res);
              Json::Value order;
              order["id"]=std::stoi(row[0]);
              Json::Reader reader;
              reader.parse(row[1],order["dishes"]);
              //  order["dishes"]=row[1];
              order["status"]=std::stoi(row[2]);
              order["mtime"]=row[3];
              orders->append(order);
            }
            mysql_free_result(res);
            return true;
          }



          bool SelectOne(int order_id,Json::Value *order){
#define ORDER_SELECTONE "select * from table_order where id=%d;"
            char sqlstr[4096]={0};
            sprintf(sqlstr,ORDER_SELECTONE,order_id);

            _mutex.lock();
            bool ret=MysqlQuery(_mysql,sqlstr);
            if(ret==false){
              _mutex.unlock();
              return false;
            }
            MYSQL_RES * res=mysql_store_result(_mysql);
            _mutex.unlock();
            if(res==NULL){
              std::cout<<mysql_error(_mysql)<<std::endl;
              return false;
            }
            int num=mysql_num_rows(res);
            if(num!=1){
              std::cout<<"ont result error\n";
              mysql_free_result(res);
              return false;
            }
            MYSQL_ROW row =mysql_fetch_row(res);
            (*order)["id"]=order_id;
            //(*order)["dishes"]=row[1];
              Json::Reader reader;
              reader.parse(row[1],(*order)["dishes"]);
            (*order)["status"]=std::stoi(row[2]);
            (*order)["mtime"]=row[3];

            mysql_free_result(res);
            return true;
          }
      };
  }
