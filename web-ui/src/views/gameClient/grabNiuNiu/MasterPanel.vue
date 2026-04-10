<template>
  <div class="master-control-panel">
    <div class="panel-header" @click="isCollapsed = !isCollapsed">
      <i class="el-icon-setting"></i> 上帝控制面板 (Master Control)
      <i :class="isCollapsed ? 'el-icon-arrow-down' : 'el-icon-arrow-up'" style="float: right; margin-top: 4px;"></i>
    </div>
    
    <div class="panel-body" v-show="!isCollapsed">
      <div v-if="players.length === 0" class="no-data">等待玩家加入...</div>
      
      <div v-for="p in players" :key="p.playerId" class="player-ctrl-row">
        <div class="p-info">
          <span class="p-name">{{ p.nickname }}</span>
          <span class="p-id">({{ p.playerId.slice(-4) }})</span>
        </div>
        
        <div class="p-cards" v-if="p.cards && p.cards.length > 0">
          <span v-for="(c, idx) in p.cards" :key="idx" class="card-mini">{{ formatCard(c) }}</span>
          <span v-if="p.pre5thCard !== undefined" class="card-mini pre-5th">{{ formatCard(p.pre5thCard) }}</span>
        </div>

        <div class="p-actions">
          <el-button size="mini" type="success" plain @click="sendCtrl(p.playerId, 'ForceWin')">强制牛牛</el-button>
          <el-button size="mini" type="danger" plain @click="sendCtrl(p.playerId, 'ForceLose')">全场点杀</el-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import GameSocket from '@/utils/gameSocket'

export default {
  name: 'MasterControlPanel',
  props: {
    players: {
      type: Array,
      default: () => []
    }
  },
  data() {
    return {
      isCollapsed: false
    }
  },
  methods: {
    formatCard(cardValue) {
      if (cardValue === 0) return '?'
      const suit = (cardValue & 0xF0) >> 4
      const rank = cardValue & 0x0F
      const suitNames = ['♦', '♣', '♥', '♠']
      const rankNames = ['0', 'A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K']
      return `${suitNames[suit]}${rankNames[rank]}`
    },
    sendCtrl(playerId, action) {
      this.$confirm(`确定对玩家 ${playerId} 执行 [${action}] 操作吗？`, '高危操作', {
        type: 'warning'
      }).then(() => {
        // 实际项目中发送控制指令到后端
        // GameSocket.sendMessage('MsgMasterControl', { targetId: playerId, action: action })
        this.$message.success(`已向服务器发送指令: ${action}`)
      }).catch(() => {})
    }
  }
}
</script>

<style scoped lang="scss">
.master-control-panel {
  position: absolute;
  top: 20px;
  right: 20px;
  width: 380px;
  background: rgba(20, 20, 20, 0.95);
  border: 1px solid #f56c6c;
  border-radius: 8px;
  z-index: 999;
  box-shadow: 0 0 20px rgba(245, 108, 108, 0.3);

  .panel-header {
    background: linear-gradient(90deg, #f56c6c, #c03639);
    color: #fff;
    padding: 10px 15px;
    font-weight: bold;
    border-radius: 7px 7px 0 0;
    cursor: pointer;
  }

  .panel-body {
    padding: 15px;
    max-height: 400px;
    overflow-y: auto;

    .no-data {
      color: #999;
      text-align: center;
      padding: 20px;
    }

    .player-ctrl-row {
      display: flex;
      flex-direction: column;
      margin-bottom: 15px;
      padding-bottom: 15px;
      border-bottom: 1px dashed #444;

      &:last-child {
        margin-bottom: 0;
        padding-bottom: 0;
        border-bottom: none;
      }

      .p-info {
        margin-bottom: 5px;
        .p-name { color: #e6a23c; font-weight: bold; }
        .p-id { color: #999; font-size: 12px; margin-left: 5px; }
      }

      .p-cards {
        display: flex;
        gap: 5px;
        margin-bottom: 10px;

        .card-mini {
          background: #fff;
          color: #000;
          padding: 2px 6px;
          border-radius: 3px;
          font-size: 12px;
          font-weight: bold;

          &.pre-5th {
            background: #f56c6c;
            color: #fff;
            border: 1px solid #ff9999;
            box-shadow: 0 0 5px #f56c6c;
          }
        }
      }

      .p-actions {
        display: flex;
        justify-content: flex-end;
      }
    }
  }
}
</style>