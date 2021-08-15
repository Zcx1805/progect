#include<thread>
#include "cloud.hpp"
void thread_start(){
  cloud_sys::FileManager fm;
  fm.Start();
}
int main(){
  std::thread scan_thr(thread_start);
  scan_thr.detach();
  
  cloud_sys::Server srv;
  srv.Start();
  return 0;
}
