#include<iostream>
#include "httplib.h"
using namespace std;
void hello(const Request &req,httblib::Response &rsp){
  cout<<req.method<<endl;
  cout<<req.path<<endl;
  cout<<req.body<<endl;
  auto it=req.handers.begin();
  for(;it!=req.handers.end();++it){
    cout<<it->first<<"  "<<it->second<<endl;
  }
  auto it=req.params.begin();
  for(;it!=req.params.end();++it){
    cout<<it->first<<"  "<<it->second<<endl;
  }

  rsp.status=200;
  rsp.body="<html><body><h1>hello bit</h1></body></html>";
  rsp.set_header("Content-Type","text/html");
  return ;

}

int main(){
  httplib::Server srv;
  srv.Get("/hello",hello);
  srv.listen("0.0.0.0",8080);
  return 0;
}
