# vh-webclient

> 在线体验地址 https://pingxingyun.github.io/vh-webclient/
> 对应云端应用源码项目在 [unity3d_demo文件夹下](/unity3d_demo/)

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

打开 App.vue 找到 237 行，修改 serverAddress 为实际要调试的地址，appliId 修改为实际要调试的 appid，appid 在 lark 系统后台查看

```javascript
const larksr = new LarkSR({
    rootElement: this.$refs["appContainer"],
    // 服务器地址,实际使用中填写您的服务器地址
    // 如：http://222.128.6.137:8181/
    // serverAddress: "https://cloudlark.pingxingyun.com:8180/",
    // serverAddress: "http://222.128.6.137:8181/",
    serverAddress: "服务器地址,实际使用中填写您的服务器地址",
    // serverAddress: "http://cloudlark.pingxingyun.com:8585",
    // 授权码
    authCode: "SDK的授权码",
    // 视频缩放模式，默认保留宽高比，不会拉伸并完整显示在容器中
    scaleMode: "contain",
    // 0 -》 用户手动触发, 1 -》 首次点击进入触发, 2 -》 每次点击触发
    fullScreenMode: 0,
    // 测试载入背景图
    loadingBgUrl: "https://home-obs.pingxingyun.com/homePage_4_0/bg.jpg",
    // logLevel: 'info',
});

larksr.connect({
    appliId: '要调试的appid，在系统后台查看',
});
```

### 智能语音相关接口

#### Web 端

> 参考 src/App.vue 下代码

##### 打开麦克风并输入语音

```javascript
// 打开语音输入
this.larksr.startAiDmVoiceInput();
// 关闭语音输入
this.larksr.stopAiDmVoiceInput();
```

##### 文字输入

```javascript
this.larksr.aiDmTextInput("文字输入");
```

##### 智能语音相关回调

```javascript
larksr.on('aivoicestatus', (e) => {
  console.log('aivoicestatus', e);
});

larksr.on('aivoiceasrresult', (e) => {
  console.log('aivoiceasrresult', e.data);
});

larksr.on('aivoicedmresult', (e) => {
  try {
    let res = JSON.parse(e.data.text);
    console.log('aivoicedmresult', res);
  } catch(e) {
    console.warn('parse aivoicedmresult failed', e.data);
  }
});

larksr.on('aivoiceerror', (e) => {
  alert(JSON.stringify(e.data));
});
```

#### 云端三维应用

云端三维应用通过集成 LarkXRDataChannel64.dll 集成智能语音功能

三维应用端只需处理智能语音输出的对话结果，具体智能语音处理过程在 LarkXR 内部完成。

##### LarkXRAiVoice.h 

开启智能语音功能并注册回调函数,要注意在数据通道开启之后再启用智能语音功能

```c++
LARKXR_API int  DC_CALL lr_client_register_aivoice_callback(on_aivoice_callback cb,void* user_data);
```

回调数据结构

```c++
struct AiVoicePacket
{
  bool	url;				//true :online audio url(mp3) .false: audio pack (pcm)
  unsigned int voice_id;		//语音ID
  const char* online_url;		//如果url为true,该字段为url地址,否则该字段为NULL 
  int	    url_size;			//url长度 包含\0
  const char* nlg;			//当前语音对讲的文本
  int	    nlg_size;			//对讲文本长度 包含\0

  //如果URL为false 那么下面字段描述每一个pcm包
  unsigned int slice_id;		//一个语音分片ID
  int		samples_per_sec;	//eg.16000
  int		channels;		    //eg.1
  const char* audio;			//数据包指针,如果 url 为true 该字段为空
  int		size_byte;			//每一包的字节数
  bool	last_packet;		//是否为最后一包
};
typedef void(*on_aivoice_callback)(struct AiVoicePacket* packet,void* user_data);
```

##### [Unity3D Demo 中集成并使用 LarkXRDataChannel64](./unity3d_demo/)

[Unity3D Demo 文档](./unity3d_demo/README.md)

