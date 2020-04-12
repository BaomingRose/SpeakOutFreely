# SpeakOutFreely
### 项目简介
该项目是一个群聊娱乐系统，用户只要注册登录就可以在这个大群中畅所欲言，用户可以分享自己的心情，可以提出自己的问题，可以发表自己的观点……
### 总体流程
用户想要加入群聊需要登录账号和密码，如果没有账号则需要注册，登录验证使用了更安全的tcp协议，客户端向服务端发起注册或者登录请求，服务端接收客户端的注册或者登录信息。如果是注册，检查提交的注册信息回复客户端一个账号；如果是登录，验证登录的账号密码是否正确，正确则可以开始聊天。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200408233340500.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
聊天内容的交流是通过udp实现，客户端将聊天信息发送给服务端，服务端的接收线程接收数据将数据压入线程安全的数据池，然后服务端的发送线程将数据池的数据弹出发送给所有在线客户。


![在这里插入图片描述](https://img-blog.csdnimg.cn/20200408233300424.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
### 实现流程
##### 约定服务端客户端交互数据格式
首先，客户端选择登陆或者注册选项，那么客户端先给服务端发送一个字符，区分注册/登陆/注销。然后客户端发送对应的注册信息（昵称、职业、密码）/登录信息(账号、密码)，服务端统一回复用户的状态和账号。**状态的重要性**：用户登录/注册是否成功需要状态标识，服务端收发数据需要判断用户状态是否在收发数据的用户池中。
```cpp
struct ReginInfo {
    char _NickName[15];
    char _career[20];
    char _passWord[20];
};

struct LoginInfo {
    uint32_t _userID;
    char _passWord[20];
};

struct ReplyInfo {
    int _status;                                                                                            
    int _userID;
};
```
##### 服务端流程
服务端的实现过程分为tcp连接模块、处理登录注册模块、udp收发数据模块、用户管理模块，还有贯穿整个项目的日志模块。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411144948949.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
##### 客户端流程
客户端的实现就容易的多了，只要按照服务端的格式收发数据，创建收发数据的线程即可。另外保存自己的信息，并保存已经登录的发送者的昵称和职业。客户端在这里就不多说了，主要客户端使用了ncurses库做出了一个界面，后面会介绍。
### 实现中的问题、知识依赖、经验总结
##### 实现日志模块

为了实时发现流程的运行位置及错误位置、错误原因……，所以打印出的过程信息、错误信息发生的文件位置和时间是必要的。不但可以清晰看出程序运行的情况，还方便调式。
这就是打印的格式：

> [时间 info/warning/error/fatal/debug 文件 行号] 	"具体的错误信息"

以下为获取时间方法：使用了time函数localtime函数解析出普通时间格式
```cpp
void GetTimeStamp(std::string& timestamp) {    
        time_t SysTime;    
        //获取时间戳    
        time(&SysTime);    
    
        //将时间戳转化为年月日时分秒    
        struct tm* ST = localtime(&SysTime);    
        //格式化字符串 [YYYY-MM-DD HH-mm-SS]    
        char TimeNow[23] = {'\0'};    
        snprintf(TimeNow, sizeof(TimeNow) - 1, "%04d-%02d-%02d %02d:%02d:%02d", ST->tm_year + 1900, ST->tm_mon + 1, ST->tm_mday, ST->tm_hour, ST->tm_min, ST->tm_sec);        
        timestamp.assign(TimeNow, strlen(TimeNow));    
    }
```

```
__FILE__，在源代码中插入当前源代码行号； 
__LINE__，是在源文件中插入当前源文件名；
```

这里的**inline的必要性**：是要将代码展开，不然在其他地方调用展开的文件名和行号都是该函数的所在文件。这里的日志并没有打印到文件，而是打印到屏幕，打印到文件也是可以实现的。
```cpp
#define LOG(lev, msg) Log(lev, __FILE__, __LINE__, msg) 

inline void Log(LogLevel lev, const char* file, int line, const std::string& logmsg) {
    std::string level_info = Level[lev];
    std::string timer_stamp;

    GetTimeStamp(timer_stamp);

    std::cout << "[" << timer_stamp << " " << level_info << " " << file << ":" <<
        line << "]" << logmsg << std::endl;
}
```
##### 创建和客户端连接进行处理登录注册的线程如何传参
监听套接字后，在创建的线程中不仅要拿到和客户端一对一服务的套接字，同时还要管理用户池，这样就不能仅传入一个带有客户端地址信息的套接字。所以我在封装tcp类时也添加了一个成员（指针——指向服务类）。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200410184424763.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
##### 服务端怎样筛选聊天信息是有效用户发送的
在实现过程中遇见如下问题：
服务端需要判断传来的数据是否合理，不能不看是谁发送的就广播给所有用户
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200410185500644.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
解决：每次客户端发送聊天信息时，不仅发送聊天内容，也要包含用户信息，方便服务端进行检查，该用户是否在线，如果不在线则将该聊天内容视为废弃内容，直接丢掉。
因为发送给服务端客户信息和聊天内容绑在一起发送，用户信息长度可以按char[]规格限制，但是客户发送的信息长度并不确定，所有使用了json格式收发数据。
##### 该项目在udp传输，进行了json格式收发数据
JSON：JavaScript Object Notation(JavaScript 对象表示法)，是==存储和交换文本信息的语法==。它类似 XML，但比 XML 更小、更快，更易解析。
JSON 是轻量级的==文本数据交换格式==，独立于语言，具有自我描述性，更易理解。
JSON 文本格式在语法上与创建 JavaScript 对象的代码相同，由于这种相似性，无需解析器，JavaScript 程序能够使用内建的 eval() 函数，用 JSON 数据来生成原生的 JavaScript 对象。
##### 语法规则
- 数据在名称/值对中
- 数据由逗号分隔
- 大括号保存对象
- 中括号保存数组

```cpp
var peopel [
	{ "lastname":"张", "firstname":"三" },
	{ "lastname":"李", "firstname":"四" },
	{ "lastname":"小", "firstname":"萝莉" }
]
```
学习实例：
读取json文件源码：
```cpp
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <cassert>
#include <errno.h>
#include <string.h>
using namespace std;

int main(void) {
    ifstream ifs;
    ifs.open("test.json");
    assert(ifs.is_open());

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(ifs, root, false)) {
        cout << "reader parse error: " << strerror(errno) << endl;
        return -1;
    }
    string name;
    int age;
    int size;
    size = root.size();
    cout << "total " << size << " elements" << endl;
    for (int i = 0; i < size; ++i) {
        name = root[i]["name"].asString();
        age = root[i]["age"].asInt();

        cout << "name: " << name << ", age: " << age << endl;
    }
    return 0;
}
```
写json文件
```cpp
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <cassert>
#include <errno.h>
#include <string.h>
using namespace std;
int main(void) {
    ifstream ifs;
    ifs.open("test.json");
    assert(ifs.is_open());

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(ifs, root, false)) {
        cout << "reader parse error: " << strerror(errno) << endl;
        return -1;
    }

    string name;
    int age;
    int size;
    size = root.size();
    cout << "total " << size << " elements" << endl;
    for (int i = 0; i < size; ++i) {
        name = root[i]["name"].asString();
        age = root[i]["age"].asInt();
        cout << "name: " << name << ", age: " << age << endl;
    }
    return 0;
}
```

##### udp通信传输数据解析过程：
```cpp
class Message {
public:
    //反序列化客户端发送给服务端的json数据串
    void deserialize(string msg) {
        Json::Reader reader;
        Json::Value val;                                                                                    
        reader.parse(msg, val, false);

        _nickName = val["_nickName"].asString();
        _career = val["_career"].asString();
        _msg = val["_msg"].asString();
        _userId  = val["_userId"].asUInt();
    }
    
    void serialize(string& msg) {
        Json::Value val;
        val["_nicknme"] = _nickName;
        val["_career"] = _career;
        val["_msg"] = _msg;
        val["_userId"] = _userId;

        Json::FastWriter writer;
        msg = writer.write(val);
    } 
private:
    string _nickName;
    string _career;
    string _msg;
    uint32_t _userId;
};
```

##### 客户端界面——使用ncurses库
这是预期中的界面效果：
**将4个框分给4个线程独立运行**，聊天窗口的线程也负责接收数据，发送窗口的线程也负责发送数据。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411234150754.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
**学习使用ncurses库：**
ncurses窗口是curses系统定义的一个假想的屏幕，即屏幕逻辑，通常80列，24行，**因为这几个框要独立运行，所以他们共用一个假想的屏幕，所以要对这个屏幕加互斥锁。**
这也是笔者在完成这个项目受到的一个阻挠，因为没有锁，所以频繁打出离奇的屏幕。
几个常用的函数：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411234748777.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
项目中也用到下面的函数
```cpp
//将字符串打印到屏幕
mvwaddstr( WINDOW *, int, int, const char * );
//getmaxyx函数返回所求窗口最大的行数存储在变量y当中，返回最大的列数存储在变量x当中
getmaxyx( win, y, x );
//从屏幕上读字符串
wgetnstr( WINDOW *, char *, int );
```
### 调式过程中遇见的问题总概
1. 经过条件分支，有的分支退出忘记解锁。
2. 一个条件判断条件未写全，逻辑错误。
3. 开始加锁，经过大量代码隔开之后，忘记加过锁，二次加锁。
4. 创建线程传参错误，改定参数规范，创建线程的参数忘记修改。
5. json序列化的变量字符串键值是双引号引用，所以里面的单词写错，解析不正确，虽然不影响客户端和服务端的数据交互，但是这个马虎也是需要耐心核对字符串内容。
6. ncurses的屏幕刷新需要加锁，没有加锁，得到的结果符不符合预期要看运气，符合预期的可能性很低很低！

### 运行效果
这是客户端首次登陆的界面，其中标题是左右实时摆动的：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411203509620.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
客户注册登录验证：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411221756349.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
服务端日志：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411220915926.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
用户聊天界面：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20200411221552746.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzOTA5MTg0,size_16,color_FFFFFF,t_70)
