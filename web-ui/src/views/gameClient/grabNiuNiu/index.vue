<template>
  <div class="grab-niuniu-room">
    <div class="room-header">
      <div class="title">抢庄牛牛 (GameType: 1024)</div>
      <div class="status">当前状态: {{ getGameStateName(gameState) }}</div>
      <el-button type="danger" size="small" @click="exitRoom">退出房间</el-button>
    </div>

    <!-- 中间游戏区域 -->
    <div class="game-area">
      <!-- 庄家位置 -->
      <div v-if="bankerId" class="banker-info">
        当前庄家 ID: {{ bankerId }}
      </div>

      <!-- 等待/准备按钮 -->
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
import defaultAvatar from '@/assets/logo/logo.png' // 使用系统自带 logo 作为默认头像

export default {
  name: 'GrabNiuNiu',
  data() {
    return {
      defaultAvatar,
      gameState: 0, // 0: WaitStart, 1: Deal4, 2: GrabBanker, 3: Betting, 4: Deal1, 5: Compare, 6: Settlement
      myId: '', 
      mySeat: -1,
      bankerId: '',
      players: [],
      grabRates: [0, 1, 2, 3, 4], // 假设的抢庄倍数
      betRates: [1, 5, 10, 15, 20] // 假设的下注倍数
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
        this.$set(p, 'cards', msg.cards)
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
  background-color: #0b3d1f; // 绿色牌桌背景
  min-height: calc(100vh - 84px);
  color: #fff;
  padding: 20px;
  position: relative;

  .room-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    background: rgba(0, 0, 0, 0.5);
    padding: 10px 20px;
    border-radius: 8px;

    .title {
      font-size: 20px;
      font-weight: bold;
      color: #e6a23c;
    }
    .status {
      font-size: 16px;
      color: #67c23a;
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
      background: rgba(0, 0, 0, 0.6);
      padding: 10px 30px;
      border-radius: 20px;
    }

    .action-panel {
      display: flex;
      gap: 15px;
      background: rgba(0, 0, 0, 0.6);
      padding: 20px;
      border-radius: 12px;
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
      background: rgba(0, 0, 0, 0.4);
      border: 2px solid transparent;
      border-radius: 8px;
      padding: 15px;
      width: 220px;
      position: relative;
      transition: all 0.3s;

      &.is-me {
        border-color: #409eff;
        background: rgba(64, 158, 255, 0.1);
      }
      &.is-banker {
        border-color: #f56c6c;
        box-shadow: 0 0 15px rgba(245, 108, 108, 0.5);
      }

      .avatar img {
        width: 60px;
        height: 60px;
        border-radius: 50%;
        border: 2px solid #fff;
      }

      .info {
        margin-top: 10px;
        .name { font-weight: bold; }
        .gold { color: #e6a23c; }
        .ready-tag { color: #67c23a; font-size: 12px; margin-top: 5px; }
      }

      .cards {
        display: flex;
        gap: -20px; // 牌叠在一起
        margin-top: 15px;

        .card {
          width: 30px;
          height: 45px;
          background: #fff;
          color: #000;
          border-radius: 4px;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 14px;
          font-weight: bold;
          border: 1px solid #ccc;
          margin-right: 5px;
        }
      }

      .niu-result {
        position: absolute;
        top: -15px;
        right: -15px;
        background: #f56c6c;
        color: #fff;
        padding: 5px 10px;
        border-radius: 12px;
        font-weight: bold;
        transform: rotate(15deg);
      }
    }
  }
}
</style>