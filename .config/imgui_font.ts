/**
 * 字体支持扩展
 * 此文件不会被游戏启动时的自动生成覆盖
 * 使用模块扩展方式为 ImGui 接口添加字体支持函数
 * @author 鼠子
 * @link https://github.com/YuiNijika/ImGuiReduxChinese
 */


declare global {
    namespace ImGui {
        /**
         * 启用或禁用自定义字体支持
         * @param enabled 是否启用自定义字体支持（包括中文、日文、韩文等多语言字符）
         */
        function SetCustomFontEnabled(enabled: boolean): void;

        /**
         * 检查自定义字体支持是否已启用
         * @returns 如果自定义字体支持已启用则返回 true，否则返回 false
         */
        function IsCustomFontEnabled(): boolean;

        /**
         * 加载自定义字体文件
         * @param fontPath 字体文件的路径（支持 .ttf、.otf 等格式）
         * @param fontSize 字体大小（像素单位）
         * @returns 如果字体加载成功则返回 true，否则返回 false
         */
        function LoadCustomFont(fontPath: string, fontSize: number): boolean;
    }
}

export {};
