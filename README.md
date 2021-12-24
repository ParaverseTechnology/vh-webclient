# vh-webclient

## node 环境搭建

1. 安装 nodejs v15.14.0 （保持环境一至，都安装 v15.14.0 版本）

[下载 v15.14.0](https://nodejs.org/download/release/v15.14.0/)

2. 安装 cnpm (npm 淘宝镜像，速度快)

安装完 nodejs 打开命令行执行下面命令

```cmd
npm install -g cnpm --registry=https://registry.npmmirror.com
```

3. 安装依赖工具

* yarn

```cmd
cnpm install -g yarn
```

## 开始项目

命令行进入本项目根目录下，执行:

```cmd
yarn install
```

### 调试

```cmd
yarn serve
```

yarn serve 执行成功后命令行显示如下：

```cmd
  App running at:
  - Local:   http://localhost:8080/
  - Network: http://192.168.0.161:8080/
```

浏览器打开下面连接 http://localhost:8080/

> 录音功能只支持 https 或者 localhost，不要打开 ip 开头的连接。

### 编译输出
```
yarn build
```

### 修改为本地调试环境

打开 App.vue 找到 236 行，修改 serverAddress 为实际要调试的地址，appliId 修改为实际要调试的 appid，appid 在 lark 系统后台查看

```javascript
// 直接调用进入应用接口创建实例，自动配置连接云端资源
CreateLarkSRClientFromeAPI(
    {
    rootElement: this.$refs["appContainer"],
    // 服务器地址,实际使用中填写您的服务器地址
    // 如：http://222.128.6.137:8181/
    serverAddress: "服务器地址,实际使用中填写您的服务器地址",
    // 授权码
    authCode: "授权码",
    // 视频缩放模式，默认保留宽高比，不会拉伸并完整显示在容器中
    scaleMode: "contain",
    // 0 -》 用户手动触发, 1 -》 首次点击进入触发, 2 -》 每次点击触发
    fullScreenMode: 0,
    // 测试载入背景图
    loadingBgUrl: "https://home-obs.pingxingyun.com/homePage_4_0/bg.jpg",
    },
    { appliId: "要调试的appid，在系统后台查看" }
```