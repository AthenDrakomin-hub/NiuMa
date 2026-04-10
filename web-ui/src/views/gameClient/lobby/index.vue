<template>
  <div class="game-lobby">
    <div class="header">
      <h2>游戏大厅 (Game Lobby)</h2>
      <el-button type="primary" size="small" @click="$router.push('/')">返回主页</el-button>
    </div>

    <div class="game-list">
      <!-- 抢庄牛牛 -->
      <div class="game-card" @click="enterGame(1024, 'grab-niuniu')">
        <div class="game-icon grab-niuniu-icon"></div>
        <h3>抢庄牛牛</h3>
        <p>GameType: 1024</p>
      </div>

      <!-- 百人牛牛 -->
      <div class="game-card" @click="enterGame(1023, 'niuniu-100')">
        <div class="game-icon niuniu-100-icon"></div>
        <h3>百人牛牛</h3>
        <p>GameType: 1023</p>
      </div>

      <!-- 掼蛋 -->
      <div class="game-card" @click="enterGame(2001, 'guandan')">
        <div class="game-icon guandan-icon"></div>
        <h3>掼蛋</h3>
        <p>GameType: 2001</p>
      </div>
    </div>
  </div>
</template>

<script>
import { createGameRoom } from '@/api/gameClient'
import GameSocket from '@/utils/gameSocket'

export default {
  name: 'GameLobby',
  methods: {
    async enterGame(gameType, routeName) {
      try {
        const loading = this.$loading({
          lock: true,
          text: '正在连接游戏服务器...',
          spinner: 'el-icon-loading',
          background: 'rgba(0, 0, 0, 0.7)'
        })

        // 1. HTTP 请求获取 WebSocket 地址和房间号
        const res = await createGameRoom({ gameType, base64: '' })
        if (res.code !== '00000000') {
          loading.close()
          this.$message.error('创建房间失败: ' + res.msg)
          return
        }

        // 2. 连接 WebSocket
        try {
          await GameSocket.connect(res.wsAddress, res.venueId, gameType)
          loading.close()
          this.$message.success('连接游戏服务器成功！')
          
          // 3. 跳转到对应的游戏房间页面
          this.$router.push(`/game-client/${routeName}`)
        } catch (wsErr) {
          loading.close()
          this.$message.error('WebSocket 连接失败，请检查网络！')
        }
      } catch (err) {
        console.error(err)
        this.$message.error('请求服务器失败！')
      }
    }
  }
}
</script>

<style scoped lang="scss">
.game-lobby {
  padding: 20px;
  background-color: #1e1e1e;
  min-height: calc(100vh - 84px);
  color: #fff;

  .header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 40px;
    border-bottom: 1px solid #333;
    padding-bottom: 20px;

    h2 {
      margin: 0;
      color: #e6a23c;
    }
  }

  .game-list {
    display: flex;
    flex-wrap: wrap;
    gap: 20px;

    .game-card {
      width: 200px;
      height: 250px;
      background: #2a2a2a;
      border-radius: 12px;
      padding: 20px;
      text-align: center;
      cursor: pointer;
      transition: transform 0.2s, box-shadow 0.2s;
      border: 1px solid #444;

      &:hover {
        transform: translateY(-5px);
        box-shadow: 0 8px 16px rgba(230, 162, 60, 0.2);
        border-color: #e6a23c;
      }

      .game-icon {
        width: 100px;
        height: 100px;
        margin: 0 auto 20px;
        border-radius: 50%;
        background-color: #444;
        display: flex;
        align-items: center;
        justify-content: center;
        font-size: 40px;
        
        &.grab-niuniu-icon { background: linear-gradient(135deg, #f56c6c, #e6a23c); }
        &.niuniu-100-icon { background: linear-gradient(135deg, #67c23a, #409eff); }
        &.guandan-icon { background: linear-gradient(135deg, #409eff, #c03639); }
      }

      h3 {
        margin: 0 0 10px 0;
        color: #fff;
        font-size: 18px;
      }

      p {
        margin: 0;
        color: #999;
        font-size: 14px;
      }
    }
  }
}
</style>