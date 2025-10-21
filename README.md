# ImGuiRedux

为GTA游戏提供完整自定义字体支持的ImGui界面库

## 代码示例

```javascript
// 启用中文支持
ImGui.SetChineseSupportEnabled(true);

// 加载中文字体
ImGui.LoadCustomFont("CLEO/Fonts/msyh.ttc", 16.0);

// 创建窗口
if (ImGui.Begin("测试窗口", true)) {
    ImGui.Text("你好世界！");
    if (ImGui.Button("点击按钮")) {
        // 按钮被点击
    }
}
ImGui.End();
```

## 下载

[最新版本](https://github.com/user-grinch/ImGuiRedux/releases/latest)

