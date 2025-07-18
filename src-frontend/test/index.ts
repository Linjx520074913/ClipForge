import { effect, ref } from 'vue';
import TestClipEngine from "./TestClipEngine.vue";
import { ClipEngine, VideoTrack, RendererUnit, ShaderParamPack, ShaderDescription } from 'clip-engine';

import { AXIOS } from '@frontend/api';

export{
    TestClipEngine
}

export function useTestClipEngine(){

    const imageRef  = ref<HTMLImageElement | null>(null);
    const canvasRef = ref<HTMLCanvasElement | null>(null);

    let engine:     ClipEngine;
    let videoTrack: VideoTrack;

    let videoFrame: VideoFrame | null = null;

    let shaders = ref([]);

    let activedShaderMap = ref<Map<string, ShaderDescriptor>>(new Map());

    async function fetchShaders(){
        try{
            // 获取服务器端滤镜列表
            const res = await AXIOS.request({
                method: 'GET',
                url: '/api/filter/query',
                headers:{
                    'Content-Type': 'multipart/form-data'
                }
            });
            shaders.value = res.data;
            console.error('Filter List:', res.data);
        }catch(e){
            console.error(e)
        }
    }

    async function activeShader(desc: ShaderDescription){
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