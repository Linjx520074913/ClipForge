import { effect, ref } from 'vue';
import TestClipEngine from "./TestClipEngine.vue";
import { ClipEngine, VideoTrack, RendererUnit } from 'clip-engine';

export{
    TestClipEngine
}

export function useTestClipEngine(){

    interface ShaderDescriptor{
        name: string,
        code: string,
        params: any,
        actived: boolean
    };

    const imageRef  = ref<HTMLImageElement | null>(null);
    const canvasRef = ref<HTMLCanvasElement | null>(null);

    let engine:     ClipEngine;
    let videoTrack: VideoTrack;

    let videoFrame: VideoFrame | null = null;

    let shaders = ref([]);

    let activedShaderMap = ref<Map<string, ShaderDescriptor>>(new Map());

    async function fetchShaders(){
        const list = [
            { 
                name: 'cartoon',
                codeURL: './shader/cartoon/cartoon.wgsl',
                code: '',
                paramsURL: '/shader/cartoon/params.json',
                params: { }
            },
            { 
                name: 'mosaic',
                codeURL: './shader/mosaic/mosaic.wgsl',
                code: '',
                paramsURL: '/shader/mosaic/params.json',
                params: { }
            }
        ]
        for(const f of list){
            const code = await (await fetch(f.codeURL)).text();
            const params = JSON.parse(await (await fetch(f.paramsURL)).text());
            shaders.value.push({ name: f.name, code, params, actived: false })
        }
    }

    async function activeShader(desc: ShaderDescriptor){
        if(desc.actived){
            // 移除 shader
            activedShaderMap.value.delete(desc.name);

            videoTrack.getEffectChain().removeNode(desc.name);
        }else{
            // 添加 shader
            activedShaderMap.value.set(desc.name, desc);

            const node = new RendererUnit(engine.getContext(), desc.name);
            node.initialize(desc.code);
            node.applyParameters(desc.params);

            videoTrack.getEffectChain().addNode(node);
        }

        desc.actived = !desc.actived;
        engine.render(videoFrame);
    }

    async function onParamsChange(s: ShaderDescriptor){
        videoTrack.getEffectChain().getNode(s.name)?.applyParameters(s.params);
        engine.render(videoFrame);
    }

    async function init(){
        engine = await ClipEngine.create();
        
        videoTrack = new VideoTrack("video-track-0", engine.getContext(), canvasRef.value);

        engine.addTrack(videoTrack);

        await fetchShaders();
    }

    async function render(){
        if(!videoFrame){
            const image = await createImageBitmap(imageRef.value);
            canvasRef.value.width = image.width;
            canvasRef.value.height= image.height;

            videoFrame = new VideoFrame(image, {
                timestamp: performance.now(),
                duration: 0
            });
        }
        await engine.render(videoFrame);
    }


    return{
        onParamsChange,
        imageRef,
        canvasRef,
        activeShader,
        shaders,
        init,
        render
    }
}