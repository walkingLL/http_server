# 自动化跟踪算法性能测试系统设计

评价跟踪算法性能的优劣，我们往往需要使用大量的已标记的样本数据对算法进行测试，然后根据测试结果，经过必要分析后，给出一个算法的大致性能评价。然而，这一系列工作如果完全由人来操作进行，是非常复杂繁琐的。我们需要有一套自动化的测试环境，该测试环境能够帮助我们快速地完成上述测试工作，并给出评价结果，如此才能更好的解放我们的双手，去做更加有意义的工作。

# 系统结构

整个测试系统采用传统的 C/S 架构设计。执行算法的运算平台，作为 `Server` 端；而对算法进行**执行控制**，**数据输入**，以及**结果收集**等工作的控制逻辑单元，作为 `Client` 端。

![系统结构示意图](structure.jpg)

其中，`Client` 的实现方式，可以使一个可执行程序，或者是一系列脚本，通常运行在一台专门的 PC 或者服务器上。`Client` 要提供基本的人机接口，便于测试人员进行操作；而 `Server` 端，存在多种情况，原则上只要是感兴趣的算法执行平台，都可以作为 `Server` 端，包括 `DSP 平台`，`PC 平台`，或者其他运算平台。

# 通讯协议
由于 Server 端可能存在多种平台。为了便于开发与实现，我们采用 HTTP 作为我们的基础通信协议。HTTP 协议之上的业务层协议，通过定义相关 url 规范与 payload 规范来实现。

业务层的通讯协议，至少需要满足以下功能需求：

- 上传图片
图片的传输方式为JSON格式
```json
{
    "name" : "xxx.jpg"
    "data" : "<data>" //用base64编码
}




* 上传图片（用于算法初始化）；
* 获取算法执行结果；
* 获取当前算法基本信息；
* 获取当前算法配置信息；
* 修改算法配置；
* 获取运算平台运行日志；
* 罗列所有可配置，可获取的数据或资源列表。

## HTTP方法

HTTP常用两种方法`POST`和`GET`，根据我们的需求，约定
* `POST`用法发送数据，设置数据，指令，如发送图像数据，设置某个参数，控制算法执行等
* `GET`用于获取数据，比如获取日志，获取算法参数，获取算法中间结果等

设计尽可能简单，有以下约定，

`GET`方法不包含 URL 参数，如果某个方法需要参数，将参数作为url的一部分。比如（只是举例，列举 URL 可能与实际不匹配），获取第1条log，使用`/log/1`而不是`log?n=1`。客户端发送请求的 payload 为空，服务端返回的数据根据定义的编码方式编码后放在 payload。

`POST`方法不包含 URL 参数。

## 数据格式

对于请求或响应的数据，大多数情况使用 JSON 格式对数据编码后传输。定义一个简单通用的格式来表示一个小的数据单元，比如一个数字，一个矩形等。

数据的基本格式为

```json
{
    "name" : "<name>"
    "type" : "<type>"
}
```

`<name>` 是可选的，在需要区分多个相同类型的数据是则需要指定
`<type>` 用于表示数据的类型

### 基本类型

基本类型包括数字和字符串，使用下面的 JSON 格式来表示

``` json
{
	"name" : "<name>"
    "type" : "<type>"
    "value" : [ <value,...> ]
}
```

为了便于表示数组类型的数据，将`value`字段定义为 JSON 数组。

其中，
`<type>` 用于指定数据的类型，基本类型包括 `"number"`, `"string"`。
`<value...>` 用于指定1个或多个数据。

### 预定义类型

**日志条目**
```json
{
    "type"  : "log",
    "log"   : "<log_string>",
    "index" : <index>，
    "level" : <level>，
    "timestamp" : <timestamp>
}
```
`<log_string>` 表示日志内容
`<index>` 表示日志索引，每条日志应该有一个严格递增的索引值
`<level>` 是一个字符串，用于表示日志等级，包括 "debug", "info", "error"
`<timestamp>` 是一个数字，标识日志的时间戳

**矩形**

``` json
{
    "type" : "rect",
    "x" : <x>,
    "y" : <y>,
    "width" : <width>,
    "height" : <height>
}

```
`<x>` 表示矩形左上角X轴坐标
`<y>` 表示矩形左上角Y轴坐标
`<width>` 表示矩形宽度
`<height>` 表示矩形高度

**目标信息**

```json
{
    "type" : "target"
    "x" : <x>,
    "y" : <y>,
    "width" : <width>,
    "height" : <height>
    "id"  : <id>
}
```

`<x>` 表示目标位置左上角X轴坐标
`<y>` 表示目标位左上角Y轴坐标
`<width>` 表示目标位宽度
`<height>` 表示目标位高度
`<id>` 表示目标ID

**图片**

只支持灰度图片，JSON 格式为

```json
{
    "type" : "image"
    "width" : <width>
    "height" : <height>
    "data" : "<data>"
}
```

`<data>` 为base64编码后的图像数据


**如果某些数据不便于使用 JOSN 格式编码，可以根据具体需要定义编码方式。**

## URL格式

根据用途，url的前缀有以下几种形式

* `/config` 用于设置和获取算法参数
* `/log` 用于获取算法日志输出
* `/algo` 用于控制算法执行

### /config

`/config`前缀用于配置算法运行时参数配置，算法有多个参数，每个参数使用一个唯一名字来标识，比如有个参数名字为 `area_track_roi_width`，类型为整数，那么，

URL：/config/area_track_roi_width
GET：用于获取该参数的值
SET：用于设置该参数的值

如果要获取参数名列表，可以对 `/config` 使用 `GET` 方法，`/config`不支持 `SET` 方法。
`GET /config`返回数据为
```json
{
    "type" : "string",
    "value" : [
        "aera_track_roi_width",
        ...
    ]
}
```

### /log

`/log`用于获取算法日志输出，算法的日志存放在一个环形队列中

|    URL     | 方法 | 参数/响应 |             注释             |
| :--------: | :--: | :-------: | :--------------------------: |
| /log/clean | POST |    无     |           清除日志           |
| /log/count | GET  | 日志条数  |         获取日志条数         |
|  /log/<N>  | GET  | 日志内容  | N为日志索引，取值为[0,count) |

** POST /log/clean **
用于清除日志，POST方法的参数为空，响应为空

** GET /log/count **
用于获取日志条数，服务端响应为
```json
{
    "type" : "number",
    "value" : [ <count> ]
}
```

**GET /log/<N>**

用于获取指定索引的日志，N是当前日志缓存区中的索引，值的范围为 0 到 count-1。服务端响应为表示日志的 json

** 注意： N和<index>不一样，N是当前缓存区中的索引，<index>是日志的索引。

### /algo

用于算法控制及获取算法参数。



|     URL      | 方法 | 请求/响应   |       注释       |
| :----------: | :--: | :-------: | :--------------: |
|  /algo/exec  | POST |    无     |     执行算法     |
| /algo/reset | POST | 无 | 重置算法 |
| /algo/track | POST | 目标信息 | 开始跟踪 |
| /algo/image  | POST |   图像    |   设置当前图像   |
| /algo/result | GET  | 算法结果   | 获取算法执行结果 |

**POST /algo/image**

用于上传图像数据到算法，算法中有一个图像缓存区，算法执行时会从这个缓存区获取图像数据。图像数据编码方法待定

**POST /algo/exec**

触发算法执行，改操作没有参数。该操作会执行当前图像，在调用改操作前需要先设置当前图像

**POST /algo/track**

用于开始跟踪目标，请求的参数为`Target`, 指定目标位置和ID。发送这个请求之前应该先设置当前图像。

**GET /algo/result**

获取算法运行结果，算法的运行结果包含很多数据，包括运行花费时间，目标信息，算法的中间结果等，使用一个 JSON 数组表示服务端返回的数据。如果算法当前正在运行，则HTTP GET 请求直到算法运行完成后再返回数据。

算法结果要便于扩展，可能会添加新的数据，目前算法结果包含的数据有：

|    数据    |       类型       |        注释        |
| :--------: | :--------------: | :----------------: |
|  targets   |    target数组    |   所有的目标信息   |
| time_spent | number，单位为秒 | 算法执行花费的时间 |
|    ...     |                  |                    |

# 服务端API

服务端的设计要尽量统一，通用。服务端提供的 URL 依赖于算法，如算法参数，算法执行结果等，这些数据在开发阶段是不稳定的，可以会添加或删除，我们不希望直接修改服务端代码来实现这些修改。所以这些易变的数据应该通过API向服务端注册，服务端提供统一的API和框架。

## 算法参数

描述一个算法参数，需要的属性有名称和类型，
* `名称` 名称应该是一个标识符，采用C语言中变量的命名规则
* `类型` 如前面定义，基本类型，包括 `number`, `string`

可以定义如下数据结构来标识一个算法参数(仅参考)
```c
enum Type {
    NUMBER,
    STRING,
};
typedef (*AlgoParamGetFun)( const char *name, void *data );
typedef (*AlgoParamSetFun)( const char *name, const void *data );
struct AlgoParamItem{
    const char *name; /* 参数名 */
    enum Type   type; /* 参数类型 */
    AlgoParamGetFun get;
    AlgoParamSetFun set;
};
```
使用以下接口向HTTP服务端注册，

```c
void netRegisterAlgoParam( const char *name, enum Type type,  AlgoParamGetFun get, AlgoParamSetFun set );
```

在服务端包含`struct AlgoParamItem`的链表或数组，用于保存注册的参数项，当HTTP请求获取或设置参数，则根据名字在链表或数组中查找指定的参数，然后调用关联的回调函数实现设置或读取。

## 算法日志

定义日志等级

```c
enum LogLevel{
    LOG_DEBUG,
    LOG_INFO,
    LOG_ERROR,
};
```

使用下面接口输出日志，
```c
void netAlgoLogOutput( enum LogLevel level, const char *fmt, ... );
```

在服务端，应该包含一个用于存储日志的缓存区，调用`algoLogOutput`函数后将日志数据保存到缓存区中。