import request from '@/utils/request'

export function createGameRoom(data) {
  return request({
    url: '/player/game/create',
    method: 'post',
    data: data
  })
}

export function enterGameRoom(data) {
  return request({
    url: '/player/game/enter',
    method: 'post',
    data: data
  })
}
