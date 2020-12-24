# horsedb
HorseDB是一款以rocksdb作为存储引擎的数据库,兼容了Mysql协议,适合写多读少场景。

支持单机模式和raft模式,raft模式是基于自研的raft共识库[horse-raft](https://github.com/hezhihua/horse-raft)实现

# 依赖环境
| 软件	 | 要求 |
| ----- | ----- |
| gcc版本 | 需要支持c++14,最好7或以上 |
| cmake版本 | 3.10及以上版本 |
| horse-rpc版本 |  最新版本 |
| horse-raft版本 |  最新版本 |
| iguana版本 | 最新版本 |
| snappy | 1.1.7 |
# 特性
1, 支持Mysql协议  
2, 支持raft一致性协议多副本备份   
3, 适合写多读少场景  



# 编译和安装  
如果gcc 版本太旧,先升级gcc版本 到7.3    
1,yum -y install centos-release-scl     
2,yum -y install devtoolset-7-gcc devtoolset-7-gcc-c++ devtoolset-7-binutils   
3,scl enable devtoolset-7 bash  
scl命令启用只是临时的，退出shell或重启就会恢复原系统gcc版本,所以这种方式不会污染原来的编译环境。  
如果要长期使用gcc 7.3的话,执行：
echo "source /opt/rh/devtoolset-7/enable" >>/etc/profile   

最好是用拥有root权限的用户安装horsedb，以下为centos环境下安装步骤  
1,yum -y install epel-release && yum -y install libzstd-devel  
2,yum -y install lz4 && yum -y install lz4-devel  
3,git clone https://github.com/hezhihua/horsedb.git  
4,cd horsedb && mkdir build && cd build && cmake ..  && make 

 安装过程会下载一些依赖库,如果网络的问题下载失败导致安装失败,请继续执行make命令安装。  
 有问题可以加QQ群咨询。
# 用法  

安装完horsedb后  
1,单机模式启动horsedb  
  ./horsedb-server ./config/server.cfg.yaml    
  
2,raft模式启动horsedb,至少三节点   
  设置配置文件server.cfg.yaml里面的raft_enable选项为 true,不同的实例需要配置不同的bind_mysql和local_node地址,  
  如果在同一机器运行实例,需要在不同的目录运行,以防rocksdb 的db目录冲突
  
3,启动成功后，可以用Mysql客户端直接连接登录到leader所在机器的horsedb ,如   
mysql  -u hzh -h 0.0.0.0  -P 8083 -p'hzh' test   

# 创建库  
create database dbtest  
如果不用命令行创建库,也可以在配置文件里面的dbname选项指定数据名,程序会自动生成数据库,多个数据库以逗号分隔(如:a,b,c)  
  
# 创建表
  
例子:  
CREATE TABLE students (myid int ,stname varchar(50), st_number int, city varchar(50), grade DOUBLE not null,  PRIMARY KEY (myid),KEY mykeyname(stname))	;	  
下面建表语句没有指定主键,horsedb会为表默认生成一个名为id的主键：  
CREATE TABLE teachers (tcname varchar(50), tc_number int, city varchar(50), grade DOUBLE not null,  KEY mykeyname(tcname));  
CREATE TABLE teachers (tcname varchar(50), tc_number int, city varchar(50), grade DOUBLE not null, UNIQUE KEY mykeyname(tcname));   

# 插入数据 
例子:  
use dbtest  ;  
insert into teachers(tcname,tc_number,city) values('xmtc',12,'beijing');  

# 查询  
例子:  
select * from teachers;    
select * from teachers where tcname='xmtc';  
show databases;  
show tables;  

# 二期   
1,delete、update语法,group by等语法  
2,事务实现  
3,支持redis协议  

# 感谢
1,horsedb底层用[RocksDB](https://github.com/facebook/rocksdb)作为存储引擎  
2,列和索引等数据的组织结构参考了[tidb](https://github.com/pingcap/tidb)的设计理念  

# 学习和交流
QQ群:1124085420  
# 开源不易，如果喜欢本项目，请给我点个赞！  

