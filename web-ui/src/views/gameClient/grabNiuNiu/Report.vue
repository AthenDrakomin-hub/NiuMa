<template>
  <div class="report-container" ref="reportBox">
    <div class="header">
      <h2>暗涌牛盟 - 20局总结算榜单</h2>
      <p class="room-id">房间号: {{ roomId }} | 结算时间: {{ new Date().toLocaleString() }}</p>
    </div>

    <div class="report-table">
      <el-table :data="finalPlayers" style="width: 100%" :row-class-name="tableRowClassName">
        <el-table-column prop="nickname" label="玩家昵称" width="180">
          <template slot-scope="scope">
            <div style="display: flex; align-items: center;">
              <img :src="scope.row.avatar || defaultAvatar" style="width: 30px; height: 30px; border-radius: 50%; margin-right: 10px;" />
              {{ scope.row.nickname }}
            </div>
          </template>
        </el-table-column>
        <el-table-column prop="finalGold" label="最终净分 (已扣抽水)">
          <template slot-scope="scope">
            <span :class="scope.row.finalGold >= 0 ? 'win-text' : 'lose-text'">
              {{ scope.row.finalGold > 0 ? '+' : '' }}{{ scope.row.finalGold }}
            </span>
          </template>
        </el-table-column>
        <el-table-column prop="taxContribution" label="累计抽水贡献" />
        <el-table-column prop="maxWinStreak" label="最大连胜" />
      </el-table>
    </div>

    <div class="footer">
      <div class="qr-box">
        <canvas id="replay-qrcode"></canvas>
        <p>扫码查看 1-20 局每手牌回放防伪记录</p>
      </div>
    </div>
    
    <div class="export-btn" data-html2canvas-ignore>
      <el-button type="warning" size="large" @click="exportImage">长按/点击保存战报图片</el-button>
      <el-button type="info" size="large" @click="$router.push('/')">返回大厅</el-button>
    </div>
  </div>
</template>

<script>
import QRCode from 'qrcode'
import html2canvas from 'html2canvas'
import defaultAvatar from '@/assets/logo/logo.png'

export default {
  name: 'FinalReport',
  data() {
    return {
      defaultAvatar,
      roomId: this.$route.query.roomId || '7A92K1',
      finalPlayers: [
        // 测试数据，实际应由后端接口提供
        { nickname: '房主', avatar: '', finalGold: 1250, taxContribution: 0, maxWinStreak: 5 },
        { nickname: '玩家A', avatar: '', finalGold: -800, taxContribution: 120, maxWinStreak: 2 },
        { nickname: '玩家B', avatar: '', finalGold: -450, taxContribution: 50, maxWinStreak: 1 }
      ]
    }
  },
  mounted() {
    this.generateQRCode()
  },
  methods: {
    tableRowClassName({row}) {
      if (row.finalGold > 0) return 'success-row';
      return 'warning-row';
    },
    generateQRCode() {
      const canvas = document.getElementById('replay-qrcode')
      const url = `${window.location.origin}/replay/${this.roomId}`
      QRCode.toCanvas(canvas, url, {
        width: 120,
        margin: 1,
        color: { dark: '#000', light: '#fff' }
      }, (error) => {
        if (error) console.error(error)
      })
    },
    exportImage() {
      const el = this.$refs.reportBox
      html2canvas(el, {
        backgroundColor: '#121212',
        scale: 2
      }).then(canvas => {
        const link = document.createElement('a')
        link.download = `暗涌牛盟战报_${this.roomId}.png`
        link.href = canvas.toDataURL('image/png')
        link.click()
      })
    }
  }
}
</script>

<style scoped lang="scss">
.report-container {
  padding: 30px;
  background: #121212;
  color: #fff;
  min-height: 100vh;
  font-family: 'Helvetica Neue', Helvetica, 'PingFang SC', 'Hiragino Sans GB', 'Microsoft YaHei', Arial, sans-serif;

  .header {
    text-align: center;
    border-bottom: 2px solid #e6a23c;
    padding-bottom: 20px;
    margin-bottom: 30px;

    h2 { color: #e6a23c; margin: 0 0 10px 0; font-size: 28px; text-shadow: 0 0 10px rgba(230, 162, 60, 0.5); }
    .room-id { color: #999; font-size: 14px; }
  }

  .report-table {
    background: rgba(255, 255, 255, 0.05);
    border-radius: 8px;
    padding: 15px;

    ::v-deep .el-table {
      background: transparent;
      color: #ddd;
      
      th { background: rgba(0, 0, 0, 0.5); color: #e6a23c; border-bottom: 1px solid #444; }
      td { border-bottom: 1px solid #333; }
      tr { background: transparent; }
      tr:hover > td { background-color: rgba(230, 162, 60, 0.1) !important; }
    }

    .win-text { color: #f56c6c; font-weight: bold; font-size: 18px; }
    .lose-text { color: #67c23a; font-size: 16px; }
  }

  .footer {
    margin-top: 40px;
    display: flex;
    justify-content: center;

    .qr-box {
      text-align: center;
      background: #fff;
      padding: 10px 10px 0;
      border-radius: 8px;

      p { color: #333; font-size: 12px; margin: 5px 0; font-weight: bold; }
    }
  }

  .export-btn {
    text-align: center;
    margin-top: 50px;
  }
}
</style>