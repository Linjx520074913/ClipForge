import { writeTextFile, readTextFile, BaseDirectory } from '@tauri-apps/plugin-fs';
import { appDataDir, join } from '@tauri-apps/api/path';

export class ConfigStore {
    private data: Record<string, any> | null = null;
    private configDirName = 'ClipForge';
    private fileName = 'clipforge.json';
    private fullPath: string;

    private defaultData = {
        render_wnd_pos: { x: 0, y: 0 },
        render_wnd_size: { w: 800, h: 600 },
        theme: 'light',
    };

    /** 初始化：读取用户配置文件，如果不存在就创建默认数据 */
    async init() {
        const appDataPath = await appDataDir();
        const configDir = await join(appDataPath, this.configDirName);
        this.fullPath = await join(configDir, this.fileName);

        try {
        // 读取配置文件
        const content = await readTextFile(this.fullPath, { dir: BaseDirectory.AppData });
        this.data = JSON.parse(content);
        } catch (e) {
        // 文件不存在或解析错误 → 使用默认值并写入文件
        this.data = { ...this.defaultData };
        // FS 插件会自动创建父目录
        await writeTextFile(this.fullPath, JSON.stringify(this.data, null, 2), { dir: BaseDirectory.AppData });
        }
        return this.data;
    }

    /** 保存当前内存数据到文件 */
    async save() {
        if (!this.data) this.data = { ...this.defaultData };
        const content = JSON.stringify(this.data, null, 2);
        await writeTextFile(this.fullPath, content, { dir: BaseDirectory.AppData });
    }

    /** 获取字段，支持嵌套路径 'render_wnd_pos.x' */
    get(key?: string, fallback?: any) {
        if (!this.data) this.data = { ...this.defaultData };
        if (!key) return { ...this.data };

        const keys = key.split('.');
        let result: any = this.data;
        for (const k of keys) {
        if (result && k in result) result = result[k];
        else return fallback !== undefined ? fallback : null;
        }
        return result;
    }

    /** 设置字段，支持嵌套路径 */
    set(key: string, value: any) {
        if (!this.data) this.data = { ...this.defaultData };
        const keys = key.split('.');
        let obj: any = this.data;
        for (let i = 0; i < keys.length - 1; i++) {
        const k = keys[i];
        if (!(k in obj)) obj[k] = {};
        obj = obj[k];
        }
        obj[keys[keys.length - 1]] = value;
    }

    /** 批量更新并保存 */
    async updateAndSave(obj: Record<string, any>) {
        if (!this.data) this.data = { ...this.defaultData };
        Object.assign(this.data, obj);
        await this.save();
    }

    /** 获取全部数据副本 */
    getAll() {
        if (!this.data) this.data = { ...this.defaultData };
        return { ...this.data };
    }
}
