const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');

let mainWindow;
const dataFilePath = path.join(app.getPath('userData'), 'data.json');

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 340,
    height: 540,
    transparent: true,
    frame: false,
    alwaysOnTop: true,
    resizable: false,
    skipTaskbar: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  mainWindow.loadFile('index.html');

  // 加载保存的位置
  loadPosition();
}

function loadPosition() {
  try {
    if (fs.existsSync(dataFilePath)) {
      const data = JSON.parse(fs.readFileSync(dataFilePath, 'utf-8'));
      if (data.position && data.position.x && data.position.y) {
        mainWindow.setPosition(data.position.x, data.position.y);
      }
    }
  } catch (e) {
    console.log('No saved position or error:', e);
  }
}

function saveData(data) {
  try {
    fs.writeFileSync(dataFilePath, JSON.stringify(data, null, 2), 'utf-8');
  } catch (e) {
    console.error('Error saving data:', e);
  }
}

function loadData() {
  try {
    if (fs.existsSync(dataFilePath)) {
      return JSON.parse(fs.readFileSync(dataFilePath, 'utf-8'));
    }
  } catch (e) {
    console.error('Error loading data:', e);
  }
  return { tasks: [], countdowns: [], position: { x: 100, y: 100 } };
}

// IPC handlers
ipcMain.handle('load-data', () => {
  return loadData();
});

ipcMain.handle('save-data', (event, data) => {
  saveData(data);
});

ipcMain.handle('get-position', () => {
  if (mainWindow) {
    return mainWindow.getPosition();
  }
  return [100, 100];
});

ipcMain.handle('set-position', (event, x, y) => {
  if (mainWindow) {
    mainWindow.setPosition(x, y);
  }
});

ipcMain.handle('close-window', () => {
  if (mainWindow) {
    mainWindow.close();
  }
});

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
