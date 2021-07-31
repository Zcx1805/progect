create database if not exists system_order;

use system_order;
   
create table if not exists table_dish(
id int primary key auto_increment,
name varchar(32) unique not null,
price int not null,
ctime datetime
);
     
create table if not exists table_order(
id int primary key auto_increment,
dishes varchar(255) comment '[1,2]',
status int comment'0-未完成 1-完成',
mtime datetime );
      
insert table_dish values(null,"板栗红烧肉",4900,now()),
(null,"辣椒炒肉",3900,now());
insert table_order values(null,"[1,2]",0,now()),
(null,"[1]",0,now());
