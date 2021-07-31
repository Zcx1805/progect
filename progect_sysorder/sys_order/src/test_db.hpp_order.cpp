#include"db.hpp"

void insert_order(){
  order_system::TableOrder order;
  Json::Value val;
  val["dishes"].append(1);
  val["dishes"].append(2);
  val["dishes"].append(3);
  order.Insert(val);
}

void delete_order(){
  order_system::TableOrder order;

  order.Delete(1);
}

void update_order(){
  order_system::TableOrder order;
  Json::Value val;
  val["dishes"].append(6);
  val["id"]=8;
  order.Update(val);
}

void select_all(){
  order_system::TableOrder order;
  Json::Value orders;
  order.SelectAll(&orders);
  std::cout<<orders<<std::endl;
}

void select_one(){
  order_system::TableOrder order;
  Json::Value orders;
  order.SelectOne(9,&orders);
  std::cout<<orders<<std::endl;

}

int main(){
  //insert_order();
  //delete_order();
  //update_order();
  //select_all();
  select_one();
  return 0;
}
