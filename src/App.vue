<template>
  <div ref="appContainer">
    <div v-if="wait" style="position: absolute; z-index: 99999;">
      <h1 style="color: red;">等待10S（这个不是倒计时）</h1>
    </div>
    <div v-else style="position: absolute; z-index: 99999;">
      <button @click="playVideo">Play</button>
    </div>
    <div class="ui" v-if="showUi">
      <!-- chat -->
      <div class="chat">
        <!-- 清理 -->
        <div class="func"><font-awesome-icon :icon="['fas', 'trash']" size="lg" @click="clearContent"/></div>
        <!-- 聊天内容 -->
        <div class="content">
          <div v-for="(content, index) in chatContent" :key="index">
            <div class="content-vm" v-if="content.type == 'vm'">
              <font-awesome-icon :icon="['fas', 'user-astronaut']" size="2x" />
              <div class="content-text">{{content.text}}</div>
            </div>
            <div class="content-user" v-else>
              <div class="content-text">{{content.text}}</div>
              <font-awesome-icon :icon="['fas', 'user-circle']" size="2x" />
            </div>
          </div>
          <div ref="hiddenScroller" style="height: 50px"></div>
        </div>
        <!-- 输入文本 -->
        <div class="input">
          <!-- 录音按钮 -->
          <div :class="recodeIconClass" 
            @touchstart="startRecode" @touchend="stopRecode" @touchcancel="stopRecode" 
            @mousedown="startRecode" @mouseup="stopRecode" @mouseleave="stopRecode">
            <font-awesome-icon :icon="['fas', 'microphone']" size="3x" />
          </div>
          <!-- 文本输入 -->
          <div class="textInput">
            <input v-model="inputValue" @focus="onInputFocus" @blur="onInputBlur" placeholder="hi,say something to me..." />
          </div>
          <div class="submit">
            <button @click="sendText">发送</button>
          </div>
        </div>
      </div>
      <!-- control groups -->
      <div class="controls">
        <div class="control-group">
          <Slider v-on:change="changeAngle" :max="180" :min="-180" />
        </div>
        <div class="control-group">
          <button @click="changeZoom(0)">全身</button>
          <button @click="changeZoom(50)">半身</button>
          <button @click="changeZoom(100)">特写</button>
        </div>
        <div class="control-group">
          <button @click="changeAlign(0)">对齐左</button>
          <button @click="changeAlign(1)">对齐中</button>
          <button @click="changeAlign(2)">对齐右</button>
        </div>
        <div class="control-group">
          <button @click="changeBg(1)">背景1</button>
          <button @click="changeBg(2)">背景2</button>
          <button @click="changeBg(3)">背景3</button>
        </div>
      </div>
      <!-- nav -->
      <div class="nav">
        <a href="https://www.pingxingyun.com/"><button>返回首页</button></a>
      </div>
    </div>
  </div>
</template>

<script>
import { LarkSR } from "larksr_websdk";
import Slider from './components/slider/slider.vue';
import VConsole from "vconsole";

const DataChannelType = {
  // 初始化配置，包括头发，⾐服，背景等。
  // 在收到初始化配置之后再显⽰虚拟⼈
  InitConfig: 0,
  // ⽂本输⼊ 
  TextInput: 1, 
  // 视⻆控制 
  CameraControl: 2, 
  // 客⼾端⼤⼩ 
  ClientSize: 3, 
  // 虚拟⼈背景 
  Background: 4,
}

export default {
  name: "App",
  components: {
    Slider,
  },
  data() {
    return {
      wait: true,
      chatContent: [],
      larksr: null,
      inputValue: '',
      showUi: false,
      recoder: null,
      recodeTimer: null,
      // 方位角(angle) -180°（左） 到 180°（右）, 0 为正前方)
      // 放大(zoom) 0 - 100 0 最小全身, 100 脸部特写
      // 对齐（align） 0 在屏幕最左边 1 在屏幕居中 2 在屏幕最右边
      savedCamera: {
        align: 1,
        zoom: 50,
        angle: 0
      },
      meidaplaymute: false,
      vconsole: null,
    };
  },
  computed: {
    recodeIconClass() {
      return this.recoder !== null ? 'recoding recoder' : 'recoder';
    },
  },
  methods: {
    // 开始录制
    startRecode(e) {
      console.log('start recode');
      e.preventDefault();
      if (this.meidaplaymute) {
        this.larksr?.videoComponent.sountPlayout();
        this.meidaplaymute = false;
      }
      this.larksr.startAiDmVoiceInput();
    },
    stopRecode(e) {
      console.log('stop recode');
      e.preventDefault();
      this.larksr.stopAiDmVoiceInput();
    },
    pauseRecode() {
      console.log('pause recode');
      this.larksr.stopAiDmVoiceInput();
    },
    // 处理文字输入输出
    vmOutput(id, text) {
      this.chatContent.push({
        id,
        type: 'vm',
        text,
      });
      this.$refs.hiddenScroller.scrollIntoView({behavior: "smooth"});
    },
    userInput(id, text) {
      let found = false;
      for (let i = this.chatContent.length - 1; i >= 0; i--) {
        if (this.chatContent[i].id == id) {
          this.chatContent[i].text = text;
          found = true;
        }
      }
      if (!found) {
        this.chatContent.push({
          id,
          type: 'user',
          text,
        });
      }
      this.$refs.hiddenScroller.scrollIntoView({behavior: "smooth"});
    },
    clearContent() {
      this.chatContent = [];
    },
    onInputFocus() {
      console.log('onInputFocus', this.larksr.op.setKeyboardEnable);
      this.larksr?.op.setKeyboardEnable(false);
    },
    onInputBlur() {
      console.log('onInputBlur');
      this.larksr?.op.setKeyboardEnable(true);
    },
    // 发送初始化配置给服务端
    // 初始化配置（新增）包括头发，⾐服，背景等。在收到初始化配置之后再显⽰虚拟⼈
    sendInitConfig() {
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.InitConfig, 
          data: {
            // 预制背景 1...,
            background: 1,
            // 预制头发 1...
            hair: 1,
            // 预制⾐服 1...
            clothing: 1,
            // 其他预制项⽬...
          }
      }));
    },
    // 发送文本给云端应用
    sendText() {
      if (this.meidaplaymute) {
        this.larksr?.videoComponent.sountPlayout();
        this.meidaplaymute = false;
      }
      if (!this.inputValue) {
        console.log('input empty');
        return;
      }
      const id = this.larksr.aiDmTextInput(this.inputValue);
      this.userInput(id, this.inputValue);
      this.inputValue = "";
    },
    // 发送控制指令到云端
    // 更换背景
    changeBg(index) {
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.Background, 
          data: {
            backgound:index,
          }
      }));
    },
    // 切换摄像机角度
    changeAngle(angle) {
      // console.log('changeAngle', angle);
      this.savedCamera.angle = angle;
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.CameraControl, 
          data: this.savedCamera,
      }));
    },
    // 切换摄像机缩放
    changeZoom(zoom) {
      this.savedCamera.zoom = zoom;
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.CameraControl, 
          data: this.savedCamera,
      }));
    },
    // 切换摄像机位置
    changeAlign(align) {
      this.savedCamera.align = align;
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.CameraControl, 
          data: this.savedCamera,
      }));
      console.log('send align ', this.savedCamera);
    },
    // 客户端大小
    clientSize(width, height) {
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.ClientSize, 
          data: {
            platform: 'win32', 
            width, 
            height, 
            scale: 1 
          }
      }));
    },
    createLarkSR() {
      console.log("LarkSR ", this.$refs["appContainer"]);
      const larksr = new LarkSR({
          rootElement: this.$refs["appContainer"],
          // 服务器地址,实际使用中填写您的服务器地址
          // 如：http://222.128.6.137:8181/
          // serverAddress: "https://cloudlark.pingxingyun.com:8180/",
          // serverAddress: "http://222.128.6.137:8181/",
          serverAddress: "https://cloudlark.pingxingyun.com:8586/",
          // serverAddress: "http://192.168.0.55:8181/",
          // serverAddress: "http://cloudlark.pingxingyun.com:8585",
          // SDK授权码，可在开发者平台申请 https://www.pingxingyun.com/console
          authCode: "SDK授权码，可在开发者平台申请 https://www.pingxingyun.com/console",
          // 视频缩放模式，默认保留宽高比，不会拉伸并完整显示在容器中
          scaleMode: "contain",
          // 0 -》 用户手动触发, 1 -》 首次点击进入触发, 2 -》 每次点击触发
          fullScreenMode: 0,
          // 测试载入背景图
          // loadingBgUrl: "https://home-obs.pingxingyun.com/homePage_4_0/bg.jpg",
          // logLevel: 'info',
          frameRate: 60,
          codeRate: 8000,
      });
      // console.log('larksr', larksr, LoadAppliParamsFromUrl());
      // larksr.setAppliParams(LoadAppliParamsFromUrl());
      // larksr.start();

      // 监听连接成功事件
      larksr.on("connect", (e) => {
        console.log("LarkSRClientEvent CONNECT", e);
      });
      larksr.on("gotremotesteam", (e) => {
        console.log("LarkSRClientEvent gotremotesteam", e);
      });
      larksr.on("meidaloaded", (e) => {
        console.log("LarkSRClientEvent meidaloaded", e);
        this.showUi = true;
      });
      larksr.on("mediaplaysuccess", (e) => {
        console.log("LarkSRClientEvent mediaplaysuccess", e);
      });
      larksr.on("mediaplayfailed", (e) => {
        console.log("LarkSRClientEvent mediaplayfailed", e);
      });
      larksr.on("meidaplaymute", (e) => {
        console.log("LarkSRClientEvent meidaplaymute", e);
        this.meidaplaymute = true;
      });
      larksr.on("datachannelopen", (e) => {
        console.log("datachannel open",  e);
        // send client size
        this.clientSize(larksr.screenState.viewPort.width, larksr.screenState.viewPort.height);
        this.sendInitConfig();
      });

      larksr.on("datachanneltext", (e) => {
        try {
          const cmd = JSON.parse(e.data);
          console.log('datachannel cmd', cmd);
          if (cmd.type == 4 && cmd.data.text == 'PROJECT_READY') {
            this.changeAlign(0);
          }
        } catch(ext) {
          console.log('parse datachannel json cmd failed', e.data);
        }
      });

      console.log("load appli success", this.larksr);

      larksr.on("error", (e) => {
        console.error("LarkSRClientEvent error", e);
        alert(JSON.stringify(e.message));
      });

      larksr.on("info", (e) => {
        console.log("LarkSRClientEvent info", e);
      });

      larksr.on('aivoicestatus', (e) => {
        console.log('aivoicestatus', e);
        this.vmOutput(0, JSON.stringify(e.data));
      });

      larksr.on('aivoiceasrresult', (e) => {
        console.log('aivoiceasrresult', e.data);
        this.userInput(e.data.voiceId, e.data.text);
      });

      larksr.on('aivoicedmresult', (e) => {
        try {
          let res = JSON.parse(e.data.text);
          console.log('aivoicedmresult', res);
          this.vmOutput(e.data.voiceId, res.dm.nlg);
        } catch(e) {
          console.warn('parse aivoicedmresult failed', e.data);
        }
      });

      larksr.on('aivoiceerror', (e) => {
        alert(JSON.stringify(e.data));
      });

      this.larksr = larksr;

      // try play video
      // document.addEventListener(
      //   "WeixinJSBridgeReady",
      //   () => {
      //     console.warn("WeixinJSBridgeReady play video");
      //     // WARNING
      //     // 可能导致视频播放成功多次
      //     larksr.videoComponent.playVideo();
      //   },
      //   false
      // );

      // if (window["WeixinJSBridge"]) {
      //   console.warn("WeixinJSBridge exits; try play video",  window["WeixinJSBridge"]);
      //   window["WeixinJSBridge"].ready(() => {
      //     console.warn("WeixinJSBridge exits; play video", window["WeixinJSBridge"]);
      //     larksr.videoComponent.playVideo();
      //   });
      // } else {
      //   console.warn("WeixinJSBridge empty", window["WeixinJSBridge"]);
      // }
    },
    // 用户手动触发视频播放，可以添加合适的提示或者引导
    playVideo() {
      console.log('manu play out');
      if (this.larksr) {
        this.larksr.videoComponent.playVideo();
      } else {
        console.warn("lark sr not ready", this.larksr);
      }
    },
  },
  mounted() {
    this.vconsole = new VConsole();

    // 直接 LarkSR 对象，避免某些浏览器中视频播放失败的问题
    // 服务器地址可以异步更新
    this.createLarkSR();

    // 此处演示异步更新服务器地址
    window.setTimeout(() => {
      this.wait = false;
      
      // 异步更新服务器地址并连接
      this.larksr.updateServerAddress("https://cloudlark.pingxingyun.com:8586/");
      
      this.larksr.connect({
         appliId: '949293606043123712',
         // appliId: '948511483568848896',
      })
      .then(() => {})
      .catch((e) => {
        alert(JSON.stringify(e));
      });

    }, 1000 * 10);
  },
  beforeUnmount() {
    this.larksr.app.disConnect();
    this.larksr = null;
    if (this.vconsole) {
      this.vconsole.destroy();
    }
  },
};
</script>

<style>
/* ui main */
.ui {
    position: absolute;
    top: 0;
    left: 0;
    z-index: 1000;
    width: 100%;
    height: 100%;
    pointer-events: none;
}

/* chat */
.ui .chat {
    position: absolute;
    right: 0;
    top: 10%;
    width: 350px;
    height: 80%;
    background-color: rgba(46, 100, 255, 0.2);
    color: white;
    border: 2px solid rgba(18, 37, 95, 0.4);
    pointer-events: all;
    display: flex;
    flex-direction: column;
    z-index: 1100;
}

.chat .func {
  padding: 10px;
  border-bottom: 1px solid #fff;
}

.chat .content {
  flex: 1;
  width: 100%;
  overflow: auto;
  word-break: break-all;
  padding: 10px;
}

.chat .input {
    display: flex;
    flex-direction: row;
    align-content: center;
    justify-content: space-around;
    padding: 10px 0;
}

.chat .input .recoding {
  color: red;
}

.chat .input .textInput {
    display: flex;
    flex-direction: row;
    align-content: center;
    justify-content: center;
}

.chat .input .submit {
    display: flex;
    flex-direction: row;
    align-content: center;
    justify-content: center;
}

.chat .content-vm {
  display: flex;
  flex-direction: row;
  align-items: center;
  justify-content: flex-start;
  padding: 5px 0;
}
.chat .content-vm .content-text {
  padding: 5px 10px;
  background-color: #fff;
  color: #333;
  margin-left: 5px;
  border-radius: 5px;
}
.chat .content-user {
  display: flex;
  flex-direction: row;
  align-items: center;
  justify-content: flex-end;
  padding: 5px 0;
}
.chat .content-user .content-text {
  padding: 5px 10px;
  background-color: rgba(18, 37, 95, 0.8);
    color: #fff;
  margin-right: 5px;
  border-radius: 5px;
}

.chat .submit button {
  border: 0;
  box-shadow: 0;
  width: 80px;
  background: rgba(18, 37, 95, 0.8);
  color: #fff;
  size: 16px;
  border-radius: 5px;
}
/* nav */
.ui .nav {
  position: absolute;
  right: 0;
  top: 0;
  padding: 20px 10px;
  pointer-events: all;
  z-index: 1002;
}
.ui .nav button {
  border: 0;
  box-shadow: 0;
  width: 80px;
  background: rgba(18, 37, 95, 0.8);
  color: #fff;
  size: 16px;
  border-radius: 5px;
  padding: 5px 0;
}
/* controls */

.ui .controls {
  position: absolute;
  bottom: 50px;
  pointer-events: all;
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  z-index: 1001;
}
.ui .controls .control-group {
    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: space-around;
    width: 300px;
    margin-bottom: 10px;
    pointer-events: all;
}
.ui .controls .control-group button {
  border: 0;
  box-shadow: 0;
  width: 80px;
  background: rgba(18, 37, 95, 0.8);
  color: #fff;
  size: 16px;
  border-radius: 5px;
  padding: 5px 0;
}
</style>
