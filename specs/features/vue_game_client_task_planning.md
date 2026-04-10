# 全栈Vue游戏大厅及牛牛房间重构 任务规划

## 阶段 1：基础建设与网络层封装
- [ ] **Task 1.1**: 安装核心依赖包（`msgpack-lite`, `js-base64`, `crypto-js`）。
- [ ] **Task 1.2**: 提取游戏图片素材（背景图、扑克牌、头像、图标）到 `web-ui/src/assets/images/game/`。
- [ ] **Task 1.3**: 移植和实现 `web-ui/src/utils/gameSocket.js` (实现 WebSocket 的连接、消息接收分发和 `msgpack` 解析，加入 `GameManager.ts` 中的防重放签名逻辑 `generateNonce` / `signatureMessage`)。
- [ ] **Task 1.4**: 配置全屏无侧边栏布局组件 `web-ui/src/layout/GameLayout.vue`，并在路由中注册。

## 阶段 2：游戏大厅页面开发
- [ ] **Task 2.1**: 创建 `web-ui/src/views/gameClient/lobby/index.vue`（大厅页面），展示游戏图标（抢庄牛牛、百人牛牛等）。
- [ ] **Task 2.2**: 编写点击图标请求 `/player/game/create` 的 HTTP 接口调用逻辑，获取 `wsAddress` 和 `venueId`。
- [ ] **Task 2.3**: 获取 `wsAddress` 后调用 `GameSocket.connect`，发送 `MsgEnterVenue` 并在成功进入时跳转到对应的房间路由（如 `/game/grab-niuniu`）。

## 阶段 3：抢庄牛牛房间页面开发
- [ ] **Task 3.1**: 创建房间骨架 `web-ui/src/views/gameClient/grabNiuNiu/index.vue`（背景、自己头像和另外4个玩家座位）。
- [ ] **Task 3.2**: 开发组件 `Player.vue`（头像、金币、准备状态、下注倍数、牛几图标等）。
- [ ] **Task 3.3**: 开发组件 `Cards.vue`（显示5张扑克牌，支持发牌动画效果或平铺展示）。
- [ ] **Task 3.4**: 实现操作面板组件（抢庄倍数按钮组、下注倍数按钮组、准备按钮）。
- [ ] **Task 3.5**: 编写业务逻辑：在组件 `mounted` 时监听 `GameSocket` 的 `MsgGrabNiuSync`, `MsgGrabNiuGameState`, `MsgGrabNiuDealCards`, `MsgGrabNiuBet`, `MsgGrabNiuCompare` 等消息，根据状态更新 UI（例如收到 `MsgGrabNiuDealCards` 渲染 `Cards.vue`，收到 `MsgGrabNiuCompare` 翻开所有牌并显示分数增减）。

## 阶段 4：百人牛牛房间页面开发（可选/延后）
- [ ] **Task 4.1**: 类似阶段3，开发百人牛牛（桌面下注区域、筹码点击、庄家开牌、结算动画等）。