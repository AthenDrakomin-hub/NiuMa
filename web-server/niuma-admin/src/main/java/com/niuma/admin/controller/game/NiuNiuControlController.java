package com.niuma.admin.controller.game;

import com.niuma.common.core.domain.AjaxResult;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.redis.core.RedisTemplate;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;

@RestController
@RequestMapping("/game/niuniu")
public class NiuNiuControlController {

    @Autowired
    private RedisTemplate<String, String> redisTemplate;

    @PostMapping("/control/preset")
    public AjaxResult setPresetCard(@RequestBody Map<String, Object> params) {
        String venueId = (String) params.get("venueId");
        Integer seatId = (Integer) params.get("seatId");
        Integer cardId = (Integer) params.get("cardId");

        if (venueId == null || seatId == null || cardId == null) {
            return AjaxResult.error("Missing parameters");
        }

        String redisKey = "PRESET_CARD:" + venueId + ":" + seatId;
        redisTemplate.opsForValue().set(redisKey, String.valueOf(cardId));
        
        return AjaxResult.success("Card preset successfully.");
    }
}
