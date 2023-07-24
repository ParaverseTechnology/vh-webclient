# vh-webclient

---

[English](./README.md) [中文](./README.zh_CN.md)

---

> Online demo https://pingxingyun.github.io/vh-webclient/
> CloudApp [unity3d_demo](/unity3d_demo/)

## Prepare

### install node

1. install nodejs v15.14.0

[v15.14.0](https://nodejs.org/download/release/v15.14.0/)

2. install yarn

* yarn

```cmd
npm install -g yarn
```

## Start

Terminal cd to project root dir and run: 

```cmd
yarn install
```

### Start Debug

```cmd
yarn serve
```

```cmd
  App running at:
  - Local:   http://localhost:8080/
  - Network: http://192.168.0.161:8080/
```

Browser Open http://localhost:8080/

> Browser request https or localhost to enable audio recoder.

### Build and output

```
yarn build
```

### Change LarkSR config

Open App.vue find line 237, change serverAddress, appliId. LarkSR cloud appid from LarkSR admin server.

```javascript
const larksr = new LarkSR({
    rootElement: this.$refs["appContainer"],
    // serverAddress: "http://cloudlark.pingxingyun.com:8585",
    serverAddress: "Lark Sever address",
    // SDK auth code，connect business@pingxingyun.com or register https://www.pingxingyun.com/console/#/
    authCode: "Your SDK ID",
    // test backgroud url
    // loadingBgUrl: "https://home-obs.pingxingyun.com/homePage_4_0/bg.jpg",
    // logLevel: 'info',
});

larksr.connect({
    // LarkSR cloud appid from LarkSR admin server.
    // doc
    // https://www.pingxingyun.com/online/api3_2.html?id=476
    appliId: 'appid from LarkSR admin',
});
```

### Functions

#### WebClient

> See src/App.vue

##### Start audio recode

```javascript
// Start audio recode and send to cloud.
this.larksr.startAiDmVoiceInput();
// Stop audio recode
this.larksr.stopAiDmVoiceInput();
```

##### Text input

```javascript
this.larksr.aiDmTextInput("Your text");
```

##### Callbacks

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

#### CloudApp

CloudApp use LarkXRDataChannel64.dll to get AiVoice Data.

CloudApp get AiVoice result from LarXR.

##### LarkXRAiVoice.h 

Regis Aivoice callback.

```c++
LARKXR_API int  DC_CALL lr_client_register_aivoice_callback(on_aivoice_callback cb,void* user_data);
```

Aivoice data struct.

```c++
struct AiVoicePacket
{
    bool url;                //true :online audio url(mp3) .false: audio pack (pcm)
    unsigned int voice_id;   //voice ID
    const char* online_url;  //if url = true url here or null
    int url_size;            //url size contain end \0
    const char* nlg;         // aivoice conversation text.
    int nlg_size;            // conversation text size, contain end \0
    // if url=false, pcm package data here
    unsigned int slice_id;   // voice slice ID
    int samples_per_sec;     //eg.16000
    int channels;            //eg.1
    const char* audio;       //data ptr,if url = true null
    int size_byte;           //data byte size
    bool last_packet;        //is last package
};
typedef void(*on_aivoice_callback)(struct AiVoicePacket* packet,void* user_data);
```

##### [Unity3D Demo](./unity3d_demo/)

[Unity3D Demo Doc](./unity3d_demo/README.md)

