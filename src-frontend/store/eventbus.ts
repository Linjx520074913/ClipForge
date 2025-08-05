import { EventBus } from 'clip-engine';

/**
 * 动作时间
 */
export type ActionEvent = {
    // 右侧 shader 
    'shader:active': {  };
};

// export const eventBus = new EventBus();
export const Bus = new EventBus<ActionEvent>();
