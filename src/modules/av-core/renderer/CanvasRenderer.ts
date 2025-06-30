import type { IRenderer } from './IRenderer';
export class CanvasRenderer implements IRenderer{

    private ctx?: CanvasRenderingContext2D;
    constructor(canvas: HTMLCanvasElement){
        if(canvas){
            this.ctx = canvas.getContext("2d") as CanvasRenderingContext2D;
        }
    }

    render(frame: VideoFrame){
        if(!this.ctx){
            return;
        }
        this.ctx.clearRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
        this.ctx.drawImage(
            frame,
            0, 0,
            frame.codedWidth, frame.codedHeight,
            0, 0,
            this.ctx.canvas.width,
            this.ctx.canvas.height
        )
        frame.close();
    }

    destroy(){

    }
}