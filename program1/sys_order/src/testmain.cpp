#include"db.hpp"


void dish_test(){
  order_system::TableDish dish;

  Json::Value val;
  val["name"]="宫保鸡";
  val["price"]=2400;

  dish.Insert(val);

}
void order_test(){
  order_system::TableOrder order;

Json::Value val;
val["dishes"].append(1);
val["dishes"].append(2);
val["dishes"].append(3);
order.Insert(val);
}
int main(){
  order_test();
  return 0;
}
