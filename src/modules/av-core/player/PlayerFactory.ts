import { WebAVPlayer } from "./WebAVPlayer";
import type { IPlayer } from './IPlayer';
import type { IRenderer } from "../renderer/IRenderer";

export function createPlayer(
    type: 'webav' | 'custom',
    renderer: IRenderer
): IPlayer{
    let player: IPlayer;
    console.error('@@@@@@@@@@', type)
    switch(type){
        case 'webav':
            player = new WebAVPlayer();
            break;
        case 'custom':
            throw new Error('Custom player not implemented yet');
        default:
            throw new Error(`Unknown player type: ${type}`);
    }

    player.initialize(renderer);
    return player;
}