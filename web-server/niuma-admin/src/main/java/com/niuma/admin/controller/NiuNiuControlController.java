package com.niuma.admin.controller;

import com.niuma.admin.common.core.domain.AjaxResult;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/**
 * 看牌抢庄牛牛控制 Controller
 */
@RestController
@RequestMapping("/niuniu/control")
public class NiuNiuControlController {

    @Autowired
    private RedisTemplate<String, Object> redisTemplate;

    /**
     * 预设牌局发牌结果
     * @param params 包含 venueId, seat, cards (例如 [12, 15, 34, 45, 2])
     */
    @PostMapping("/preset")
    public AjaxResult presetCards(@RequestBody Map<String, Object> params) {
        String venueId = (String) params.get("venueId");
        Integer seat = (Integer) params.get("seat");
        List<Integer> cards = (List<Integer>) params.get("cards");

        if (venueId == null || seat == null || cards == null || cards.isEmpty()) {
            return AjaxResult.error("参数不完整，必须包含 venueId, seat, cards");
        }

        // 写入 Redis，有效期 30 分钟。键名格式: NiuMa:PresetCard:VenueID:{venueId}:Seat:{seat}
        String redisKey = "NiuMa:PresetCard:VenueID:" + venueId + ":Seat:" + seat;
        
        try {
            redisTemplate.opsForValue().set(redisKey, cards, 30, TimeUnit.MINUTES);
            return AjaxResult.success("发牌预设成功，游戏服将在下一局发牌时读取此配置");
        } catch (Exception e) {
            return AjaxResult.error("Redis 写入失败：" + e.getMessage());
        }
    }

    /**
     * 清除预设
     */
    @DeleteMapping("/preset/{venueId}/{seat}")
    public AjaxResult clearPreset(@PathVariable String venueId, @PathVariable Integer seat) {
        String redisKey = "NiuMa:PresetCard:VenueID:" + venueId + ":Seat:" + seat;
        redisTemplate.delete(redisKey);
        return AjaxResult.success("预设清除成功");
    }
}
