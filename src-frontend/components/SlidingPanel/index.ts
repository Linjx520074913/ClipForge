import type { DefineComponent } from "vue"
export interface PanelItem{
    id:        number
    label:     string
    icon?:     string
    tooltip?:  string,
    component?:DefineComponent<any, any, any>
}

export function useSlidingPanel(){
    return{
        
    }
}