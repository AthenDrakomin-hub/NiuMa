# 网页版游戏大厅及牛牛房间 技术方案

## 1. 架构设计
- **前端框架**: Vue 2 (复用原有的 `web-ui` 项目依赖 `element-ui`, `vue-router`, `vuex`)。
- **网络通信**: 原生 `WebSocket` API + `msgpack-lite` 二进制序列化。
- **打包部署**: 直接集成到原来的 `npm run build:prod` 中，由 GitHub Actions 统一部署到 `/var/www/niuma-admin/` 下的某个路由（如 `/game`）。

## 2. 目录结构设计
新增以下目录和文件到 `web-ui/src/` 中：
```
src/
├── api/
│   └── game/
│       └── game.js         # HTTP API：/player/game/create 等
├── assets/
│   └── images/
│       └── game/           # 从原 Cocos 提取的游戏素材图片
├── utils/
│   ├── msgpack.js          # MsgPack 的引入
│   └── websocket.js        # Vue 全局 WebSocket 管理器 (NetworkManager 移植)
├── views/
│   └── gameClient/
│       ├── lobby/          # 游戏大厅页面
│       │   └── index.vue
│       ├── grabNiuNiu/     # 抢庄牛牛房间页面
│       │   ├── index.vue
│       │   ├── Player.vue  # 玩家组件
│       │   └── Cards.vue   # 扑克牌组件
│       └── niu100/         # 百人牛牛房间页面
│           └── index.vue
```

## 3. 核心模块技术细节

### 3.1 WebSocket 管理器 (websocket.js)
负责管理游戏服务器长连接，维护状态（断开、连接中、已连接）。
使用 `msgpack-lite` 将 JS 对象转换为二进制流，并通过 Base64 包装后发送，兼容原 Cocos 代码 `NetworkManager.ts` 中的编解码逻辑：
```javascript
import msgpack from 'msgpack-lite'
import { Base64 } from 'js-base64'
import store from '@/store' // 用于获取玩家 token 等

class GameSocket {
  constructor() {
    this.ws = null
    this.callbacks = {}
    this.isConnected = false
  }

  connect(address, venueId, gameType) {
    this.ws = new WebSocket(address)
    this.ws.binaryType = 'arraybuffer'
    this.ws.onopen = () => {
      this.isConnected = true
      // 发送 MsgPlayerConnect 和 MsgEnterVenue
    }
    this.ws.onmessage = (e) => this.handleMessage(e)
    this.ws.onclose = () => { this.isConnected = false }
  }

  sendMessage(msgType, msg, signature = true) {
    // 签名逻辑 (复用 GameManager.ts 的签名)
    // msgpack 序列化并发送
  }

  handleMessage(e) {
    // msgpack 反序列化并分发给对应的 Vue 组件回调
  }
}
export default new GameSocket()
```

### 3.2 路由与权限
在 `src/router/index.js` 添加：
```javascript
{
  path: '/game-client',
  component: Layout, // 或者独立全屏布局
  children: [
    { path: 'lobby', component: () => import('@/views/gameClient/lobby/index') },
    { path: 'grab-niuniu', component: () => import('@/views/gameClient/grabNiuNiu/index') }
  ]
}
```

### 3.3 抢庄牛牛房间状态机 (GrabNiuNiu/index.vue)
Vue 组件中维护响应式状态 `gameState`，根据后端广播的消息（`MsgGrabNiuSync`, `MsgGrabNiuGameState`, `MsgGrabNiuDealCards` 等）实时更新视图。
```javascript
export default {
  data() {
    return {
      gameState: 0, // 0: 等待, 1: 发牌, 2: 抢庄, 3: 下注, 4: 比牌, 5: 结算
      players: [],
      myCards: [],
      mySeat: -1,
      bankerId: ''
    }
  },
  mounted() {
    // 注册 websocket 消息回调
    GameSocket.on('MsgGrabNiuGameState', this.onGameStateChange)
    GameSocket.on('MsgGrabNiuDealCards', this.onDealCards)
  }
}
```

## 4. 依赖管理
需要向 `web-ui` 项目中添加两个必须的网络包：
```bash
cd web-ui
npm install msgpack-lite js-base64 crypto-js
```