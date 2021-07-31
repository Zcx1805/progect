#include<iostream>
#include<string>
#include<jsoncpp/json/json.h>
using namespace std;
string seria(){

  const char* name="张三";
  int id=001;
  int score[3]={99,98,97};
  const char* sex="男";
  
  Json::Value val;
  val["姓名"]=name;
  val["学号"]=id;
  val["性别"]=sex;
  for(int i=0;i<3;++i){
    val["成绩"].append(score[i]);
  }
  Json::StyledWriter writer;
  cout<<writer.write(val["成绩"])<<endl;
  cout<<writer.write(val["姓名"])<<endl;
  cout<<writer.write(val["学号"])<<endl;
  cout<<endl;
  string str;
  
  str=writer.write(val);
  cout<<str<<endl;
  return str;
}

void unseria(string& str){
  Json::Value val;
  Json::Reader reader;
  bool ret=reader.parse(str,val);
  if(ret==false){
    cout<<"prase error;"<<endl;
    return;
  }
  //cout<<val["姓名"]<<endl;
  //cout<<val["学号"]<<endl;
  //cout<<val["性别"]<<endl;
  //cout<<val["成绩"]<<endl;
  
  cout<<val["姓名"].asString()<<endl;
  cout<<val["学号"].asInt()<<endl;
  cout<<val["性别"].asString()<<endl;
  int num=val["成绩"].size();
  for(int i=0;i<num;++i){
    cout<<val["成绩"][i].asInt()<<endl;
  }
  return;
}


int main(){
  string str=seria();
  unseria(str);
  return 0;
}
