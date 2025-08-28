class ReconnectWebSocket {
    constructor(url, options = {}) {
        this.url = url;
        this.reconnectDelay = options.reconnectDelay || 3000; // 重连间隔
        this.maxRetries = options.maxRetries || Infinity;    // 最大重试次数
        this.retries = 0;
        this.connect();
    }

    connect() {
        this.ws = new WebSocket(this.url);

        this.ws.onopen = () => {
            console.log("✅ 已连接");
            this.retries = 0; // 连接成功，重置重试次数
            this.send({event: 'connect', data: {msg: "Hello, I am client"}});
        };

        this.ws.onmessage = (e) => {
            console.log("📩 收到:", e.data);
        };

        this.ws.onerror = (err) => {
            console.error("❌ 错误:", err);
        };

        this.ws.onclose = () => {
            console.log("🔌 连接关闭");
            if (this.retries < this.maxRetries) {
                    this.retries++;
                    console.log(`⏳ ${this.reconnectDelay / 1000}s 后重试 (${this.retries})`);
                    setTimeout(() => this.connect(), this.reconnectDelay);
            }
        };
    }

    send(data) {
        if (this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(JSON.stringify(data));
        } else {
            console.warn("⚠️ 连接未建立，消息丢弃:", data);
        }
    }

    close() {
        this.maxRetries = 0; // 停止重连
        this.ws.close();
    }
}

// 使用
export const wssocket = new ReconnectWebSocket("ws://localhost:9002");

