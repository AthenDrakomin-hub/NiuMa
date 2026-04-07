<template>
  <div class="app-container">
    <el-card>
      <div slot="header" class="clearfix">
        <span>预设卡牌控制</span>
      </div>
      <el-form :model="form" label-width="120px">
        <el-form-item label="房间ID (Venue ID)">
          <el-input v-model="form.venueId" placeholder="请输入房间ID" />
        </el-form-item>
        <el-form-item label="座位号 (Seat ID)">
          <el-input v-model.number="form.seatId" placeholder="请输入座位号(0-5)" type="number" />
        </el-form-item>
        <el-form-item label="预设卡牌 (Card ID)">
          <el-select v-model="form.cardId" placeholder="请选择预设卡牌">
            <el-option v-for="i in 52" :key="i" :label="'Card ' + i" :value="i" />
          </el-select>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="submitControl">提交预设</el-button>
        </el-form-item>
      </el-form>
    </el-card>
  </div>
</template>

<script>
import request from '@/utils/request'

export default {
  data() {
    return {
      form: {
        venueId: '',
        seatId: null,
        cardId: null
      }
    }
  },
  methods: {
    submitControl() {
      if (!this.form.venueId || this.form.seatId === null || this.form.cardId === null) {
        this.$message.error('请填写完整信息');
        return;
      }
      request({
        url: '/game/niuniu/control/preset',
        method: 'post',
        data: this.form
      }).then(res => {
        this.$message.success('预设成功');
      });
    }
  }
}
</script>
