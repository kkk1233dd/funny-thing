// 宫崎骏风格桌面日历小部件 - 渲染进程逻辑 (浏览器版本)

const DATA_KEY = 'desktop_calendar_data';

// 农历数据
const lunarCalendar = {
  0: '初一', 1: '初二', 2: '初三', 3: '初四', 4: '初五',
  5: '初六', 6: '初七', 7: '初八', 8: '初九', 9: '初十',
  10: '十一', 11: '十二', 12: '十三', 13: '十四', 14: '十五',
  15: '十六', 16: '十七', 17: '十八', 18: '十九', 19: '二十',
  20: '廿一', 21: '廿二', 22: '廿三', 23: '廿四', 24: '廿五',
  25: '廿六', 26: '廿七', 27: '廿八', 28: '廿九', 29: '三十'
};

const lunarMonth = ['正', '二', '三', '四', '五', '六', '七', '八', '九', '十', '冬', '腊'];
const weekDays = ['星期日', '星期一', '星期二', '星期三', '星期四', '星期五', '星期六'];

// 季节提示
const seasonHints = {
  spring: '🌸 春暖花开',
  summer: '☀️ 夏日蝉鸣',
  autumn: '🍂 秋高气爽',
  winter: '❄️ 冬日暖阳'
};

// 全局数据
let appData = {
  tasks: [],
  countdowns: [],
  position: { x: 100, y: 100 }
};

// 初始化
document.addEventListener('DOMContentLoaded', () => {
  initDate();
  loadData();
  renderTasks();
  renderCountdowns();
  setupEventListeners();
  startCountdownUpdate();
  loadPosition();
});

// 初始化日期显示
function initDate() {
  const now = new Date();
  const dateNumber = document.getElementById('dateNumber');
  const dateWeek = document.getElementById('dateWeek');
  const dateMonth = document.getElementById('dateMonth');
  const lunarDate = document.getElementById('lunarDate');
  const seasonHint = document.getElementById('seasonHint');

  dateNumber.textContent = now.getDate();
  dateWeek.textContent = weekDays[now.getDay()];
  dateMonth.textContent = `${now.getFullYear()}年${now.getMonth() + 1}月`;

  // 计算农历（简化版，基于实际日期偏移）
  const lunarInfo = calculateLunar(now);
  lunarDate.textContent = `农历${lunarMonth[lunarInfo.month - 1]}月${lunarCalendar[lunarInfo.day - 1]}`;

  // 季节提示
  const month = now.getMonth() + 1;
  if (month >= 3 && month <= 5) {
    seasonHint.textContent = seasonHints.spring;
  } else if (month >= 6 && month <= 8) {
    seasonHint.textContent = seasonHints.summer;
  } else if (month >= 9 && month <= 11) {
    seasonHint.textContent = seasonHints.autumn;
  } else {
    seasonHint.textContent = seasonHints.winter;
  }
}

// 简化农历计算
function calculateLunar(date) {
  // 使用一个简化的算法
  const startDate = new Date(2024, 0, 1); // 2024年春节是2月10日
  const springFestival = new Date(2024, 1, 10);
  const diffDays = Math.floor((date - startDate) / (1000 * 60 * 60 * 24));

  // 粗略计算
  let lunarDay = (diffDays % 30) + 1;
  let lunarMonth = Math.floor(diffDays / 30) + 1;

  if (lunarDay < 1) lunarDay = 30 + lunarDay;
  if (lunarMonth > 12) lunarMonth = lunarMonth - 12;

  return { month: lunarMonth || 12, day: lunarDay };
}

// 加载数据
function loadData() {
  try {
    const saved = localStorage.getItem(DATA_KEY);
    if (saved) {
      appData = JSON.parse(saved);
    }
  } catch (e) {
    console.error('加载数据失败:', e);
  }
}

// 保存数据
function saveData() {
  try {
    localStorage.setItem(DATA_KEY, JSON.stringify(appData));
  } catch (e) {
    console.error('保存数据失败:', e);
  }
}

// 加载位置
function loadPosition() {
  const container = document.getElementById('widgetContainer');
  if (appData.position) {
    container.style.left = appData.position.x + 'px';
    container.style.top = appData.position.y + 'px';
  }
}

// 渲染任务列表
function renderTasks() {
  const taskList = document.getElementById('taskList');

  if (!appData.tasks || appData.tasks.length === 0) {
    taskList.innerHTML = '<div class="empty-state">还没有任务，点击 + 添加</div>';
    return;
  }

  taskList.innerHTML = appData.tasks.map(task => `
    <div class="task-item ${task.completed ? 'completed' : ''}" data-id="${task.id}">
      <div class="task-checkbox ${task.completed ? 'checked' : ''}" data-id="${task.id}"></div>
      <span class="task-text" data-id="${task.id}">${escapeHtml(task.text)}</span>
      <button class="task-delete" data-id="${task.id}">×</button>
    </div>
  `).join('');
}

// 渲染倒计时列表
function renderCountdowns() {
  const countdownList = document.getElementById('countdownList');

  if (!appData.countdowns || appData.countdowns.length === 0) {
    countdownList.innerHTML = '<div class="empty-state">还没有倒计时，点击 + 添加</div>';
    return;
  }

  countdownList.innerHTML = appData.countdowns.map(countdown => {
    const daysLeft = calculateDaysLeft(countdown.targetDate);
    return `
      <div class="countdown-item" data-id="${countdown.id}">
        <div class="countdown-icon">⏰</div>
        <div class="countdown-info">
          <div class="countdown-name">${escapeHtml(countdown.name)}</div>
          <div class="countdown-date">${formatDate(countdown.targetDate)}</div>
        </div>
        <div class="countdown-days">
          <div class="countdown-number">${daysLeft}</div>
          <div class="countdown-unit">天</div>
        </div>
        <button class="countdown-delete" data-id="${countdown.id}">×</button>
      </div>
    `;
  }).join('');
}

// 计算剩余天数
function calculateDaysLeft(targetDate) {
  const now = new Date();
  now.setHours(0, 0, 0, 0);
  const target = new Date(targetDate);
  target.setHours(0, 0, 0, 0);
  const diff = target - now;
  const days = Math.ceil(diff / (1000 * 60 * 60 * 24));
  return days >= 0 ? days : 0;
}

// 格式化日期
function formatDate(dateStr) {
  const date = new Date(dateStr);
  return `${date.getMonth() + 1}月${date.getDate()}日`;
}

// HTML转义
function escapeHtml(text) {
  const div = document.createElement('div');
  div.textContent = text;
  return div.innerHTML;
}

// 添加任务
function addTask(text) {
  if (!text.trim()) return;

  if (!appData.tasks) appData.tasks = [];

  const task = {
    id: Date.now(),
    text: text.trim(),
    completed: false,
    createdAt: new Date().toISOString().split('T')[0]
  };

  appData.tasks.push(task);
  saveData();
  renderTasks();
}

// 切换任务状态
function toggleTask(id) {
  const task = appData.tasks.find(t => t.id === id);
  if (task) {
    task.completed = !task.completed;
    saveData();
    renderTasks();
  }
}

// 删除任务
function deleteTask(id) {
  appData.tasks = appData.tasks.filter(t => t.id !== id);
  saveData();
  renderTasks();
}

// 添加倒计时
function addCountdown(name, targetDate) {
  if (!name.trim() || !targetDate) return;

  if (!appData.countdowns) appData.countdowns = [];

  const countdown = {
    id: Date.now(),
    name: name.trim(),
    targetDate: targetDate,
    createdAt: new Date().toISOString().split('T')[0]
  };

  appData.countdowns.push(countdown);
  saveData();
  renderCountdowns();
}

// 删除倒计时
function deleteCountdown(id) {
  appData.countdowns = appData.countdowns.filter(c => c.id !== id);
  saveData();
  renderCountdowns();
}

// 模态框操作
function openTaskModal() {
  document.getElementById('taskModal').classList.add('show');
  document.getElementById('taskInput').focus();
}

function closeTaskModal() {
  document.getElementById('taskModal').classList.remove('show');
  document.getElementById('taskInput').value = '';
}

function confirmAddTask() {
  const input = document.getElementById('taskInput');
  addTask(input.value);
  closeTaskModal();
}

function openCountdownModal() {
  document.getElementById('countdownModal').classList.add('show');
  document.getElementById('countdownNameInput').focus();

  // 设置默认日期为明天
  const tomorrow = new Date();
  tomorrow.setDate(tomorrow.getDate() + 1);
  document.getElementById('countdownDateInput').value = tomorrow.toISOString().split('T')[0];
}

function closeCountdownModal() {
  document.getElementById('countdownModal').classList.remove('show');
  document.getElementById('countdownNameInput').value = '';
  document.getElementById('countdownDateInput').value = '';
}

function confirmAddCountdown() {
  const name = document.getElementById('countdownNameInput').value;
  const date = document.getElementById('countdownDateInput').value;
  addCountdown(name, date);
  closeCountdownModal();
}

// 事件监听
function setupEventListeners() {
  // 关闭按钮
  document.getElementById('closeBtn').addEventListener('click', () => {
    savePosition();
    // 隐藏窗口（浏览器版本）
    document.getElementById('widgetContainer').style.display = 'none';
  });

  // 添加任务按钮
  document.getElementById('addTaskBtn').addEventListener('click', openTaskModal);
  document.getElementById('confirmTaskBtn').addEventListener('click', confirmAddTask);
  document.getElementById('cancelTaskBtn').addEventListener('click', closeTaskModal);
  document.getElementById('closeTaskModal').addEventListener('click', closeTaskModal);

  // 添加倒计时按钮
  document.getElementById('addCountdownBtn').addEventListener('click', openCountdownModal);
  document.getElementById('confirmCountdownBtn').addEventListener('click', confirmAddCountdown);
  document.getElementById('cancelCountdownBtn').addEventListener('click', closeCountdownModal);
  document.getElementById('closeCountdownModal').addEventListener('click', closeCountdownModal);

  // 任务输入框
  document.getElementById('taskInput').addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      confirmAddTask();
    } else if (e.key === 'Escape') {
      closeTaskModal();
    }
  });

  // 倒计时输入框
  document.getElementById('countdownNameInput').addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      document.getElementById('countdownDateInput').focus();
    }
  });

  document.getElementById('countdownDateInput').addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      confirmAddCountdown();
    } else if (e.key === 'Escape') {
      closeCountdownModal();
    }
  });

  // 点击模态框外部关闭
  document.querySelectorAll('.modal').forEach(modal => {
    modal.addEventListener('click', (e) => {
      if (e.target === modal) {
        modal.classList.remove('show');
      }
    });
  });

  // 任务列表事件委托
  document.getElementById('taskList').addEventListener('click', (e) => {
    const target = e.target;
    const id = parseInt(target.dataset.id);

    if (target.classList.contains('task-checkbox')) {
      toggleTask(id);
    } else if (target.classList.contains('task-delete')) {
      deleteTask(id);
    }
  });

  // 倒计时列表事件委托
  document.getElementById('countdownList').addEventListener('click', (e) => {
    const target = e.target;
    const id = parseInt(target.dataset.id);

    if (target.classList.contains('countdown-delete')) {
      deleteCountdown(id);
    }
  });

  // 窗口拖拽
  setupDrag();
}

// 保存位置
function savePosition() {
  const container = document.getElementById('widgetContainer');
  const rect = container.getBoundingClientRect();
  appData.position = {
    x: rect.left,
    y: rect.top
  };
  saveData();
}

// 拖拽设置
function setupDrag() {
  const container = document.getElementById('widgetContainer');
  const dragRegion = document.querySelector('.drag-region');
  let isDragging = false;
  let startX, startY, startLeft, startTop;

  dragRegion.addEventListener('mousedown', (e) => {
    if (e.target.classList.contains('close-btn')) return;

    isDragging = true;
    startX = e.clientX;
    startY = e.clientY;
    startLeft = container.offsetLeft;
    startTop = container.offsetTop;
    container.style.cursor = 'grabbing';
  });

  document.addEventListener('mousemove', (e) => {
    if (!isDragging) return;

    const dx = e.clientX - startX;
    const dy = e.clientY - startY;

    container.style.left = (startLeft + dx) + 'px';
    container.style.top = (startTop + dy) + 'px';
  });

  document.addEventListener('mouseup', () => {
    if (isDragging) {
      isDragging = false;
      container.style.cursor = '';
      savePosition();
    }
  });
}

// 定期更新倒计时显示
function startCountdownUpdate() {
  setInterval(() => {
    renderCountdowns();
  }, 60000); // 每分钟更新一次
}

// 阻止默认右键菜单
document.addEventListener('contextmenu', (e) => {
  e.preventDefault();
});
