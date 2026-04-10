<template>
  <div class="squeeze-card-container">
    <div 
      class="card-wrapper" 
      @touchstart="onTouchStart" 
      @touchmove="onTouchMove" 
      @touchend="onTouchEnd"
      :style="{ transform: `translateY(${offsetY}px)` }"
    >
      <!-- 牌底 (牌面内容) -->
      <div class="card-face">
        <div class="value" :class="getSuitColor(cardValue)">{{ formatCard(cardValue) }}</div>
      </div>
      <!-- 遮罩盖板 (牌背) -->
      <div 
        class="card-back" 
        :style="{ height: `${100 - (offsetY / maxOffset * 100)}%` }"
      >
        <div class="pattern"></div>
      </div>
    </div>
    <div class="hint" v-if="offsetY === 0">向上滑动搓牌</div>
  </div>
</template>

<script>
export default {
  name: 'SqueezeCard',
  props: {
    cardValue: {
      type: Number,
      required: true
    }
  },
  data() {
    return {
      startY: 0,
      offsetY: 0,
      maxOffset: 120, // 最大滑动距离 (露出全牌)
      isRevealed: false
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
    getSuitColor(cardValue) {
      if (cardValue === 0) return ''
      const suit = (cardValue & 0xF0) >> 4
      return (suit === 0 || suit === 2) ? 'red' : 'black' // 方块和红桃是红色
    },
    onTouchStart(e) {
      if (this.isRevealed) return
      this.startY = e.touches[0].clientY
    },
    onTouchMove(e) {
      if (this.isRevealed) return
      const currentY = e.touches[0].clientY
      let diff = this.startY - currentY
      if (diff < 0) diff = 0
      if (diff > this.maxOffset) diff = this.maxOffset

      this.offsetY = diff

      // 露出超过 50% 且是关键牌时触发震动反馈
      if (this.offsetY > this.maxOffset * 0.5) {
        this.triggerVibration()
      }
    },
    onTouchEnd() {
      if (this.isRevealed) return
      if (this.offsetY > this.maxOffset * 0.6) {
        // 搓开了
        this.offsetY = this.maxOffset
        this.isRevealed = true
        this.$emit('revealed', this.cardValue)
      } else {
        // 没搓开，弹回去
        this.offsetY = 0
      }
    },
    triggerVibration() {
      // 节流处理，防止持续震动
      if (this.vibrated) return
      this.vibrated = true
      setTimeout(() => { this.vibrated = false }, 500)

      // 如果是 J, Q, K 或 10，给高频短促震动
      const rank = this.cardValue & 0x0F
      if (rank >= 10 && rank <= 13) {
        if (navigator.vibrate) {
          navigator.vibrate([50, 30, 50])
        }
      } else {
        if (navigator.vibrate) {
          navigator.vibrate(30)
        }
      }
    }
  }
}
</script>

<style scoped lang="scss">
.squeeze-card-container {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  margin-top: 20px;

  .card-wrapper {
    width: 100px;
    height: 140px;
    position: relative;
    border-radius: 8px;
    overflow: hidden;
    box-shadow: 0 10px 20px rgba(0, 0, 0, 0.5);
    background: #fff;
    touch-action: none; // 防止滚动屏幕
  }

  .card-face {
    width: 100%;
    height: 100%;
    position: absolute;
    top: 0;
    left: 0;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 32px;
    font-weight: bold;
    
    .red { color: #f56c6c; }
    .black { color: #333; }
  }

  .card-back {
    width: 100%;
    position: absolute;
    bottom: 0; // 从底部开始覆盖
    left: 0;
    background: #e6a23c;
    border-top: 2px solid rgba(255, 255, 255, 0.5);
    transition: height 0.1s;

    .pattern {
      width: 100%;
      height: 100%;
      background-image: repeating-linear-gradient(45deg, transparent, transparent 5px, rgba(0,0,0,0.1) 5px, rgba(0,0,0,0.1) 10px);
    }
  }

  .hint {
    margin-top: 20px;
    color: #e6a23c;
    font-size: 14px;
    animation: blink 1s infinite alternate;
  }

  @keyframes blink {
    from { opacity: 0.4; }
    to { opacity: 1; }
  }
}
</style>