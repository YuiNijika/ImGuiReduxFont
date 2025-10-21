/**
 * 中文字体支持扩展 - 永久类型定义
 * 此文件不会被游戏启动时的自动生成覆盖
 * 使用模块扩展方式为 ImGui 接口添加中文字体支持函数
 */

declare global {
    interface ImGui {
        /**
         * 启用或禁用中文字体支持
         * @param enabled 是否启用中文字体支持
         */
        SetChineseSupportEnabled(enabled: boolean): void;

        /**
         * 检查是否启用了中文字体支持
         * @returns 如果启用了中文字体支持则返回 true
         */
        IsChineseSupportEnabled(): boolean;

        /**
         * 加载自定义字体文件
         * @param fontPath 字体文件路径
         * @param fontSize 字体大小
         * @returns 如果成功加载字体则返回 true
         */
        LoadCustomFont(fontPath: string, fontSize: float): boolean;
    }
}

// 确保这是一个模块
export {};