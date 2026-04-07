package com.niuma.admin.controller.common;

import org.springframework.web.bind.annotation.*;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.web.client.RestTemplate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.io.Resource;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;

import java.io.IOException;
import java.net.URI;

/**
 * 图片代理控制器 - 解决 WebGL 跨域图片问题
 * @author system
 * @date 2024.04.07
 */
@RestController
@RequestMapping("/proxy")
public class ImageProxyController {

    @Autowired
    private RestTemplate restTemplate;

    /**
     * 图片代理接口
     * @param url 图片 URL
     * @return 图片数据
     */
    @GetMapping("/image")
    public ResponseEntity<Resource> proxyImage(@RequestParam String url) {
        try {
            // 解码 URL
            String decodedUrl = java.net.URLDecoder.decode(url, "UTF-8");

            // 验证 URL 格式，防止 SSRF 攻击
            if (!isValidUrl(decodedUrl)) {
                return ResponseEntity.badRequest().build();
            }

            // 请求远程图片
            ResponseEntity<byte[]> response = restTemplate.getForEntity(
                URI.create(decodedUrl),
                byte[].class
            );

            // 获取原始 Content-Type
            MediaType contentType = response.getHeaders().getContentType();
            if (contentType == null) {
                contentType = MediaType.IMAGE_JPEG;
            }

            // 创建响应头
            HttpHeaders headers = new HttpHeaders();
            headers.setContentType(contentType);
            headers.setCacheControl("public, max-age=3600");
            headers.setAccessControlAllowOrigin("*");

            // 返回图片数据
            ByteArrayResource resource = new ByteArrayResource(response.getBody());
            return ResponseEntity.ok()
                .headers(headers)
                .body(resource);

        } catch (HttpClientErrorException | HttpServerErrorException e) {
            return ResponseEntity.status(e.getStatusCode()).build();
        } catch (Exception e) {
            return ResponseEntity.internalServerError().build();
        }
    }

    /**
     * 验证 URL 是否合法，防止 SSRF 攻击
     */
    private boolean isValidUrl(String url) {
        try {
            URI uri = URI.create(url);
            String scheme = uri.getScheme();

            // 只允许 http 和 https
            if (!"http".equals(scheme) && !"https".equals(scheme)) {
                return false;
            }

            String host = uri.getHost();

            // 阻止内网地址
            if (host == null) {
                return false;
            }

            // 阻止 localhost 和内网 IP
            String[] blockedHosts = {
                "localhost", "127.0.0.1", "0.0.0.0",
                "::1", "localhost.localdomain"
            };

            for (String blocked : blockedHosts) {
                if (host.equalsIgnoreCase(blocked)) {
                    return false;
                }
            }

            // 阻止内网 IP 段
            String[] blockedPatterns = {
                "10.", "172.16.", "172.17.", "172.18.", "172.19.",
                "172.20.", "172.21.", "172.22.", "172.23.", "172.24.",
                "172.25.", "172.26.", "172.27.", "172.28.", "172.29.",
                "172.30.", "172.31.", "192.168."
            };

            for (String pattern : blockedPatterns) {
                if (host.startsWith(pattern)) {
                    return false;
                }
            }

            return true;
        } catch (Exception e) {
            return false;
        }
    }
}
