#include"db.hpp"
void insert_dish(){
  order_system::TableDish dish;

  Json::Value val;
  val["name"]="测试菜品1";
  val["price"]=100;

  dish.Insert(val);
}

void delete_dish(){
  order_system::TableDish dish;
  dish.Delete(9);
}

void update_dish(){
  order_system::TableDish dish;
  Json::Value val;
  val["name"]="测试菜品2";
  val["price"]=200;
  val["id"]=10;
  dish.Update(val);
}

void select_all_dish(){
  order_system::TableDish dish;
  Json::Value dishes;
  dish.SelectAll(&dishes);
  std::cout<<dishes<<std::endl;
}

void select_one_dish(){
  order_system::TableDish dish;
  Json::Value dish1;
  dish.SelectOne(1,&dish1);
  std::cout<<dish1;
  //Json::StyledWriter writer;
  //std::cout<<writer.write(dish["name"])<<std::endl;
  
}
int main(){
  //insert_dish();
  //delete_dish();
  //insert_dish();
  //update_dish();
  //select_all_dish();
  select_one_dish();
  return 0;
}
