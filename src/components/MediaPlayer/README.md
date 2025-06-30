# MediaPlayer 组件

## 概述
MediaPlayer 是一个基于 Vue 3 的视频播放器组件，支持 MP4 格式的视频播放。它提供了完整的播放控制功能，包括播放/暂停、停止、进度条控制和时间显示。

## 组件结构

### 1. MediaPlayer (主组件)
- **文件路径**: `src/components/MediaPlayer/index.vue`
- **职责**: 管理播放器状态，处理视频加载和播放逻辑
- **核心功能**:
  - 视频源加载
  - 播放状态管理
  - 时间进度追踪
  - 事件处理和状态同步

### 2. MediaControl (控制组件)
- **文件路径**: `src/components/MediaPlayer/MediaControl/index.vue`
- **职责**: 提供用户交互界面
- **核心功能**:
  - 播放/暂停/停止按钮
  - 进度条显示和点击跳转
  - 时间显示（当前时间/总时长）

## 数据流向

```
视频文件 → MediaPlayer → WebAVPlayer/VideoPlayer → 渲染器
    ↓
播放状态 → MediaControl → 用户界面
    ↓
用户操作 → 事件回调 → 播放器控制
```

## 使用方法

### 基本用法

```vue
<template>
  <MediaPlayer :source="videoSource" />
</template>

<script setup>
import MediaPlayer from '@/components/MediaPlayer/index.vue'

const videoSource = '/path/to/video.mp4'
</script>
```

### Props

| 属性名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| source | `string \| ReadableStream<Uint8Array>` | - | 视频源文件路径或流 |

### 事件

| 事件名 | 参数 | 说明 |
|--------|------|------|
| ready | - | 视频加载完成，可以播放 |
| play | - | 开始播放 |
| pause | - | 暂停播放 |
| stop | - | 停止播放 |
| ended | - | 播放结束 |

## 核心设计模式

### 1. 状态管理模式
使用 Vue 的响应式系统管理播放器状态：

```typescript
interface PlayerState {
  isReady: boolean;      // 播放器是否就绪
  isPlaying: boolean;    // 是否正在播放
  currentTime: number;   // 当前播放时间（微秒）
  duration: number;      // 视频总时长（微秒）
}
```

### 2. 事件驱动模式
通过事件系统实现组件间通信：

```typescript
// 播放器事件监听
player.on('ready', () => {
  playerState.isReady = true;
  playerState.duration = player.getDurationUs();
});

// 控制组件事件发射
emit('seek', seekTime);
```

### 3. 适配器模式
通过 IPlayer 接口统一不同播放器实现：

```typescript
interface IPlayer {
  load(source: string | ReadableStream<Uint8Array>): Promise<void>;
  play(): void;
  pause(): void;
  stop(): void;
  seek(timeUs: number): void;
  getCurrentTimeUs(): number;
  getDurationUs(): number;
}
```

## 进度条更新修复

### 问题描述
之前的进度条存在以下问题：
1. `updateCurrentTime` 函数被注释，导致进度条不更新
2. 缺少获取视频总时长的方法
3. **关键问题：WebAVPlayer 中的时间单位转换错误**
4. 播放时间无法实时更新
5. 进度条点击跳转功能异常

### 解决方案

#### 1. 恢复时间更新函数
```typescript
const updateCurrentTime = () => {
    if (player && playerState.isReady) {
        playerState.currentTime = player.getCurrentTimeUs();
        requestAnimationFrame(updateCurrentTime);
    }
};
```

#### 2. 添加时长获取方法
在 `IPlayer` 接口中添加：
```typescript
getDurationUs(): number;
```

在 `WebAVPlayer` 中实现：
```typescript
getDurationUs(): number {
    if (this.clip && (this.clip as any).meta?.duration) {
        return Math.round((this.clip as any).meta.duration * 1000000);
    }
    return 30000000; // 30秒默认值
}
```

#### 3. **修复关键的时间单位转换错误**
这是最重要的修复！之前的代码中存在时间单位转换错误：

**错误的实现：**
```typescript
// ❌ 错误：timeUs 是微秒，但只除以1000变成毫秒
this.startTime = performance.now() - timeUs / 1000 / this.playbackRate;
```

**正确的实现：**
```typescript
// ✅ 正确：timeUs 是微秒，需要除以1000000转换为秒
seek(timeUs: number): void {
    this.currentTimeUs = timeUs;
    if(this.playing){
        this.startTime = performance.now() - (timeUs / 1000000) / this.playbackRate;
    }
}

setPlaybackRate(rate: number): void {
    this.playbackRate = rate;
    if (this.playing) {
        this.startTime = performance.now() - (this.currentTimeUs / 1000000) / this.playbackRate;
    }
}
```

#### 4. 改进状态同步
```typescript
player.on('ready', () => {
    playerState.isReady = true;
    playerState.duration = player.getDurationUs();
    updateCurrentTime(); // 开始时间更新循环
});
```

#### 5. 添加调试信息
为了便于问题排查，添加了详细的控制台日志：
```typescript
// MediaControl 中的进度计算
const progressPercentage = computed(() => {
    console.log('Progress calculation:', {
        currentTime: props.playerState.currentTime,
        duration: props.playerState.duration,
        percentage: /* 计算结果 */
    });
    // ...
});

// 跳转处理
const handleSeekClick = (event: MouseEvent) => {
    console.log('Seek details:', {
        clickX, width: rect.width, percentage, seekTime, duration
    });
    // ...
};
```

### 测试方法

使用以下 Vue 模板测试功能：

```vue
<template>
  <div>
    <MediaPlayer :source="videoSource" />
    <div class="debug-info">
      <p>当前时间: {{ formatTime(playerState.currentTime) }}</p>
      <p>总时长: {{ formatTime(playerState.duration) }}</p>
      <p>进度: {{ progressPercentage.toFixed(1) }}%</p>
    </div>
  </div>
</template>
```

### 重要注意事项

⚠️ **时间单位统一性**：整个系统使用微秒（microseconds）作为时间单位
- `currentTime`: 微秒
- `duration`: 微秒  
- `seek(timeUs)`: 参数为微秒
- 显示时需要除以 1,000,000 转换为秒

⚠️ **性能考虑**：使用 `requestAnimationFrame` 进行时间更新，确保流畅的动画效果

⚠️ **错误处理**：添加了时长获取失败的默认值处理，防止组件崩溃

## 扩展建议

### 1. 添加更多控制功能
- 音量控制
- 播放速度调节
- 全屏模式
- 画中画模式

### 2. 改进用户体验
- 键盘快捷键支持
- 鼠标悬停显示预览
- 拖拽进度条
- 缓冲进度显示

### 3. 增强播放器功能
- 字幕支持
- 多音轨切换
- 视频质量选择
- 播放列表管理

### 4. 性能优化
- 视频预加载策略
- 内存使用优化
- 错误恢复机制
- 网络状态适配

## 重要提醒

1. **依赖管理**: 确保 `@avcore` 模块正确安装和配置
2. **浏览器兼容性**: 某些功能可能需要现代浏览器支持
3. **文件格式**: 目前主要支持 MP4 格式
4. **错误处理**: 建议添加适当的错误边界和用户提示
5. **时间单位**: 系统内部统一使用微秒，显示时转换为秒