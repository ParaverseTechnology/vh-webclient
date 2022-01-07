<template>
  <div ref="appContainer">
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
          <div :class="recodeIconClass" @mousedown="startRecode" @mouseup="stopRecode" @mouseleave="stopRecode">
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
import { CreateLarkSRClientFromeAPI } from "larksr_websdk";
import Recorderx, { 
  // ENCODE_TYPE,
  RECORDER_STATE,
} from "recorderx";
import Slider from './components/slider/slider.vue';

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
      chatContent: [ { type: 'vm', text: 'hello', }, ],
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
    };
  },
  computed: {
    recodeIconClass() {
      return this.recoder !== null ? 'recoding recoder' : 'recoder';
    },
  },
  methods: {
    // 开始录制
    startRecode() {
      console.log('start recode');
      this.recoder = new Recorderx();
      // start recorderx
      this.recoder.start()
        .then(() => {
          console.log("start recording");
          this.sendRecoderBuffer(0x1);
          this.recodeTimeout();
          this.userInput('recoding....');
        })
        .catch(error => {
          console.log("Recording failed.", error);
        });
    },
    stopRecode() {
      if (this.recodeTimer) {
        window.clearInterval(this.recodeTimer);
        this.recodeTimer = null;
      }
      // pause recorderx
      if (this.recoder && this.recoder.state == RECORDER_STATE.RECORDING) {
        console.log('stop recode', this.recoder.state, this.recoder.ctx);
        this.sendRecoderBuffer(0x3);
        this.recoder?.pause();
        this.recoder?.clear();
        this.userInput('recoded');
      }
      this.recoder = null;
    },
    pauseRecode() {
      // pause recorderx
      this.recoder?.pause();
    },
    recodeTimeout() {
      if (this.recodeTimer) {
        window.clearInterval(this.recodeTimer);
      }
      this.recodeTimer = window.setInterval(() => {
          // recodeing
          console.log('recoder update...');
          this.sendRecoderBuffer(0x2);
      }, 1000);
    },
    // 0x1---------音频输入开始
    // 0x2---------音频输入中(用户录音时循环发送，初步定义为 1s 中切片一次即发送一次音频)
    // 0x3---------音频输入结束(用户本次输入结束)
    sendRecoderBuffer(state) {
      // get recoder
      if (this.recoder && this.recoder.state == RECORDER_STATE.RECORDING) {
          let buffer = this.recoder.getRecord({
              encodeTo: "wav",
              compressible: true,
          });
          console.log('send recode ', state, buffer, buffer.arrayBuffer());

          const blob = new Blob([new Uint8Array([0x0, 0x0, 0x0, state]), buffer]);
          blob.arrayBuffer()
          .then((value) => {
            const uint8array = new Uint8Array(value);
            console.log("send to datachannel buffers", uint8array);
            // send array buffer to datachannel.
            this.larksr?.sendBinaryToDataChannel(uint8array);
          })
          .catch((e) => {
            console.warn('connect buffer failed', e);
          })

          this.recoder.clear();
      }
    },
    // 处理文字输入输出
    vmOutput(text) {
      this.chatContent.push({
        type: 'vm',
        text,
      })
      this.$refs.hiddenScroller.scrollIntoView({behavior: "smooth"});
    },
    userInput(text) {
      this.chatContent.push({
        type: 'user',
        text,
      });
      this.$refs.hiddenScroller.scrollIntoView({behavior: "smooth"});
    },
    clearContent() {
      this.chatContent = [];
    },
    onInputFocus() {
      console.log('onInputFocus');
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
      if (!this.inputValue) {
        console.log('input empty');
        return;
      }
      this.larksr?.sendTextToDataChannel(JSON.stringify({
          type: DataChannelType.TextInput, 
          data: {
            // 语⾳⽣成的⻆⾊。 1 ==》 客⼾端侧 2 ===》 虚拟⼈侧 
            // 当⽤⼾输⼊的情况下 role 恒定为 1，表⽰客⼾侧 
            // 云端解析的情况下分两种 
            // 云端发送给客⼾端的⽂字为解析客⼾端语⾳ role 为 1，表⽰客⼾侧 
            // 云端发送给客⼾端的⽂字为虚拟⼈语⾳内容 role 为 2, 表⽰虚拟⼈侧
            role: 1,
            text: this.inputValue,
          }
      }));
      this.userInput(this.inputValue);
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
    }
  },
  mounted() {
    // 直接调用进入应用接口创建实例，自动配置连接云端资源
    CreateLarkSRClientFromeAPI(
      {
        rootElement: this.$refs["appContainer"],
        // 服务器地址,实际使用中填写您的服务器地址
        // 如：http://222.128.6.137:8181/
        // serverAddress: "https://cloudlark.pingxingyun.com:8180/",
        serverAddress: "http://222.128.6.137:8181/",
        // 授权码
        authCode: "授权码",
        // 视频缩放模式，默认保留宽高比，不会拉伸并完整显示在容器中
        scaleMode: "contain",
        // 0 -》 用户手动触发, 1 -》 首次点击进入触发, 2 -》 每次点击触发
        fullScreenMode: 0,
        // 测试载入背景图
        loadingBgUrl: "https://home-obs.pingxingyun.com/homePage_4_0/bg.jpg",
      },
      { appliId: "814071627896586240" }
    )
      .then((larksr) => {
        this.larksr = larksr;
        this.larksr.start();
        // 监听连接成功事件
        this.larksr.on("connect", (e) => {
          console.log("LarkSRClientEvent CONNECT", e);
        });
        this.larksr.on("gotremotesteam", (e) => {
          console.log("LarkSRClientEvent gotremotesteam", e);
        });
        this.larksr.on("meidaloaded", (e) => {
          console.log("LarkSRClientEvent meidaloaded", e);
          this.showUi = true;
        });
        this.larksr.on("mediaplaysuccess", (e) => {
          console.log("LarkSRClientEvent mediaplaysuccess", e);
        });
        this.larksr.on("mediaplayfailed", (e) => {
          console.log("LarkSRClientEvent mediaplayfailed", e);
        });
        this.larksr.on("meidaplaymute", (e) => {
          console.log("LarkSRClientEvent meidaplaymute", e);
        });
        this.larksr.on("datachannelopen", (e) => {
          console.log("datachannel open",  e);
          // send client size
          this.clientSize(this.larksr.screenState.viewPort.width, this.larksr.screenState.viewPort.height);

          this.sendInitConfig();
        });
        this.larksr.on("datachanneltext", (e) => {
          if (!e.data) {
            return;
          }
          let data;
          try {
            data = JSON.parse(e.data);
          } catch (jsonerror) {
            console.log('parse message failed.', e.data);
            // 暂时将文本直接作为输出
            this.vmOutput(e.data);
            return;
          }
          if (typeof(data) == 'number') {
            this.vmOutput(e.data);
            return;
          }
          switch (data.type) {
            case DataChannelType.TextInput:
              if (data.data.role) {
                if (data.data.role === 1) {
                  this.userInput(data.data.text);  
                } else {
                  this.vmOutput(data.data.text);  
                }
              } else {
                this.vmOutput(data.data);
              }
              break;
            default:
              console.log('unsupport data type', data);
              break;
          }
        });
        console.log("load appli success", larksr);
        this.larksr.on("error", (e) => {
          console.error("LarkSRClientEvent error", e);
          alert(JSON.stringify(e.message));
        });
        this.larksr.on("info", (e) => {
          console.log("LarkSRClientEvent info", e);
        });
      })
      .catch((e) => {
        console.log("load appli failed", e);
        alert(JSON.stringify(e));
      });
  },
  beforeUnmount() {
    this.larksr.app.disConnect();
    this.larksr = null;
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
