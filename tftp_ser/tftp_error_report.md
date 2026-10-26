# TFTP 项目错误复盘报告

## 背景

这次 TFTP（Trivial File Transfer Protocol）项目，客户端和服务端都写完了，基本功能能跑通。但中间有一个 bug 卡了很久：**服务端 bind 成功之后，日志显示 bind 的是 `255.255.255.255:69`，客户端发什么包服务端都收不到，`recvfrom` 的 PEEK 日志永远不打印**。最后排查出来，**`#define SER_IP " 172.18.0.1"` 前面多了一个空格**，`inet_addr` 解析失败返回了 `INADDR_NONE`（`0xFFFFFFFF`），被 bind 成了一个广播地址。

这份报告把整条链路上的坑都拉出来盘一遍。

## 踩坑清单（一页扫完）

| 坑 | 踩了会怎样 | 怎么提前暴露 |
|:---|:---|:---|
| 宏定义里带空格，`inet_addr` 静默返回 `INADDR_NONE` | bind 到广播/无效地址，收不到包，不报错 | 编译后加 `if(inet_addr(ip)==INADDR_NONE)` 直接 panic；日志打 `inet_ntoa(sin.sin_addr)`，一眼看到 `255.255.255.255` 就知道错了 |
| `bind` 失败只 `perror` 不 `return` | 继续 `recvfrom`，但 socket 根本没在监听，死等或收到一堆错 | `bind` 失败立刻 `exit(1)` 或 `return`；`setsockopt` 同理 |
| UDP `sendto` 成功不等于对端收到 | 客户端以为发出去了，服务端其实没在监听那个 IP，包静默丢弃 | `tcpdump -i any 'udp port 69'` 抓两端流量；服务端开 `DBG_RECV` 打印每条收到的包 |
| `recvfrom + MSG_PEEK` 之后不真收包 | PEEK 只是"偷看"，包还在队列里，下一次 `recvfrom` 才会真正拿走；如果 PEEK 之后没继续收，后续逻辑就拿到的是旧数据 | 每段 PEEK 之后必须有配套的真 `recvfrom`；或者干脆用 `select` 替代 PEEK |
| 服务端 `std::getline(std::cin, m)` 卡死 | 非交互运行（管道/脚本）时，服务端阻塞在键盘输入上，请求全堆在队列里 | 生产环境把交互 prompt 用宏关掉；或从环境变量/配置文件读权限；测试时用 `echo y \| ./a.out` 喂 stdin |
| `handle_erorr` 里 `msg[1]=5` 没设 `msg[0]=0` | 错误报文头两字节不是标准的 `0x00 0x05`，接收端按 opcode=0 解析 | 所有发报文的构造函数（包括 error）都走一个 `make_packet(opcode, ...)` 统一封装，`msg[0]=0` 由它保证 |
| `*(uint16_t*)(msg+2)` 强转 + 字节序 | 小端系统上传/下载时块号反着来，服务端拿到的块号是反转的 | 全部走 `htons`/`ntohs`；编译加 `-Wall -Wextra` 看有没有对齐告警（虽然 2 字节对齐通常不告警，`*(uint16_t*)` 本身合法） |
| `char*` 数组当 C 字符串 + 魔数 `0` 判断结束 | `msg[0]=0` 是 opcode 字段，不是字符串结束符；`strlen(msg)` 会返回 0；`msg+2` 的指针不是标准 C 字符串 | 永远用 `sprintf` 返回值，不要 `strlen`；解析包时按协议字段偏移读，别当成字符串 |
| `setsockopt(SO_REUSEADDR)` 其实没用 | UDP 单 socket 不需要 `SO_REUSEADDR`；它主要管 TCP TIME_WAIT 和 IP 复用 | UDP 服务里这行可以删掉；留着不报错，但别当救命稻草 |
| `lseek(fd,0,SEEK_END)` 打印文件总大小，但没 `lseek` 回去再读 | 读到 EOF 立刻退出，文件没发出去 | `lseek` 之后必须 `lseek(fd,0,SEEK_SET)`；或者干脆别打印总大小，只打印每次 `read` 的实际长度 |
| 上传循环里 `ret` 变量作用域混乱 | `ret` 在外层循环被赋一次，内层再被赋一次，两次值不一样，逻辑看着就绕 | 内外层各用 `nrecv` / `nwrite` 分开命名，一眼看清谁是谁 |
| `if(sockfd!=-1)` 在析构函数里没起作用 | 如果 `socket()` 失败没 return，`sockfd==-1` 还会被 `close`，报 `EBADF` | 析构函数前加 `if(sockfd>=0) close(sockfd);` |
| `system("clear")` 清屏 | Windows/跨平台下报错 | 用 `\033[2J\033[H` 或者干脆不清屏 |
| 服务端 `handle_download` 里 `ret==0` 分支直接 `return`，没 `close(fd)` | 文件描述符泄漏 | 所有 `open` 之后必须有配套的 `close`，用 RAII（`std::unique_ptr<int, Closer>`）或者至少在 return 前 `close` |

## 逐坑详解

### 1. `#define SER_IP " 172.18.0.1"` 前导空格

这是这次最坑的。`inet_addr` 对串头的空格直接判失败，返回 `INADDR_NONE`（`-1`，内存里 `0xFFFFFFFF`），被 bind 到 `255.255.255.255`。这种地址 bind 是合法的（内核不校验），所以 `bind` 返回 0，日志打 `bind 成功`。

`inet_addr` 不是宽容解析器，串头、串尾的任意空白、下划线、`::` 之类的都会让它判失败。工业写法：

```c
#include <netdb.h>

int init_addr(const char* ip, in_addr_t* out) {
    struct addrinfo hints = {0}, *res = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int rc = getaddrinfo(ip, NULL, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo(%s): %s\n", ip, gai_strerror(rc));
        return -1;
    }
    *out = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(res);
    return 0;
}
```

或者最简：

```c
in_addr_t a = inet_addr(ip);
if (a == INADDR_NONE) { /* 解析失败，打日志并退出 */ }
```

`inet_ntoa(a)` 打出来一眼就知道是不是 `255.255.255.255`，这种地址出现就是 `inet_addr` 失败。

### 2. `bind` 失败不 `return`

`bind` 失败 `perror` 后继续 `recvfrom`，等于 socket 根本没在监听就等着收包，`recvfrom` 要么直接报 `ENOTCONN`，要么永远阻塞。工业写法：

```c
if (bind(sockfd, (const sockaddr*)&sin, slen) == -1) {
    perror("bind");
    close(sockfd);
    exit(1);
}
```

同理，`socket()` 失败、`setsockopt()` 失败，都要立刻退出。

### 3. UDP `sendto` "成功"的陷阱

UDP 没有握手，`sendto` 只是把包丢进内核发送队列，**不会告诉你对端有没有收到**。服务端如果 bind 到了错的地址、或者防火墙拦截，客户端这边是没有任何"发不出去"信号的。调试时：

- 服务端开 `DBG_RECV` 打印每条收到的包（opcode + 块号 + 字节数）
- 客户端开 `DBG_SEND` 打印每条发出的包
- 用 `tcpdump -i any -s 0 'udp port 69'` 在两端各抓一份，看包到底走没走到 69 端口

`tcpdump` 是 UDP 排错的最硬证据。

### 4. `MSG_PEEK` 配对

`recvfrom(..., MSG_PEEK, ...)` 不会把包从队列里取走，它只是"看一眼"。如果 PEEK 之后没有配套的普通 `recvfrom`，后续逻辑拿到的还是同一份数据。`select` 比 PEEK 更稳：

```c
fd_set rfds;
FD_ZERO(&rfds);
FD_SET(sockfd, &rfds);
struct timeval tv = {5, 0}; // 5 秒超时
int n = select(sockfd+1, &rfds, NULL, NULL, &tv);
if (n < 0) { perror("select"); ... }
else if (n == 0) { /* 超时 */ }
else {
    // 真正收包
    int nrecv = recvfrom(sockfd, msg, MSG_SIZE, 0, (sockaddr*)&cin, &clen);
}
```

### 5. 交互 prompt 卡死

`std::getline(std::cin, m)` 在非交互环境（管道、`nohup`、CI）会直接阻塞。生产环境要么关掉，要么从环境变量/配置文件读权限：

```c
bool allow = (getenv("TFTP_ALLOW_UPLOAD") && *getenv("TFTP_ALLOW_UPLOAD"));
```

测试时喂 stdin：

```sh
echo "y" | ./a.out
```

### 6. `handle_erorr` 头字段

```c
void handle_erorr(uint16_t err_code) {
    msg[0] = 0;  // ← 这一行别漏
    msg[1] = 5;
    *(uint16_t*)(msg+2) = htons(err_code);
    // ...
}
```

### 7. 字节序 + 块号

所有 16/32 位整数字段进 `msg` 前必须 `htons`/`htonl`，读出来必须 `ntohs`/`ntohl`。TFTP 协议规定所有多字节字段是大端，host 端是小端，不转就反。

### 8. `char*` 当 C 字符串 + 魔数 `0`

`msg[0]` 是 opcode 字段，不是字符串结束符。`strlen(msg)` 永远返回 0（因为 `msg[0]` 本身就是 0）。解析包永远按协议偏移读：

```c
uint16_t opcode = ntohs(*(uint16_t*)(msg));  // 实际 TFTP 是 msg[0..1] 两字节
const char* filename = msg + 2;
const char* mode = filename + strlen(filename) + 1;
```

不要用 `strlen(msg)`，不要当成 C 字符串。

### 9. `SO_REUSEADDR` 其实没用

UDP 单 socket 不需要 `SO_REUSEADDR`，它主要管 TCP TIME_WAIT 和 IP 复用。留着不报错，但别当救命稻草。

### 10. `lseek` 之后忘 `lseek` 回去

`lseek(fd, 0, SEEK_END)` 读总大小之后必须 `lseek(fd, 0, SEEK_SET)`，否则下一次 `read` 直接到 EOF，`ret==0` 立刻退出。

### 11. `ret` 变量作用域混乱

`handle_download` 外层循环 `ret=read(fd, msg+4, MSG_SIZE-4)`，内层再 `int nrecv=recvfrom(...)`。两个变量名混着看容易拿错值。建议：

- 外层：`nread`（文件读到的字节数）
- 内层：`nrecv`（socket 收到的字节数）

### 12. `close` 配对

所有 `open` 之后必须有配套的 `close`。服务端 `handle_download` 里 `ret==0` 分支直接 `return`，`fd` 没关。RAII 写法：

```c
int fd = open(path, O_RDONLY);
if (fd < 0) { ... }
// 用 std::unique_ptr<int, Closer> 包一下，所有 return 路径都自动 close
```

### 13. 析构函数里 `close(sockfd)`

如果 `socket()` 失败没 return，`sockfd==-1` 还会被 `close`，报 `EBADF`：

```c
tftp_ser::~tftp_ser() {
    if (sockfd >= 0) close(sockfd);
}
```

### 14. `system("clear")`

Windows/跨平台下报错。用 `\033[2J\033[H` 或者干脆不清屏。

## 最安全/工业写法

| 场景 | 写法 | 原理 |
|:---|:---|:---|
| 地址解析 | `getaddrinfo` 或 `inet_addr + INADDR_NONE` 检查 | `inet_addr` 静默失败，必须显式判断 |
| 错误返回 | `perror` 后立刻 `exit(1)` 或 `return` | 失败后继续走等于在错误状态下运行 |
| UDP 收包 | `select` + 真 `recvfrom` 配对 | `select` 不会"偷看"，超时可控 |
| 报文构造 | 统一 `make_packet(opcode, ...)`，保证 `msg[0]=0` | 协议字段偏移写死，别在调用点拼 |
| 字节序 | 所有多字节字段 `htons`/`ntohs` | TFTP 协议规定大端 |
| 文件描述符 | RAII（`std::unique_ptr<int, Closer>`） | 所有 return 路径都自动 close，不漏 |
| 权限 | 环境变量/配置文件，不用 `getline` | 生产环境非交互 |
| 日志 | `DBG_RECV` / `DBG_SEND` 宏，编译期开关 | `#ifdef DEBUG` 控制，生产关掉 |
| 编译选项 | `g++ -Wall -Wextra -g -O2` | `-Wall -Wextra` 暴露对齐/未初始化，`-g` 给 gdb 用 |
| 抓包 | `tcpdump -i any -s 0 'udp port 69'` | UDP 没握手，抓包是最硬证据 |
| 端口检查 | `ss -ulnp \| grep 69` / `lsof -i :69` | 看服务端到底 bind 在哪 |

## 调试工具箱

**编译期**
```sh
g++ -Wall -Wextra -g -O2 -o a.out main.cpp tftp_ser.cpp
```
`-Wall -Wextra` 暴露未初始化、对齐、`char*` 强转告警。

**运行期**
- `inet_ntoa(sin.sin_addr)` 打出来，一眼看到是不是 `255.255.255.255`（`INADDR_NONE` 的征兆）
- 每条 `recvfrom`/`sendto` 前后 `DBG_RECV`/`DBG_SEND` 打印（opcode + 块号 + 字节数）
- `perror` 带 `__LINE__`/`__FILE__`，定位到代码行

**抓包**
```sh
# 服务端
sudo tcpdump -i eth0 -s 0 'udp port 69' -w ser.pcap
# 客户端
sudo tcpdump -i eth0 -s 0 'udp port 69' -w cli.pcap
# Wireshark 打开，看包到底走没走到 69
```

**端口检查**
```sh
ss -ulnp | grep 69        # 看服务端 bind 在哪
lsof -i :69              # 哪个进程占着 69
```

**gdb**
```sh
gdb ./a.out
(gdb) b tftp_ser::tftp_ser   # 断在构造函数
(gdb) c
(gdb) p inet_addr(" 172.18.0.1")   # 看 inet_addr 返回值
(gdb) p sin.sin_addr.s_addr        # 看 bind 前的地址
(gdb) p/x sin.sin_addr.s_addr      # 十六进制，一眼看到 0xFFFFFFFF
```

## 原理

**UDP 没有握手**：`sendto` 只是把包丢进内核发送队列，不会告诉你对端有没有收到。服务端 bind 错了，客户端这边是没有任何"发不出去"信号的。

**`INADDR_NONE` 不是广播**：`255.255.255.255` 是广播地址，`INADDR_NONE`（`-1`）是 `inet_addr` 的失败哨兵。bind 到 `INADDR_NONE` 是合法的，内核不校验，所以 `bind` 返回 0，日志打"成功"。

**`SO_REUSEADDR` 管什么**：主要管 TCP TIME_WAIT 和 IP 复用，UDP 单 socket 根本不需要它。

**`MSG_PEEK` 不取包**：只是"看一眼"，包还在队列里。后续必须有配套的普通 `recvfrom` 才能真拿到。

**`#define` 是文本替换**：`#define SER_IP " 172.18.0.1"` 展开后就是字符串字面量，前面那个空格是**宏定义里带的**，编译器不会帮你 trim。`inet_addr` 对串头空格直接判失败。

## 建议（给以后）

1. 所有 `#define` 的字符串字面量，**永远不要带空格**（包括前导/尾随）。CI 里加个 grep 检查。
2. 所有网络初始化，`inet_addr`/`getaddrinfo` 之后必须 `inet_ntoa` 打出来，看到 `255.255.255.255` 就 panic。
3. 所有 `bind`/`setsockopt` 失败，立刻 `exit(1)`，别继续往下走。
4. UDP 排错，先抓包再猜。`tcpdump` 是最硬证据。
5. 生产环境关掉所有交互 prompt，权限走环境变量/配置文件。
6. 编译永远 `-Wall -Wextra -g`，别省。
7. 报文构造走一个 `make_packet` 封装，`msg[0]=0` 由它保证，调用点不拼。
