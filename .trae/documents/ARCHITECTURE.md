# 桌面日历小部件 - 技术架构文档

## 1. 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| 桌面框架 | Electron 28+ | 透明窗口支持成熟 |
| 前端 | HTML5 + CSS3 + Vanilla JS | 轻量无框架依赖 |
| 数据存储 | JSON (data.json) | 本地文件持久化 |

## 2. 项目结构

```
/workspace/
├── package.json          # npm配置，Electron启动脚本
├── main.js               # 主进程：创建窗口、读取数据
├── preload.js            # 安全桥接：暴露fs操作
├── index.html            # 页面结构
├── style.css             # 样式：宫崎骏浅绿风格
├── renderer.js           # 渲染逻辑：任务/倒计时CRUD
├── data.json             # 数据文件（自动创建）
└── .trae/documents/      # 文档目录
    ├── PRD.md
    └── ARCHITECTURE.md
```

## 3. 窗口配置

```javascript
{
  width: 340,
  height: 520,
  transparent: true,       // 透明背景
  frame: false,           // 无边框
  alwaysOnTop: true,      // 置顶
  resizable: false,       // 固定尺寸
  skipTaskbar: true,      // 不显示任务栏
  webPreferences: {
    preload: 'preload.js',
    contextIsolation: true,
    nodeIntegration: false
  }
}
```

## 4. 数据模型

### 4.1 data.json 结构

```json
{
  "tasks": [
    {
      "id": 1719123456789,
      "text": "每日任务内容",
      "completed": false,
      "createdAt": "2024-06-23"
    }
  ],
  "countdowns": [
    {
      "id": 1719123456789,
      "name": "中考",
      "targetDate": "2024-06-25",
      "createdAt": "2024-06-23"
    }
  ],
  "position": {
    "x": 100,
    "y": 100
  }
}
```

## 5. 模块职责

| 模块 | 职责 |
|------|------|
| main.js | 创建BrowserWindow，处理窗口事件，初始化数据文件 |
| preload.js | 暴露安全的fs API给渲染进程 |
| renderer.js | 绑定DOM事件，执行任务/倒计时增删改查 |
| style.css | 磨砂玻璃效果，宫崎骏配色，动画过渡 |

## 6. 关键实现

### 6.1 透明窗口 + 磨砂玻璃
```css
background: rgba(232, 245, 233, 0.85);
backdrop-filter: blur(12px);
border-radius: 20px;
box-shadow: 0 8px 32px rgba(129, 199, 132, 0.25);
```

### 6.2 拖拽窗口
```javascript
// 整个窗口可拖拽（无frame时）
document.body.style.cssText = '-webkit-app-region: drag';
```

### 6.3 数据持久化
- 渲染进程通过 preload 暴露的 API 读写 data.json
- 每次修改后自动保存
- 启动时自动加载

## 7. 依赖

```json
{
  "electron": "^28.0.0"
}
```
