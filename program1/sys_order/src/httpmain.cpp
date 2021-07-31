#include"db.hpp"
#include"httplib.h"
#define WWWROOT "./wwwroot"
using namespace httplib;

order_system::TableDish *table_dish=NULL;
order_system::TableOrder *table_order=NULL;

void DishInsert(const Request &req,Response &rsp){
  Json::Value dish;
  Json::Reader reader;
  bool ret=reader.parse(req.body,dish);
  if(ret==false){
    rsp.status=400;
    Json::Value reason;
    Json::FastWriter writer;
    reason["result"]=false;
    reason["reason"]="dish info parse failed!";
    rsp.body=writer.write(reason);
    rsp.set_header("Content-Type","application/json");
    return;
  }
  ret=table_dish->Insert(dish);
  if(ret==false){
    rsp.status=500;
    Json::Value reason;
    Json::FastWriter writer;
    reason["result"]=false;
    reason["reason"]="mysql insert failed!";
    rsp.body=writer.write(reason);
    rsp.set_header("Content-Type","application/json");
    return;
  }
  rsp.status=200;
  return;
}

void DishDelete(const Request &req,Response &rsp){
  int dish_id=std::stoi(req.matches[1]);
  bool ret=table_dish->Delete(dish_id);
  if(ret==false){
    std::cout<<"mysql delete error\n";
    rsp.status=500;
    return;
  }
  return;
}

void DishUpdate(const Request &req,Response &rsp){
  int dish_id =std::stoi(req.matches[1]);
  Json::Value dish;
  Json::Reader reader;
  bool ret=reader.parse(req.body,dish);
  if(ret==false){
    rsp.status=400;
    std::cout<<"uptade parse dish info error\n";
    return;
  }

  dish["id"]=dish_id;
  ret=table_dish->Update(dish);
  if(ret==false){
    rsp.status=500;
    std::cout<<"uptade sql error\n";
    return;

  }
  return ;
}

void DishGetAll(const Request &req,Response &rsp){
  Json::Value dishes;
  bool ret=table_dish->SelectAll(&dishes);
  if(ret==false){
    rsp.status=500;
    std::cout<<"get all sql error\n";
    return;
  }
  Json::FastWriter writer;
  rsp.body=writer.write(dishes);
  //rsp.set_header("Content-Type","application/json");
  return;
}

void DishGetOne(const Request &req,Response &rsp){
  Json::Value dish;
  int dish_id =std::stoi(req.matches[1]);
  bool ret=table_dish->SelectOne(dish_id,&dish);
  if(ret==false){
    rsp.status=500;
    std::cout<<"get one sql error\n";
    return;
  }
  Json::FastWriter writer;
  rsp.body=writer.write(dish);
  return;
}

void OrderInsert(const Request &req,Response &rsp){
  Json::Value order;
  Json::Reader reader;
  bool ret=reader.parse(req.body,order);
  if(ret==false){
    rsp.status=400;
    std::cout<<"orderinsert parse error\n";
    return;
  }
  ret=table_order->Insert(order);
  if(ret==false){
    rsp.status=500;
    Json::FastWriter writer;
    Json::Value reason;
    reason["result"]=false;
    reason["reason"]="mysql insert false";
    rsp.body=writer.write(reason);
    rsp.set_header("Content-Type","application/json");
    
    return;
  }
return;
}


void OrderDelete(const Request &req,Response &rsp){
  int order_id=std::stoi(req.matches[1]);
  bool ret=table_order->Delete(order_id);
  if(ret==false){
    std::cout<<"mysql delete error\n";
    rsp.status=500;
    return;
  }
  return;

}

void OrderUpdate(const Request &req,Response &rsp){
  int order_id =std::stoi(req.matches[1]);
  Json::Value order;
  Json::Reader reader;
  bool ret=reader.parse(req.body,order);
  if(ret==false){
    rsp.status=400;
    std::cout<<"uptade parse order info error\n";
    return;
  }

  order["id"]=order_id;
  ret=table_order->Update(order);
  if(ret==false){
    rsp.status=500;
    std::cout<<"uptade sql error\n";
    return;

  }
  return ;

}
void OrderGetAll(const Request &req,Response &rsp){
  Json::Value orderes;
  bool ret=table_order->SelectAll(&orderes);
  if(ret==false){
    rsp.status=500;
    std::cout<<"get all sql error\n";
    return;
  }
  Json::FastWriter writer;
  rsp.body=writer.write(orderes);
  return;
}

void OrderGetOne(const Request &req,Response &rsp){
  int order_id =std::stoi(req.matches[1]);
  Json::Value orderes;
  bool ret=table_order->SelectOne(order_id,&orderes);
  if(ret==false){
    rsp.status=500;
    std::cout<<"get all sql error\n";
    return;
  }
  Json::FastWriter writer;
  rsp.body=writer.write(orderes);
  return;
}

int main(){
  table_dish=new order_system:: TableDish();
  table_order=new order_system:: TableOrder();
  Server server;
  server.set_base_dir(WWWROOT);
  server.Post("/dish",DishInsert);
  server.Delete(R"(/dish/(\d+))",DishDelete);
  server.Put(R"(/dish/(\d+))",DishUpdate);
  server.Get("/dish",DishGetAll);
  server.Get(R"(/dish/(\d+))",DishGetOne);


  server.Post("/order",OrderInsert);
  server.Delete(R"(/order/(\d+))",OrderDelete);
  server.Put(R"(/order/(\d+))",OrderUpdate);
  server.Get("/order",OrderGetAll);
  server.Get(R"(/order/(\d+))",OrderGetOne);

  server.listen("0.0.0.0",9898);
  return 0;
}
