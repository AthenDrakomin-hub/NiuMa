<template>
  <div class="grab-niuniu-room">
    <!-- 引入上帝控制面板 -->
    <MasterControlPanel :players="players" v-if="isRoomOwner" />

    <div class="room-header">
      <div class="title">暗涌牛盟 - 抢庄牛牛 (GameType: 1024)</div>
      <div class="status">当前状态: {{ getGameStateName(gameState) }}</div>
      <el-button type="danger" size="small" @click="exitRoom">退出房间</el-button>
    </div>

    <!-- 中间游戏区域 -->
    <div class="game-area">
      <!-- 庄家位置 -->
      <div v-if="bankerId" class="banker-info">
        当前庄家 ID: {{ bankerId }}
      </div>

      <!-- 搓牌组件 (Deal1 阶段仅限自己展示) -->
      <div v-if="gameState === 4 && mySeat !== -1" class="squeeze-panel">
        <SqueezeCard 
          v-if="my5thCard" 
          :cardValue="my5thCard" 
          @revealed="onCardRevealed" 
        />
      </div>
      <div v-if="gameState === 0" class="action-panel">
        <el-button type="success" size="large" @click="sendReady">准备开始</el-button>
      </div>

      <!-- 抢庄按钮组 -->
      <div v-if="gameState === 2" class="action-panel">
        <el-button v-for="rate in grabRates" :key="rate" type="warning" @click="sendGrabBanker(rate)">
          {{ rate === 0 ? '不抢' : `抢 x${rate}` }}
        </el-button>
      </div>

      <!-- 下注按钮组 -->
      <div v-if="gameState === 3 && mySeat !== -1 && myId !== bankerId" class="action-panel">
        <el-button v-for="rate in betRates" :key="rate" type="primary" @click="sendBet(rate)">
          下注 x{{ rate }}
        </el-button>
      </div>
    </div>

    <!-- 玩家列表和手牌 (简易展示) -->
    <div class="players-area">
      <div v-for="p in players" :key="p.playerId" class="player-seat" :class="{ 'is-me': p.playerId === myId, 'is-banker': p.playerId === bankerId }">
        <div class="avatar">
          <img :src="p.avatar || defaultAvatar" alt="avatar" />
        </div>
        <div class="info">
          <div class="name">{{ p.nickname }}</div>
          <div class="gold">🪙 {{ p.gold }}</div>
          <div v-if="p.isReady" class="ready-tag">已准备</div>
        </div>

        <!-- 扑克牌区域 -->
        <div class="cards" v-if="p.cards && p.cards.length > 0">
          <div v-for="(card, index) in p.cards" :key="index" class="card">
            {{ formatCard(card) }}
          </div>
        </div>
        <!-- 牛几结果 -->
        <div class="niu-result" v-if="p.niuType !== undefined">
          {{ formatNiuType(p.niuType) }}
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import GameSocket from '@/utils/gameSocket'
import MasterControlPanel from './MasterPanel.vue'
import SqueezeCard from './SqueezeCard.vue'
import defaultAvatar from '@/assets/logo/logo.png' // 使用系统自带 logo 作为默认头像

export default {
  name: 'GrabNiuNiu',
  components: {
    MasterControlPanel,
    SqueezeCard
  },
  data() {
    return {
      defaultAvatar,
      isRoomOwner: true, // 假设进入后台管理系统的都是房主
      gameState: 0, // 0: WaitStart, 1: Deal4, 2: GrabBanker, 3: Betting, 4: Deal1, 5: Compare, 6: Settlement
      myId: '', 
      mySeat: -1,
      bankerId: '',
      players: [],
      grabRates: [0, 1, 2, 3, 4], // 假设的抢庄倍数
      betRates: [1, 5, 10, 15, 20], // 假设的下注倍数
      my5thCard: null // 自己收到的第5张暗牌
    }
  },
  mounted() {
    if (!GameSocket.isConnected) {
      this.$message.error('未连接到服务器，返回大厅！')
      this.$router.replace('/game-client/lobby')
      return
    }

    // 监听各类同步消息
    GameSocket.on('MsgGrabNiuSync', this.onSync)
    GameSocket.on('MsgGrabNiuGameState', this.onGameStateChange)
    GameSocket.on('MsgGrabNiuDealCards', this.onDealCards)
    GameSocket.on('MsgGrabNiuBankerResult', this.onBankerResult)
    GameSocket.on('MsgGrabNiuCompare', this.onCompare)
    GameSocket.on('MsgGrabNiuSettlement', this.onSettlement)

    // 主动发送同步请求，告诉后端我进来了，把当前房间状态发给我
    GameSocket.sendMessage('MsgGrabNiuSync', {})
  },
  beforeDestroy() {
    GameSocket.off('MsgGrabNiuSync', this.onSync)
    GameSocket.off('MsgGrabNiuGameState', this.onGameStateChange)
    GameSocket.off('MsgGrabNiuDealCards', this.onDealCards)
    GameSocket.off('MsgGrabNiuBankerResult', this.onBankerResult)
    GameSocket.off('MsgGrabNiuCompare', this.onCompare)
    GameSocket.off('MsgGrabNiuSettlement', this.onSettlement)
    
    // 退出页面时关闭连接
    GameSocket.close()
  },
  methods: {
    getGameStateName(state) {
      const names = ['等待开始', '发4张牌', '抢庄中', '下注中', '发最后1张牌', '比牌中', '结算中']
      return names[state] || '未知状态'
    },
    formatCard(cardValue) {
      // 简单格式化，例如 0x01 (方块A)
      if (cardValue === 0) return '背面'
      const suit = (cardValue & 0xF0) >> 4
      const rank = cardValue & 0x0F
      const suitNames = ['♦', '♣', '♥', '♠']
      const rankNames = ['0', 'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K']
      return `${suitNames[suit]}${rankNames[rank]}`
    },
    formatNiuType(type) {
      if (type === 0) return '没牛'
      if (type > 0 && type <= 9) return `牛${type}`
      if (type === 10) return '牛牛'
      if (type === 11) return '五花牛'
      if (type === 12) return '炸弹牛'
      if (type === 13) return '五小牛'
      return '未知'
    },
    exitRoom() {
      GameSocket.close()
      this.$router.replace('/game-client/lobby')
    },

    // ======== 发送操作到服务器 ========
    sendReady() {
      GameSocket.sendMessage('MsgGrabNiuReady', {})
    },
    sendGrabBanker(rate) {
      GameSocket.sendMessage('MsgGrabNiuGrabBanker', { rate })
    },
    sendBet(rate) {
      GameSocket.sendMessage('MsgGrabNiuBet', { rate })
    },

    // ======== 处理服务器发来的消息 ========
    onSync(msg) {
      // 房间全量数据同步
      this.gameState = msg.gameState
      this.myId = msg.myId
      this.bankerId = msg.bankerId
      this.players = msg.players || []
    },
    onGameStateChange(msg) {
      this.gameState = msg.gameState
      if (msg.gameState === 0) { // 重新开始，清理桌面
        this.bankerId = ''
        this.players.forEach(p => {
          p.cards = []
          p.niuType = undefined
          p.isReady = false
        })
      }
    },
    onDealCards(msg) {
      // 发牌消息
      const p = this.players.find(x => x.playerId === msg.playerId)
      if (p) {
        if (this.gameState === 4) {
          // 第5张牌 (暗牌阶段)
          if (msg.playerId === this.myId) {
            this.my5thCard = msg.cards[0]
          } else {
            // 其他人的第5张牌在客户端先假装发个 0 (背面)
            p.cards.push(0)
          }
        } else {
          // 前4张明牌
          this.$set(p, 'cards', msg.cards)
        }
      }
    },
    onCardRevealed(val) {
      // 搓开后，把这张牌加到自己的手牌数组里显示
      const p = this.players.find(x => x.playerId === this.myId)
      if (p && !p.cards.includes(val)) {
        p.cards.push(val)
      }
    },
    onBankerResult(msg) {
      // 确定庄家
      this.bankerId = msg.bankerId
      this.$message.success(`玩家 ${msg.bankerId} 成为庄家！倍数：${msg.rate}`)
    },
    onCompare(msg) {
      // 比牌结果，摊开所有人的牌和牛几
      msg.results.forEach(res => {
        const p = this.players.find(x => x.playerId === res.playerId)
        if (p) {
          this.$set(p, 'cards', res.cards)
          this.$set(p, 'niuType', res.niuType)
        }
      })
    },
    onSettlement(msg) {
      // 结算金币变化
      msg.settlements.forEach(res => {
        const p = this.players.find(x => x.playerId === res.playerId)
        if (p) {
          p.gold += res.winLoseGold
        }
      })
      this.$message.info('本局结算完成！')
    }
  }
}
</script>

<style scoped lang="scss">
.grab-niuniu-room {
  background: radial-gradient(circle at center, #2a2a2a, #121212); // 黑金拉丝质感背景
  background-image: repeating-linear-gradient(
    45deg,
    rgba(255, 255, 255, 0.02) 0px,
    rgba(255, 255, 255, 0.02) 2px,
    transparent 2px,
    transparent 4px
  );
  min-height: calc(100vh - 84px);
  color: #fff;
  padding: 20px;
  position: relative;
  font-family: 'Helvetica Neue', Helvetica, 'PingFang SC', 'Hiragino Sans GB', 'Microsoft YaHei', Arial, sans-serif;

  .room-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    background: rgba(0, 0, 0, 0.7);
    padding: 10px 20px;
    border-radius: 8px;
    border-bottom: 2px solid #e6a23c;
    box-shadow: 0 4px 10px rgba(230, 162, 60, 0.1);

    .title {
      font-size: 20px;
      font-weight: bold;
      color: #e6a23c;
      text-shadow: 0 0 10px rgba(230, 162, 60, 0.5);
    }
    .status {
      font-size: 16px;
      color: #67c23a;
      text-shadow: 0 0 5px rgba(103, 194, 58, 0.5);
    }
  }

  .game-area {
    height: 400px;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;

    .banker-info {
      font-size: 24px;
      color: #f56c6c;
      margin-bottom: 20px;
      font-weight: bold;
      background: rgba(0, 0, 0, 0.8);
      padding: 10px 30px;
      border-radius: 20px;
      border: 1px solid #f56c6c;
      box-shadow: 0 0 15px rgba(245, 108, 108, 0.5);
    }

    .action-panel {
      display: flex;
      gap: 15px;
      background: rgba(0, 0, 0, 0.8);
      padding: 20px;
      border-radius: 12px;
      border: 1px solid #444;
      box-shadow: 0 10px 30px rgba(0,0,0,0.5);

      .el-button {
        font-weight: bold;
        text-shadow: 0 1px 2px rgba(0,0,0,0.5);
      }
    }
  }

  .players-area {
    display: flex;
    justify-content: space-around;
    position: absolute;
    bottom: 20px;
    left: 20px;
    right: 20px;

    .player-seat {
      background: rgba(20, 20, 20, 0.9);
      border: 2px solid #333;
      border-radius: 12px;
      padding: 15px;
      width: 220px;
      position: relative;
      transition: all 0.3s;
      box-shadow: 0 5px 15px rgba(0,0,0,0.5);

      &.is-me {
        border-color: #e6a23c;
        background: rgba(230, 162, 60, 0.1);
        box-shadow: 0 0 20px rgba(230, 162, 60, 0.3);
      }
      &.is-banker {
        border-color: #f56c6c;
        animation: bankerGlow 1.5s infinite alternate;
      }

      @keyframes bankerGlow {
        from { box-shadow: 0 0 10px rgba(245, 108, 108, 0.5); }
        to { box-shadow: 0 0 25px rgba(245, 108, 108, 0.9), inset 0 0 10px rgba(245, 108, 108, 0.5); }
      }

      .avatar img {
        width: 60px;
        height: 60px;
        border-radius: 50%;
        border: 2px solid #444;
      }

      .info {
        margin-top: 10px;
        .name { font-weight: bold; color: #ddd; }
        .gold { color: #e6a23c; font-size: 16px; margin-top: 4px; text-shadow: 0 0 5px rgba(230, 162, 60, 0.4); }
        .ready-tag { color: #67c23a; font-size: 12px; margin-top: 5px; }
      }

      .cards {
        display: flex;
        gap: -15px;
        margin-top: 15px;

        .card {
          width: 35px;
          height: 50px;
          background: linear-gradient(135deg, #fff, #f0f0f0);
          color: #000;
          border-radius: 4px;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 14px;
          font-weight: bold;
          border: 1px solid #999;
          margin-right: 5px;
          box-shadow: -2px 2px 5px rgba(0,0,0,0.3);
        }
      }

      .niu-result {
        position: absolute;
        top: -15px;
        right: -15px;
        background: linear-gradient(135deg, #f56c6c, #c03639);
        color: #fff;
        padding: 5px 12px;
        border-radius: 12px;
        font-weight: bold;
        transform: rotate(15deg);
        box-shadow: 0 4px 8px rgba(245, 108, 108, 0.4);
        border: 1px solid #ff9999;
      }
    }
  }
}
</style>