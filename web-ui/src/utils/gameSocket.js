import msgpack from 'msgpack-lite'
import { Base64 } from 'js-base64'
import CryptoJS from 'crypto-js'

class GameSocket {
  constructor() {
    this.ws = null
    this.isConnected = false
    this.callbacks = {}
    this.venueId = ''
    this.gameType = 0
    this.nonceCount = 0
    this.key = '90f9b69b-8197-40fa-b166-4199c0892018' // 与 GameManager.ts 保持一致
  }

  generateNonce() {
    this.nonceCount++
    const timestamp = Date.now().toString()
    const countStr = this.nonceCount.toString().padStart(6, '0')
    const randomStr = Math.floor(Math.random() * 1000000).toString().padStart(6, '0')
    return `${timestamp}${countStr}${randomStr}`
  }

  signatureMessage(msgType, jsonMsg, nonce) {
    const sortedKeys = Object.keys(jsonMsg).sort()
    let rawStr = ''
    for (const key of sortedKeys) {
      if (jsonMsg[key] !== null && jsonMsg[key] !== undefined && jsonMsg[key] !== '') {
        rawStr += `${key}=${jsonMsg[key]}&`
      }
    }
    rawStr += `msgType=${msgType}&nonce=${nonce}&key=${this.key}`
    return CryptoJS.MD5(rawStr).toString()
  }

  connect(wsAddress, venueId, gameType) {
    this.venueId = venueId
    this.gameType = gameType

    return new Promise((resolve, reject) => {
      this.ws = new WebSocket(wsAddress)
      this.ws.binaryType = 'arraybuffer'

      this.ws.onopen = () => {
        this.isConnected = true
        console.log('WebSocket Connected!')

        // 发送心跳和进入房间的包
        this.sendMessage('MsgPlayerConnect', {})
        this.sendMessage('MsgEnterVenue', { venueId: this.venueId, gameType: this.gameType })
        
        resolve()
      }

      this.ws.onmessage = (event) => {
        const buffer = new Uint8Array(event.data)
        const decodedMsg = msgpack.decode(buffer)
        const msgType = decodedMsg.msgType
        const msgData = JSON.parse(decodedMsg.jsonMsg)

        console.log(`[WS Receive] ${msgType}:`, msgData)

        if (this.callbacks[msgType]) {
          this.callbacks[msgType].forEach(cb => cb(msgData))
        }
      }

      this.ws.onerror = (error) => {
        console.error('WebSocket Error: ', error)
        reject(error)
      }

      this.ws.onclose = () => {
        console.log('WebSocket Closed')
        this.isConnected = false
      }
    })
  }

  sendMessage(msgType, jsonMsg, useSignature = true) {
    if (!this.isConnected) {
      console.error('WebSocket is not connected!')
      return
    }

    const finalMsg = { ...jsonMsg }
    if (useSignature) {
      const nonce = this.generateNonce()
      const sign = this.signatureMessage(msgType, finalMsg, nonce)
      finalMsg.nonce = nonce
      finalMsg.sign = sign
    }

    const payload = {
      msgType: msgType,
      jsonMsg: JSON.stringify(finalMsg)
    }

    const encodedBuffer = msgpack.encode(payload)
    console.log(`[WS Send] ${msgType}:`, payload)
    this.ws.send(encodedBuffer)
  }

  on(msgType, callback) {
    if (!this.callbacks[msgType]) {
      this.callbacks[msgType] = []
    }
    this.callbacks[msgType].push(callback)
  }

  off(msgType, callback) {
    if (!this.callbacks[msgType]) return
    if (!callback) {
      this.callbacks[msgType] = []
    } else {
      this.callbacks[msgType] = this.callbacks[msgType].filter(cb => cb !== callback)
    }
  }

  close() {
    if (this.ws) {
      this.ws.close()
    }
  }
}

export default new GameSocket()
