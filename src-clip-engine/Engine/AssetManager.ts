/**
 * 视频、音频、图片、字体、转场等资源统一加载、缓存、销毁
 * 
 */

export type AssetType = 'video' | 'image' | 'audio' | 'text';

export interface Asset {
    id:        string;
    type:      AssetType;
    url:       string;
    label:     string;
    duration:  number;      // 仅视频
    width?:    number;      // 仅视频
    height?:   number;      // 仅视频
    cover?:    ImageBitmap; // 封面
}

export class AssetManager {
    private assets: Map<string, Asset> = new Map();

    register(asset: Asset){
        if(this.assets.has(asset.id)) return;
        
        this.assets.set(asset.id, asset);
    }

    get(id: string): Asset | undefined {
        return this.assets.get(id);
    }

    remove(id: string){
        this.assets.delete(id);
    }

    has(id: string): boolean {
        return this.assets.has(id);
    }

    clear() {
        this.assets.clear();
    }
    
    list(): Asset[] {
        return Array.from(this.assets.values());
    }
}

